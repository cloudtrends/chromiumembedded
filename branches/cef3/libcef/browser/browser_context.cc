// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/browser/browser_context.h"
#include "libcef/browser/browser_main.h"
#include "libcef/browser/download_manager_delegate.h"
#include "libcef/browser/resource_context.h"
#include "libcef/browser/url_request_context_getter.h"

#include "base/bind.h"
#include "base/environment.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/threading/thread.h"
#include "content/browser/appcache/chrome_appcache_service.h"
#include "content/browser/chrome_blob_storage_context.h"
#include "content/browser/download/download_manager_impl.h"
#include "content/browser/file_system/browser_file_system_helper.h"
#include "content/browser/host_zoom_map_impl.h"
#include "content/browser/in_process_webkit/webkit_context.h"
#include "content/browser/speech/speech_input_preferences.h"
#include "content/browser/ssl/ssl_host_state.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/geolocation_permission_context.h"
#include "webkit/database/database_tracker.h"
#include "webkit/quota/quota_manager.h"

#if defined(OS_WIN)
#include "base/base_paths_win.h"
#elif defined(OS_LINUX)
#include "base/nix/xdg_util.h"
#endif

using content::BrowserThread;

namespace {

#if defined(OS_LINUX)
const char kDotConfigDir[] = ".config";
const char kXdgConfigHomeEnvVar[] = "XDG_CONFIG_HOME";
#endif

class CefGeolocationPermissionContext :
    public content::GeolocationPermissionContext {
 public:
  CefGeolocationPermissionContext() {
  }

  // GeolocationPermissionContext implementation).
  virtual void RequestGeolocationPermission(
      int render_process_id,
      int render_view_id,
      int bridge_id,
      const GURL& requesting_frame,
      base::Callback<void(bool)> callback)  // NOLINT(readability/function)
      OVERRIDE {
    NOTIMPLEMENTED();
  }

  virtual void CancelGeolocationPermissionRequest(
      int render_process_id,
      int render_view_id,
      int bridge_id,
      const GURL& requesting_frame) OVERRIDE {
    NOTIMPLEMENTED();
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(CefGeolocationPermissionContext);
};

class CefSpeechInputPreferences : public SpeechInputPreferences {
 public:
  CefSpeechInputPreferences() {
  }

  // SpeechInputPreferences implementation.
  virtual bool filter_profanities() const OVERRIDE {
    return false;
  }

  virtual void set_filter_profanities(bool filter_profanities) OVERRIDE {
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(CefSpeechInputPreferences);
};

}  // namespace

CefBrowserContext::CefBrowserContext(
    CefBrowserMainParts* main_parts)
    : main_parts_(main_parts) {
  CreateQuotaManagerAndClients();
}

CefBrowserContext::~CefBrowserContext() {
  if (resource_context_.get()) {
    BrowserThread::DeleteSoon(
      BrowserThread::IO, FROM_HERE, resource_context_.release());
  }
}

FilePath CefBrowserContext::GetPath() {
  if (!path_.empty())
    return path_;

#if defined(OS_WIN)
  CHECK(PathService::Get(base::DIR_LOCAL_APP_DATA, &path_));
  path_ = path_.Append(std::wstring(L"cef_data"));
#elif defined(OS_LINUX)
  scoped_ptr<base::Environment> env(base::Environment::Create());
  FilePath config_dir(base::nix::GetXDGDirectory(env.get(),
                                                 kXdgConfigHomeEnvVar,
                                                 kDotConfigDir));
  path_ = config_dir.Append("cef_data");
#else
  NOTIMPLEMENTED();
#endif

  if (!file_util::PathExists(path_))
    file_util::CreateDirectory(path_);

  return path_;
}

bool CefBrowserContext::IsOffTheRecord()  {
  return false;
}

SSLHostState* CefBrowserContext::GetSSLHostState()  {
  if (!ssl_host_state_.get())
    ssl_host_state_.reset(new SSLHostState());
  return ssl_host_state_.get();
}

content::DownloadManager* CefBrowserContext::GetDownloadManager()  {
  if (!download_manager_.get()) {
    download_manager_delegate_ = new CefDownloadManagerDelegate();
    download_manager_ = new DownloadManagerImpl(download_manager_delegate_,
                                                NULL);
    download_manager_delegate_->SetDownloadManager(download_manager_.get());
    download_manager_->Init(this);
  }
  return download_manager_.get();
}

net::URLRequestContextGetter* CefBrowserContext::GetRequestContext()  {
  if (!url_request_getter_) {
    url_request_getter_ = new CefURLRequestContextGetter(
        GetPath(),
        BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::IO),
        BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::FILE));
  }
  return url_request_getter_;
}

