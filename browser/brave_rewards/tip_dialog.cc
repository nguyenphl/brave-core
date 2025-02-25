/* Copyright 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/brave_rewards/tip_dialog.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/memory/raw_ptr.h"
#include "base/strings/utf_string_conversions.h"
#include "brave/browser/brave_rewards/rewards_panel/rewards_panel_coordinator.h"
#include "brave/browser/brave_rewards/rewards_service_factory.h"
#include "brave/components/brave_rewards/common/pref_names.h"
#include "brave/components/constants/webui_url_constants.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/webui/chrome_web_contents_handler.h"
#include "chrome/browser/ui/webui/constrained_web_dialog_ui.h"
#include "components/guest_view/browser/guest_view_base.h"
#include "components/web_modal/web_contents_modal_dialog_host.h"
#include "content/public/browser/web_contents.h"
#include "ui/web_dialogs/web_dialog_delegate.h"

using content::WebContents;
using content::WebUIMessageHandler;

namespace {

constexpr int kDialogMargin = 25;
constexpr int kDialogMinHeight = 400;
constexpr int kDialogMaxHeight = 700;
// max width = 1920 - margin
constexpr int kDialogMaxWidth = 1895;

// A ui::WebDialogDelegate that specifies the tip dialog appearance.
class TipDialogDelegate : public ui::WebDialogDelegate {
 public:
  explicit TipDialogDelegate(WebContents* initiator, base::Value::Dict params);
  TipDialogDelegate(const TipDialogDelegate&) = delete;
  TipDialogDelegate& operator=(const TipDialogDelegate&) = delete;
  ~TipDialogDelegate() override;

  ui::ModalType GetDialogModalType() const override;
  std::u16string GetDialogTitle() const override;
  GURL GetDialogContentURL() const override;
  void GetWebUIMessageHandlers(
      std::vector<WebUIMessageHandler*>* handlers) const override;
  void GetDialogSize(gfx::Size* size) const override;
  std::string GetDialogArgs() const override;
  void OnDialogClosed(const std::string& json_retval) override;
  void OnCloseContents(WebContents* source, bool* out_close_dialog) override;
  bool ShouldShowDialogTitle() const override;

 private:
  raw_ptr<WebContents> initiator_ = nullptr;
  base::Value::Dict params_;
};

TipDialogDelegate::TipDialogDelegate(WebContents* initiator,
                                     base::Value::Dict params)
    : initiator_(initiator), params_(std::move(params)) {}

TipDialogDelegate::~TipDialogDelegate() = default;

ui::ModalType TipDialogDelegate::GetDialogModalType() const {
  // Not used, returning dummy value.
  NOTREACHED();
  return ui::MODAL_TYPE_WINDOW;
}

std::u16string TipDialogDelegate::GetDialogTitle() const {
  // Only used on Windows?
  return std::u16string();
}

GURL TipDialogDelegate::GetDialogContentURL() const {
  return GURL(kBraveUITipURL);
}

void TipDialogDelegate::GetWebUIMessageHandlers(
    std::vector<WebUIMessageHandler*>* /* handlers */) const {
  // TipWebUI should add its own message handlers.
}

void TipDialogDelegate::GetDialogSize(gfx::Size* size) const {
  DCHECK(size);

  gfx::Size target_size;

  web_modal::WebContentsModalDialogHost* host = nullptr;
  content::WebContents* outermost_web_contents =
      guest_view::GuestViewBase::GetTopLevelWebContents(initiator_);

  Browser* browser = chrome::FindBrowserWithWebContents(outermost_web_contents);
  if (browser)
    host = browser->window()->GetWebContentsModalDialogHost();

  if (host)
    target_size = host->GetMaximumDialogSize();
  else
    target_size = outermost_web_contents->GetContainerBounds().size();
  // initial size in between min and max
  const int max_height =
      kDialogMinHeight + (kDialogMaxHeight - kDialogMinHeight);
  int width = std::min(target_size.width() - kDialogMargin, kDialogMaxWidth);
  size->SetSize(width, max_height);
}

std::string TipDialogDelegate::GetDialogArgs() const {
  std::string json;
  base::JSONWriter::Write(params_, &json);
  return json;
}

void TipDialogDelegate::OnDialogClosed(
    const std::string& /* json_retval */) {
}

void TipDialogDelegate::OnCloseContents(WebContents* /* source */,
                                                 bool* out_close_dialog) {
  *out_close_dialog = true;
}

bool TipDialogDelegate::ShouldShowDialogTitle() const {
  return false;
}

}  // namespace

namespace brave_rewards {

void OpenTipDialog(WebContents* initiator, base::Value::Dict params) {
  auto* profile = Profile::FromBrowserContext(initiator->GetBrowserContext());

  // If the user has not enabled rewards, redirect the user to the Rewards
  // panel to complete onboarding.
  if (!profile->GetPrefs()->GetBoolean(prefs::kEnabled)) {
    if (auto* browser = chrome::FindBrowserWithWebContents(initiator)) {
      if (auto* coordinator = RewardsPanelCoordinator::FromBrowser(browser)) {
        coordinator->OpenRewardsPanel();
        return;
      }
    }
  }

  content::WebContents* outermost_web_contents =
    guest_view::GuestViewBase::GetTopLevelWebContents(initiator);
  gfx::Size host_size = outermost_web_contents->GetContainerBounds().size();
  int width = std::min(host_size.width() - kDialogMargin, kDialogMaxWidth);
  gfx::Size min_size(width, kDialogMinHeight);
  gfx::Size max_size(width, kDialogMaxHeight);
  // TODO(petemill): adjust min and max when host size changes (e.g. window
  // resize)
  ShowConstrainedWebDialogWithAutoResize(
      initiator->GetBrowserContext(),
      std::make_unique<TipDialogDelegate>(initiator, std::move(params)),
      initiator, min_size, max_size);
}

}  // namespace brave_rewards
