// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CEF_LIBCEF_BROWSER_BROWSER_CONTEXT_H_
#define CEF_LIBCEF_BROWSER_BROWSER_CONTEXT_H_
#pragma once

#include "base/compiler_specific.h"
#include "base/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_context.h"

class DownloadManager;
class SSLHostState;

namespace content {
class DownloadManagerDelegate;
class HostZoomMap;
class ResourceContext;
}

class CefBrowserMainParts;
class CefDownloadManagerDelegate;

class CefBrowserContext : public content::BrowserContext {
 public:
  explicit CefBrowserContext(CefBrowserMainParts* main_parts);
  virtual ~CefBrowserContext();

  // BrowserContext methods.
  virtual FilePath GetPath() OVERRIDE;
  virtual bool IsOffTheRecord() OVERRIDE;
  virtual SSLHostState* GetSSLHostState() OVERRIDE;
  virtual content::DownloadManager* GetDownloadManager() OVERRIDE;
  virtual net::URLRequestContextGetter* GetRequestContext() OVERRIDE;
  virtual net::URLRequestContextGetter* GetRequestContextForRenderProcess(
      int renderer_child_id) OVERRIDE;
  virtual net::URLRequestContextGetter* GetRequestContextForMedia() OVERRIDE;
  virtual content::ResourceContext* GetResourceContext() OVERRIDE;
  virtual content::HostZoomMap* GetHostZoomMap() OVERRIDE;
  virtual content::GeolocationPermissionContext*
      GetGeolocationPermissionContext() OVERRIDE;
  virtual SpeechInputPreferences* GetSpeechInputPreferences() OVERRIDE;
  virtual bool DidLastSessionExitCleanly() OVERRIDE;
  virtual quota::QuotaManager* GetQuotaManager() OVERRIDE;
  virtual WebKitContext* GetWebKitContext() OVERRIDE;
  virtual webkit_database::DatabaseTracker* GetDatabaseTracker() OVERRIDE;
  virtual ChromeBlobStorageContext* GetBlobStorageContext() OVERRIDE;
  virtual ChromeAppCacheService* GetAppCacheService() OVERRIDE;
  virtual fileapi::FileSystemContext* GetFileSystemContext() OVERRIDE;

 private:
  void CreateQuotaManagerAndClients();

  FilePath path_;
  scoped_ptr<content::ResourceContext> resource_context_;
  scoped_ptr<SSLHostState> ssl_host_state_;
  scoped_refptr<CefDownloadManagerDelegate> download_manager_delegate_;
  scoped_refptr<content::DownloadManager> download_manager_;
  scoped_refptr<net::URLRequestContextGetter> url_request_getter_;
  scoped_refptr<content::HostZoomMap> host_zoom_map_;
  scoped_refptr<content::GeolocationPermissionContext>
      geolocation_permission_context_;
  scoped_refptr<SpeechInputPreferences> speech_input_preferences_;
  scoped_refptr<WebKitContext> webkit_context_;
  scoped_refptr<ChromeAppCacheService> appcache_service_;
  scoped_refptr<webkit_database::DatabaseTracker> db_tracker_;
  scoped_refptr<fileapi::FileSystemContext> file_system_context_;
  scoped_refptr<quota::QuotaManager> quota_manager_;
  scoped_refptr<ChromeBlobStorageContext> blob_storage_context_;

  CefBrowserMainParts* main_parts_;

  DISALLOW_COPY_AND_ASSIGN(CefBrowserContext);
};

#endif  // CEF_LIBCEF_BROWSER_BROWSER_CONTEXT_H_