net::URLRequestContextGetter*
    CefBrowserContext::GetRequestContextForRenderProcess(
        int renderer_child_id)  {
  return GetRequestContext();
}

net::URLRequestContextGetter*
    CefBrowserContext::GetRequestContextForMedia()  {
  return GetRequestContext();
}

content::ResourceContext* CefBrowserContext::GetResourceContext()  {
  if (!resource_context_.get()) {
    resource_context_.reset(new CefResourceContext(
        static_cast<CefURLRequestContextGetter*>(GetRequestContext()),
        GetBlobStorageContext()));
  }
  return resource_context_.get();
}

content::HostZoomMap* CefBrowserContext::GetHostZoomMap()  {
  if (!host_zoom_map_)
    host_zoom_map_ = content::HostZoomMap::Create();
  return host_zoom_map_.get();
}

content::GeolocationPermissionContext*
CefBrowserContext::GetGeolocationPermissionContext()  {
  if (!geolocation_permission_context_) {
    geolocation_permission_context_ =
        new CefGeolocationPermissionContext();
  }
  return geolocation_permission_context_;
}

SpeechInputPreferences* CefBrowserContext::GetSpeechInputPreferences() {
  if (!speech_input_preferences_.get())
    speech_input_preferences_ = new CefSpeechInputPreferences();
  return speech_input_preferences_.get();
}

bool CefBrowserContext::DidLastSessionExitCleanly()  {
  return true;
}

quota::QuotaManager* CefBrowserContext::GetQuotaManager()  {
  CreateQuotaManagerAndClients();
  return quota_manager_.get();
}

WebKitContext* CefBrowserContext::GetWebKitContext()  {
  CreateQuotaManagerAndClients();
  return webkit_context_.get();
}

webkit_database::DatabaseTracker* CefBrowserContext::GetDatabaseTracker()  {
  CreateQuotaManagerAndClients();
  return db_tracker_;
}

ChromeBlobStorageContext* CefBrowserContext::GetBlobStorageContext()  {
  if (!blob_storage_context_) {
    blob_storage_context_ = new ChromeBlobStorageContext();
    BrowserThread::PostTask(
        BrowserThread::IO, FROM_HERE,
        base::Bind(
            &ChromeBlobStorageContext::InitializeOnIOThread,
            blob_storage_context_.get()));
  }
  return blob_storage_context_;
}

ChromeAppCacheService* CefBrowserContext::GetAppCacheService()  {
  CreateQuotaManagerAndClients();
  return appcache_service_;
}

fileapi::FileSystemContext* CefBrowserContext::GetFileSystemContext()  {
  CreateQuotaManagerAndClients();
  return file_system_context_.get();
}

void CefBrowserContext::CreateQuotaManagerAndClients() {
  if (quota_manager_.get())
    return;
  quota_manager_ = new quota::QuotaManager(
      IsOffTheRecord(),
      GetPath(),
      BrowserThread::GetMessageLoopProxyForThread(BrowserThread::IO),
      BrowserThread::GetMessageLoopProxyForThread(BrowserThread::DB),
      NULL);

  file_system_context_ = CreateFileSystemContext(
      GetPath(), IsOffTheRecord(), NULL, quota_manager_->proxy());
  db_tracker_ = new webkit_database::DatabaseTracker(
      GetPath(), IsOffTheRecord(), false, NULL, quota_manager_->proxy(),
      BrowserThread::GetMessageLoopProxyForThread(BrowserThread::FILE));
  webkit_context_ = new WebKitContext(
      IsOffTheRecord(), GetPath(), NULL, false, quota_manager_->proxy(),
      BrowserThread::GetMessageLoopProxyForThread(
          BrowserThread::WEBKIT_DEPRECATED));
  appcache_service_ = new ChromeAppCacheService(quota_manager_->proxy());
  scoped_refptr<quota::SpecialStoragePolicy> special_storage_policy;
  BrowserThread::PostTask(
    BrowserThread::IO, FROM_HERE,
    base::Bind(
        &ChromeAppCacheService::InitializeOnIOThread,
        appcache_service_.get(),
        IsOffTheRecord()
            ? FilePath() : GetPath().Append(FILE_PATH_LITERAL("AppCache")),
        GetResourceContext(),
        special_storage_policy));
}
