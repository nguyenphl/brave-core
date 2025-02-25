/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import * as React from 'react'

import {
  AlertCircleIcon,
  RedditColorIcon,
  TwitterColorIcon,
  TwitchColorIcon,
  GitHubColorIcon,
  YoutubeColorIcon,
  VimeoColorIcon
} from 'brave-ui/components/icons'

import {
  PublisherInfo,
  ExternalWalletInfo,
  MediaMetaData
} from '../lib/interfaces'

import { HostContext } from '../lib/host_context'

import {
  Locale,
  LocaleContext,
  formatMessage
} from '../../shared/lib/locale_context'
import { getExternalWalletProviderName } from '../../shared/lib/external_wallet'
import {
  isPublisherVerified,
  PublisherStatus,
  publisherStatusToWalletProviderName
} from '../../shared/lib/publisher_status'

import { MediaCard } from './media_card'
import { NewTabLink } from '../../shared/components/new_tab_link'
import { VerifiedIcon } from './icons/verified_icon'

import * as style from './publisher_banner.style'

import * as mojom from '../../shared/lib/mojom'

function getLogoURL (publisherInfo: PublisherInfo) {
  const { logo } = publisherInfo
  if (!logo || !isPublisherVerified(publisherInfo.status)) {
    return ''
  }
  if (/^https:\/\/[a-z0-9-]+\.invalid(\/)?$/.test(logo)) {
    return `chrome://favicon/size/160@1x/${logo}`
  }
  return logo
}

function getLogo (publisherInfo: PublisherInfo) {
  const logoURL = getLogoURL(publisherInfo)
  if (logoURL) {
    return <img src={logoURL} />
  }
  const name = publisherInfo.name || publisherInfo.publisherKey
  return (
    <style.logoLetter>
      {name ? name[0] : ''}
    </style.logoLetter>
  )
}

function getProviderName (publisherInfo: PublisherInfo) {
  switch (publisherInfo.provider) {
    case 'youtube': return 'YouTube'
    case 'twitter': return 'Twitter'
    case 'twitch': return 'Twitch'
    case 'reddit': return 'Reddit'
    case 'vimeo': return 'Vimeo'
    case 'github': return 'GitHub'
    default: return ''
  }
}

function getSocialScreenName (mediaMetaData: MediaMetaData) {
  switch (mediaMetaData.mediaType) {
    case 'twitter':
      return '@' + mediaMetaData.publisherName
    case 'github':
      return '@' + mediaMetaData.publisherScreenName
    case 'reddit':
      return 'u/' + mediaMetaData.publisherName
  }
  return ''
}

function getPublisherName (
  locale: Locale,
  publisherInfo: PublisherInfo,
  mediaMetaData: MediaMetaData
) {
  const name = publisherInfo.name || publisherInfo.publisherKey
  const platform = getProviderName(publisherInfo)
  if (!platform) {
    return name
  }
  const screenName = getSocialScreenName(mediaMetaData)
  return (
    <>
      {name} {locale.getString('on')} {platform}
      {screenName && <style.socialName>{screenName}</style.socialName>}
    </>
  )
}

function getSocialIcon (type: string) {
  switch (type) {
    case 'twitter': return <TwitterColorIcon />
    case 'youtube': return <YoutubeColorIcon />
    case 'twitch': return <TwitchColorIcon />
    case 'github': return <GitHubColorIcon />
    case 'reddit': return <RedditColorIcon />
    case 'vimeo': return <VimeoColorIcon />
    default: return null
  }
}

function isValidSocialLink (url: string) {
  try {
    // The URL constructor will throw when provided with any
    // string that is not an absolute URL. If the URL constuctor
    // does not throw, consider it a valid social link URL.
    // eslint-disable-next-line no-new
    new URL(url)
    return true
  } catch (_) {
    return false
  }
}

