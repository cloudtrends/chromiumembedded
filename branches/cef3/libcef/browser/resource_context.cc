// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/browser/resource_context.h"
#include "libcef/browser/url_request_context_getter.h"

#include "content/browser/chrome_blob_storage_context.h"

CefResourceContext::CefResourceContext(
    CefURLRequestContextGetter* getter,
    ChromeBlobStorageContext* blob_storage_context)
    : getter_(getter),
      blob_storage_context_(blob_storage_context) {
}

CefResourceContext::~CefResourceContext() {
}

net::HostResolver* CefResourceContext::GetHostResolver() {
  return getter_->host_resolver();
}

net::URLRequestContext* CefResourceContext::GetRequestContext() {
  return getter_->GetURLRequestContext();
}

ChromeAppCacheService* CefResourceContext::GetAppCacheService() {
  return NULL;
}

webkit_database::DatabaseTracker* CefResourceContext::GetDatabaseTracker() {
  return NULL;
}

fileapi::FileSystemContext* CefResourceContext::GetFileSystemContext() {
  return NULL;
}

ChromeBlobStorageContext* CefResourceContext::GetBlobStorageContext() {
  return blob_storage_context_;
}

quota::QuotaManager* CefResourceContext::GetQuotaManager() {
  return NULL;
}

content::HostZoomMap* CefResourceContext::GetHostZoomMap() {
  return NULL;
}

MediaObserver* CefResourceContext::GetMediaObserver() {
  return NULL;
}

media_stream::MediaStreamManager*
    CefResourceContext::GetMediaStreamManager() {
  return NULL;
}

AudioManager* CefResourceContext::GetAudioManager() {
  return NULL;
}
