/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/speedreader/speedreader_url_loader.h"

#include <memory>
#include <string>
#include <utility>

#include "base/check.h"
#include "base/command_line.h"
#include "base/files/file_util.h"
#include "base/functional/bind.h"
#include "base/memory/weak_ptr.h"
#include "base/metrics/histogram_macros.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"
#include "brave/components/body_sniffer/body_sniffer_throttle.h"
#include "brave/components/speedreader/rust/ffi/speedreader.h"
#include "brave/components/speedreader/speedreader_rewriter_service.h"
#include "brave/components/speedreader/speedreader_service.h"
#include "brave/components/speedreader/speedreader_throttle.h"
#include "brave/components/speedreader/speedreader_throttle_delegate.h"
#include "mojo/public/cpp/bindings/self_owned_receiver.h"

namespace speedreader {

namespace {

constexpr uint32_t kReadBufferSize = 32768;

void MaybeSaveDistilledDataForDebug(const GURL& url,
                                    const std::string& data,
                                    const std::string& stylesheet,
                                    const std::string& transformed) {
#if DCHECK_IS_ON()
  constexpr const char kCollectSwitch[] = "speedreader-collect-test-data";
  if (!base::CommandLine::ForCurrentProcess()->HasSwitch(kCollectSwitch))
    return;
  const auto dir = base::CommandLine::ForCurrentProcess()->GetSwitchValuePath(
      kCollectSwitch);
  base::CreateDirectory(dir);
  base::WriteFile(dir.AppendASCII("page.url"), url.spec());
  base::WriteFile(dir.AppendASCII("original.html"), data);
  base::WriteFile(dir.AppendASCII("distilled.html"), transformed);
  base::WriteFile(dir.AppendASCII("result.html"), stylesheet + transformed);
#endif
}

}  // namespace

// static
std::tuple<mojo::PendingRemote<network::mojom::URLLoader>,
           mojo::PendingReceiver<network::mojom::URLLoaderClient>,
           SpeedReaderURLLoader*>
SpeedReaderURLLoader::CreateLoader(
    base::WeakPtr<body_sniffer::BodySnifferThrottle> throttle,
    base::WeakPtr<SpeedreaderThrottleDelegate> delegate,
    const GURL& response_url,
    scoped_refptr<base::SingleThreadTaskRunner> task_runner,
    SpeedreaderRewriterService* rewriter_service,
    SpeedreaderService* speedreader_service) {
  mojo::PendingRemote<network::mojom::URLLoader> url_loader;
  mojo::PendingRemote<network::mojom::URLLoaderClient> url_loader_client;
  mojo::PendingReceiver<network::mojom::URLLoaderClient>
      url_loader_client_receiver =
          url_loader_client.InitWithNewPipeAndPassReceiver();

  auto loader = base::WrapUnique(new SpeedReaderURLLoader(
      std::move(throttle), std::move(delegate), response_url,
      std::move(url_loader_client), std::move(task_runner), rewriter_service,
      speedreader_service));
  SpeedReaderURLLoader* loader_rawptr = loader.get();
  mojo::MakeSelfOwnedReceiver(std::move(loader),
                              url_loader.InitWithNewPipeAndPassReceiver());
  return std::make_tuple(std::move(url_loader),
                         std::move(url_loader_client_receiver), loader_rawptr);
}

SpeedReaderURLLoader::SpeedReaderURLLoader(
    base::WeakPtr<body_sniffer::BodySnifferThrottle> throttle,
    base::WeakPtr<SpeedreaderThrottleDelegate> delegate,
    const GURL& response_url,
    mojo::PendingRemote<network::mojom::URLLoaderClient>
        destination_url_loader_client,
    scoped_refptr<base::SingleThreadTaskRunner> task_runner,
    SpeedreaderRewriterService* rewriter_service,
    SpeedreaderService* speedreader_service)
    : body_sniffer::BodySnifferURLLoader(
          throttle,
          response_url,
          std::move(destination_url_loader_client),
          task_runner),
      delegate_(delegate),
      response_url_(response_url),
      rewriter_service_(rewriter_service),
      speedreader_service_(speedreader_service) {}

SpeedReaderURLLoader::~SpeedReaderURLLoader() = default;

void SpeedReaderURLLoader::OnBodyReadable(MojoResult) {
  DCHECK_EQ(State::kLoading, state_);

  if (!BodySnifferURLLoader::CheckBufferedBody(kReadBufferSize)) {
    return;
  }

  // TODO(iefremov): We actually can partially |pumpContent| to speedreader,
  // but skipping it for now to simplify things. Pumping is not free in terms
  // of CPU ticks, so we will have to keep alive speedreader instance on another
  // thread.

  body_consumer_watcher_.ArmOrNotify();
}

void SpeedReaderURLLoader::OnBodyWritable(MojoResult r) {
  DCHECK_EQ(State::kSending, state_);
  if (bytes_remaining_in_buffer_ > 0) {
    SendBufferedBodyToClient();
  } else {
    CompleteSending();
  }
}

void SpeedReaderURLLoader::CompleteLoading(std::string body) {
  DCHECK_EQ(State::kLoading, state_);
  if (!throttle_ || !rewriter_service_) {
    Abort();
    return;
  }

  VLOG(2) << __func__ << " buffered body size = " << body.size();
  bytes_remaining_in_buffer_ = body.size();

  if (bytes_remaining_in_buffer_ > 0) {
    // Offload heavy distilling to another thread.
    base::ThreadPool::PostTaskAndReplyWithResult(
        FROM_HERE, {base::TaskPriority::USER_BLOCKING, base::MayBlock()},
        base::BindOnce(
            [](const GURL& response_url, std::string data,
               std::unique_ptr<Rewriter> rewriter,
               const std::string& stylesheet) -> auto{
              SCOPED_UMA_HISTOGRAM_TIMER("Brave.Speedreader.Distill");
              int written = rewriter->Write(data.c_str(), data.length());
              // Error occurred
              if (written != 0) {
                return data;
              }

              rewriter->End();
              const std::string& transformed = rewriter->GetOutput();

              // TODO(brave-browser/issues/10372): would be better to pass
              // explicit signal back from rewriter to indicate if content was
              // found
              if (transformed.length() < 1024) {
                return data;
              }
              MaybeSaveDistilledDataForDebug(response_url, data, stylesheet,
                                             transformed);
              return stylesheet + transformed;
            },
            response_url_, std::move(body),
            rewriter_service_->MakeRewriter(
                response_url_, speedreader_service_->GetThemeName(),
                speedreader_service_->GetFontFamilyName(),
                speedreader_service_->GetFontSizeName(),
                speedreader_service_->GetContentStyleName()),
            rewriter_service_->GetContentStylesheet()),
        base::BindOnce(
            [](base::WeakPtr<SpeedReaderURLLoader> self, std::string result) {
              if (self) {
                self->BodySnifferURLLoader::CompleteLoading(std::move(result));
              }
            },
            weak_factory_.GetWeakPtr()));
    return;
  }
  BodySnifferURLLoader::CompleteLoading(std::move(body));
}

void SpeedReaderURLLoader::OnCompleteSending() {
  // TODO(keur, iefremov): This API could probably be improved with an enum
  // indicating distill success, distill fail, load from cache.
  // |complete_status_| has an |exists_in_cache| field.
  if (delegate_)
    delegate_->OnDistillComplete();
}

}  // namespace speedreader