function getSocialLinks (publisherInfo: PublisherInfo) {
  return Object.entries(publisherInfo.links).map(([type, url]) => {
    const icon = getSocialIcon(type)
    return icon && isValidSocialLink(url)
      ? <NewTabLink key={type} href={url}>{icon}</NewTabLink>
      : null
  })
}

function getTitle (
  locale: Locale,
  publisherInfo: PublisherInfo,
  mediaMetaData: MediaMetaData
) {
  // For Twitter and Reddit posts a media card is displayed
  // instead of a title
  if (mediaMetaData.mediaType === 'twitter' ||
      mediaMetaData.mediaType === 'reddit') {
    return
  }

  return publisherInfo.title || locale.getString('welcome')
}

function getVerifiedIcon (publisherInfo: PublisherInfo) {
  if (!isPublisherVerified(publisherInfo.status)) {
    return null
  }
  return <VerifiedIcon />
}

function showUnverifiedNotice (
  publisherInfo: PublisherInfo,
  externalWalletInfo?: ExternalWalletInfo
) {
  // Show the notice if the publisher is not verified.
  if (!isPublisherVerified(publisherInfo.status)) {
    return true
  }

  // Do not show the notice if the publisher is verified and the user does
  // not have a connected external wallet.
  if (!externalWalletInfo) {
    return false
  }
  switch (externalWalletInfo.status) {
    case mojom.WalletStatus.kLoggedOut:
    case mojom.WalletStatus.kNotConnected:
      return false
  }

  // Show the notice if the publisher is verified and their wallet provider does
  // not match the user's external wallet provider.
  switch (publisherInfo.status) {
    case PublisherStatus.UPHOLD_VERIFIED:
      return externalWalletInfo.type !== 'uphold'
    case PublisherStatus.BITFLYER_VERIFIED:
      return externalWalletInfo.type !== 'bitflyer'
    case PublisherStatus.GEMINI_VERIFIED:
      return externalWalletInfo.type !== 'gemini'
  }

  return true
}

function getUnverifiedNotice (
  locale: Locale,
  publisherInfo: PublisherInfo,
  walletInfo: ExternalWalletInfo | undefined
) {
  if (!showUnverifiedNotice(publisherInfo, walletInfo)) {
    return null
  }

  const { getString } = locale

  const text =
    !isPublisherVerified(publisherInfo.status)
      ? getString('siteBannerNoticeNotRegistered')
      : <>
        {
          formatMessage(getString('pendingTipTitleRegistered'), [
            getExternalWalletProviderName(walletInfo!.type),
            publisherStatusToWalletProviderName(publisherInfo.status)
          ])
        }
        {getString('pendingTipText')}
      </>

  return (
    <style.unverifiedNotice>
      <style.unverifiedNoticeIcon>
        <AlertCircleIcon />
      </style.unverifiedNoticeIcon>
      <style.unverifiedNoticeText>
        <strong>{getString('siteBannerNoticeNote')}</strong>&nbsp;
        {text}&nbsp;
        <NewTabLink href='https://brave.com/faq/#unclaimed-funds'>
          {getString('unverifiedTextMore')}
        </NewTabLink>
      </style.unverifiedNoticeText>
    </style.unverifiedNotice>
  )
}

function getPostRelativeTime (postDate: Date) {
  // TS does not yet recongnize RelativeTimeFormatter (since chromium 71)
  const { RelativeTimeFormat } = Intl as any
  const formatter = new RelativeTimeFormat()
  const sec = Math.max(0, Date.now() - postDate.getTime()) / 1000
  if (sec < 60) {
    return formatter.format(-Math.round(sec), 'second')
  }
  if (sec < 60 * 60) {
    return formatter.format(-Math.round(sec / 60), 'minute')
  }
  if (sec < 60 * 60 * 24) {
    return formatter.format(-Math.round(sec / 60 / 60), 'hour')
  }
  return ''
}

