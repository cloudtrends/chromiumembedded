// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CEF_LIBCEF_BROWSER_RESOURCE_CONTEXT_H_
#define CEF_LIBCEF_BROWSER_RESOURCE_CONTEXT_H_
#pragma once

#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "content/public/browser/resource_context.h"

class ChromeBlobStorageContext;
class DownloadIdFactory;

class CefURLRequestContextGetter;

class CefResourceContext : public content::ResourceContext {
 public:
  CefResourceContext(
      CefURLRequestContextGetter* getter,
      ChromeBlobStorageContext* blob_storage_context);
  virtual ~CefResourceContext();

 private:
  // ResourceContext methods.
  virtual net::HostResolver* GetHostResolver() OVERRIDE;
  virtual net::URLRequestContext* GetRequestContext() OVERRIDE;
  virtual ChromeAppCacheService* GetAppCacheService() OVERRIDE;
  virtual webkit_database::DatabaseTracker* GetDatabaseTracker() OVERRIDE;
  virtual fileapi::FileSystemContext* GetFileSystemContext() OVERRIDE;
  virtual ChromeBlobStorageContext* GetBlobStorageContext() OVERRIDE;
  virtual quota::QuotaManager* GetQuotaManager() OVERRIDE;
  virtual content::HostZoomMap* GetHostZoomMap() OVERRIDE;
  virtual MediaObserver* GetMediaObserver() OVERRIDE;
  virtual media_stream::MediaStreamManager* GetMediaStreamManager() OVERRIDE;
  virtual AudioManager* GetAudioManager() OVERRIDE;

  scoped_refptr<CefURLRequestContextGetter> getter_;
  scoped_refptr<ChromeBlobStorageContext> blob_storage_context_;

  DISALLOW_COPY_AND_ASSIGN(CefResourceContext);
};

#endif  // CEF_LIBCEF_BROWSER_RESOURCE_CONTEXT_H_
