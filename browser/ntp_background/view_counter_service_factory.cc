// Copyright (c) 2020 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/browser/ntp_background/view_counter_service_factory.h"

#include <memory>

#include "brave/browser/brave_ads/ads_service_factory.h"
#include "brave/browser/brave_browser_process.h"
#include "brave/browser/ntp_background/ntp_p3a_helper_impl.h"
#include "brave/browser/profiles/profile_util.h"
#include "brave/components/brave_ads/browser/ads_service.h"
#include "brave/components/constants/pref_names.h"
#include "brave/components/ntp_background_images/browser/ntp_background_images_service.h"
#include "brave/components/ntp_background_images/browser/ntp_background_images_source.h"
#include "brave/components/ntp_background_images/browser/ntp_sponsored_images_source.h"
#include "brave/components/ntp_background_images/browser/view_counter_service.h"
#include "brave/components/ntp_background_images/buildflags/buildflags.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/url_data_source.h"

#if BUILDFLAG(ENABLE_CUSTOM_BACKGROUND)
#include "brave/browser/ntp_background/ntp_custom_background_images_service_factory.h"
#endif

namespace ntp_background_images {

// static
ViewCounterService* ViewCounterServiceFactory::GetForProfile(Profile* profile) {
  return static_cast<ViewCounterService*>(
      GetInstance()->GetServiceForBrowserContext(profile, true));
}

// static
ViewCounterServiceFactory* ViewCounterServiceFactory::GetInstance() {
  return base::Singleton<ViewCounterServiceFactory>::get();
}

ViewCounterServiceFactory::ViewCounterServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "ViewCounterService",
          BrowserContextDependencyManager::GetInstance()) {
  DependsOn(brave_ads::AdsServiceFactory::GetInstance());
#if BUILDFLAG(ENABLE_CUSTOM_BACKGROUND)
  DependsOn(NTPCustomBackgroundImagesServiceFactory::GetInstance());
#endif
}

ViewCounterServiceFactory::~ViewCounterServiceFactory() = default;

KeyedService* ViewCounterServiceFactory::BuildServiceInstanceFor(
    content::BrowserContext* browser_context) const {
  // Only NTP in normal profile uses sponsored services.
  if (!brave::IsRegularProfile(browser_context))
    return nullptr;

  if (auto* service =
          g_brave_browser_process->ntp_background_images_service()) {
    Profile* profile = Profile::FromBrowserContext(browser_context);
    bool is_supported_locale = false;
    auto* ads_service = brave_ads::AdsServiceFactory::GetForProfile(profile);
    if (ads_service) {
      is_supported_locale = ads_service->IsSupportedLocale();
    }
    content::URLDataSource::Add(
        browser_context, std::make_unique<NTPBackgroundImagesSource>(service));
    content::URLDataSource::Add(
        browser_context, std::make_unique<NTPSponsoredImagesSource>(service));

    return new ViewCounterService(
        service,
#if BUILDFLAG(ENABLE_CUSTOM_BACKGROUND)
        NTPCustomBackgroundImagesServiceFactory::GetForContext(profile),
#else
        nullptr,
#endif
        ads_service, profile->GetPrefs(), g_browser_process->local_state(),
        std::make_unique<NTPP3AHelperImpl>(
            g_browser_process->local_state(),
            g_brave_browser_process->brave_p3a_service(), ads_service),
        is_supported_locale);
  }

  return nullptr;
}

void ViewCounterServiceFactory::RegisterProfilePrefs(
    user_prefs::PrefRegistrySyncable* registry) {
  ViewCounterService::RegisterProfilePrefs(registry);
}

bool ViewCounterServiceFactory::ServiceIsCreatedWithBrowserContext() const {
  return true;
}

}  // namespace ntp_background_images
