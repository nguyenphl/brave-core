/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import { localeStrings as walletCardStrings } from '../../shared/components/wallet_card/stories/locale_strings'
import { localeStrings as onboardingStrings } from '../../shared/components/onboarding/stories/locale_strings'

export const localeStrings = {
  ...walletCardStrings,
  ...onboardingStrings,

  aboutRewardsText: 'Using Brave Rewards helps support content creators, and lets you earn BAT.',
  connectAccount: 'Connect account',
  connectAccountText: '$1Ready to start earning BAT?$2 Just connect your custodial account to your Rewards profile. If you don’t have an account, you’ll get the chance to create one.',
  connectAccountNoProviders: 'To earn BAT, users must connect a custodial account to Brave Rewards. Unfortunately, there\'s no custodian available in your region, so earning isn\'t available. For now, turning on Brave Rewards will automatically support creators.',
  learnMore: 'Learn more',
  publisherCountText: 'This month, you\'ve visited $1 creators supported by Brave Rewards',
  headerTitle: 'Brave Rewards',
  headerTextAdsEnabled: 'You are helping support content creators',
  headerTextAdsDisabled: 'Turn on to help support content creators',
  rewardsSettings: 'Rewards settings',
  learnMoreAboutBAT: 'Learn more about BAT',
  summary: 'Summary',
  tip: 'Tip',
  unverifiedCreator: 'Unverified Creator',
  verifiedCreator: 'Verified Creator',
  refreshStatus: 'Refresh Creator verification status',
  pendingTipText: 'For now, any tip you make will remain pending ' +
                  'and retry automatically for 90 days.',
  pendingTipTitle: 'This creator is not signed up yet.',
  pendingTipTitleRegistered: 'You\'re connected to $1, but this creator ' +
                             'is connected to $2. This means your tip ' +
                             'can\'t reach this creator.',
  unverifiedTextMore: 'Learn more.',
  platformPublisherTitle: '$1 on $2',
  attention: 'Attention',
  sendTip: 'Send Tip',
  includeInAutoContribute: 'Include in Auto-Contribute',
  monthlyTip: 'Monthly Tip',
  ok: 'OK',
  set: 'Set',
  changeAmount: 'Change amount',
  cancel: 'Cancel',

  grantCaptchaTitle: 'Confirm you are a human being',
  grantCaptchaFailedTitle: 'Hmm… Not Quite. Try Again.',
  grantCaptchaHint: 'Drag the BAT icon onto the $1 target.',
  grantCaptchaPassedTitleUGP: 'It’s your lucky day!',
  grantCaptchaPassedTextUGP: 'Your token grant is on its way.',
  grantCaptchaAmountUGP: 'Free Token Grant',
  grantCaptchaPassedTitleAds: 'You\'ve earned an Ads Reward!',
  grantCaptchaPassedTextAds: 'Your Reward is on its way.',
  grantCaptchaAmountAds: 'Your Reward Amount',
  grantCaptchaExpiration: 'Grant Expiration Date',
  grantCaptchaErrorTitle: 'Oops, something is wrong.',
  grantCaptchaErrorText: 'Brave Rewards is having an issue. Please try again later.',

  captchaSolvedTitle: 'Solved!',
  captchaSolvedText: 'Ads and earnings will now resume. Thanks for helping us protect Brave Rewards and privacy-based ads.',
  captchaDismiss: 'Dismiss',
  captchaMaxAttemptsExceededTitle: 'Max attempts exceeded',
  captchaMaxAttemptsExceededText: 'Looks like this is not working, Brave ads will remain paused. Contact us if you need help with the captcha.',
  captchaContactSupport: 'Contact support',

  notificationReconnect: 'Reconnect',
  notificationClaimRewards: 'Claim Rewards',
  notificationClaimTokens: 'Claim Tokens',
  notificationAutoContributeCompletedTitle: 'Auto-Contribute',
  notificationAutoContributeCompletedText: 'You\'ve contributed $1.',
  notificationWalletDisconnectedTitle: 'You are logged out',
  notificationWalletDisconnectedText: 'This can happen to keep your account secure. Click below to reconnect now.',
  notificationUpholdBATNotAllowedTitle: 'Error: BAT unavailable',
  notificationUpholdBATNotAllowedText: 'BAT is not yet supported in your region on Uphold.',
  notificationUpholdInsufficientCapabilitiesTitle: 'Error: Limited Uphold account functionality',
  notificationUpholdInsufficientCapabilitiesText: 'According to Uphold, there are currently some limitations on your Uphold account. Please log in to your Uphold account and check whether there are any notices or remaining account requirements to complete, then try again.',
  notificationWalletDisconnectedAction: 'Reconnect',
  notificationTokenGrantTitle: 'A token grant is available!',
  notificationAdGrantAmount: '$1 Rewards: $2',
  notificationAdGrantTitle: 'Your $1 Ad Rewards are here!',
  notificationGrantDaysRemaining: 'You have $1 left to claim',
  notificationMonthlyContributionFailedText: 'There was a problem processing your contribution.',
  notificationMonthlyContributionFailedTitle: 'Monthly contribution failed',
  notificationMonthlyTipCompletedTitle: 'Contributions and tips',
  notificationMonthlyTipCompletedText: 'Your monthly contributions have been processed.',
  notificationPublisherVerifiedTitle: 'Pending contribution',
  notificationPublisherVerifiedText: 'Creator $1 recently verified.',

  rewardsConnectAccount: 'Connect account',
  rewardsVBATNoticeTitle1: 'Action required: Connect a custodial account or your vBAT will be lost',
  rewardsVBATNoticeText1: 'On $1, we will be discontinuing support for existing virtual BAT balances. Connect a custodial account before this date so we can transfer your earned balance to your custodial account, otherwise your balance will be lost.',
  rewardsVBATNoticeTitle2: 'You still have time to contribute your vBAT to your favorite creators',
  rewardsVBATNoticeText2: 'On $1, we will be discontinuing support for existing virtual BAT balances. Unfortunately, there are no available custodians in your region ($2) to withdraw your earnings. Until then, you can still contribute to your favorite creators.'
}