function getPostTimeString (postTimestamp: string) {
  const postDate = new Date(postTimestamp)
  const invalidDate = !Number(postDate)
  if (invalidDate) {
    return postTimestamp
  }
  const relative = getPostRelativeTime(postDate)
  if (relative) {
    return relative
  }
  return postDate.toLocaleDateString(undefined, {
    month: 'short',
    day: 'numeric',
    year: postDate.getFullYear() !== new Date().getFullYear()
      ? 'numeric'
      : undefined
  })
}

function getDescription (
  locale: Locale,
  publisherInfo: PublisherInfo,
  mediaMetaData: MediaMetaData
) {
  const { getString } = locale
  const { name } = publisherInfo

  if (mediaMetaData.mediaType === 'twitter') {
    const postTime = getPostTimeString(mediaMetaData.postTimestamp)
    const title = formatMessage(getString('postHeaderTwitter'), [name])
    return (
      <MediaCard title={title} postTime={postTime} icon={<TwitterColorIcon />}>
        {mediaMetaData.postText}
      </MediaCard>
    )
  }

  if (mediaMetaData.mediaType === 'reddit') {
    const postTime = getPostTimeString(mediaMetaData.postTimestamp)
    const title = formatMessage(getString('postHeader'), [name])
    return (
      <MediaCard title={title} postTime={postTime} icon={<RedditColorIcon />}>
        {mediaMetaData.postText}
      </MediaCard>
    )
  }

  if (publisherInfo.description) {
    return publisherInfo.description
  }

  return getString('rewardsBannerText1')
}

function hashString (input: string) {
  let h = 0
  for (let i = 0; i < input.length; i++) {
    h = (((h << 5) - h) + input.charCodeAt(i)) >>> 0
  }
  return h
}

function getBackgroundClass (publisherInfo: PublisherInfo) {
  if (publisherInfo.background) {
    return 'background-style-custom'
  }

  // Vary the background type by publisher key and day
  const hash = hashString(
    publisherInfo.publisherKey + '-' +
    Math.floor(Date.now() / 1000 / 60 / 60 / 24))

  return `background-style-${String(hash % 5 + 1)}`
}

export function PublisherBanner () {
  const host = React.useContext(HostContext)
  const locale = React.useContext(LocaleContext)

  const [publisherInfo, setPublisherInfo] = React.useState(
    host.state.publisherInfo)
  const [walletInfo, setWalletInfo] = React.useState(
    host.state.externalWalletInfo)

  React.useEffect(() => {
    return host.addListener((state) => {
      setPublisherInfo(state.publisherInfo)
      setWalletInfo(state.externalWalletInfo)
    })
  }, [host])

  if (!publisherInfo) {
    return <style.loading />
  }

  const { mediaMetaData } = host.getDialogArgs()

  const backgroundImage = publisherInfo.background
    ? `url(${publisherInfo.background})`
    : undefined

  const verifiedType = isPublisherVerified(publisherInfo.status)
    ? 'verified'
    : 'not-verified'

  return (
    <style.root style={{ backgroundImage }}>
      <style.background className={getBackgroundClass(publisherInfo)}>
        <style.card>
          <style.header>
            <style.logo>
              <style.logoMask>
                {getLogo(publisherInfo)}
              </style.logoMask>
              <style.verifiedIcon className={verifiedType}>
                {getVerifiedIcon(publisherInfo)}
              </style.verifiedIcon>
            </style.logo>
            <style.name>
              {getPublisherName(locale, publisherInfo, mediaMetaData)}
            </style.name>
          </style.header>
          <style.socialLinks>
            {getSocialLinks(publisherInfo)}
          </style.socialLinks>
          {getUnverifiedNotice(locale, publisherInfo, walletInfo)}
          <style.title>
            {getTitle(locale, publisherInfo, mediaMetaData)}
          </style.title>
          <style.description>
            {getDescription(locale, publisherInfo, mediaMetaData)}
          </style.description>
        </style.card>
      </style.background>
    </style.root>
  )
}
