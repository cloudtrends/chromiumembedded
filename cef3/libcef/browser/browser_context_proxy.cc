// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/browser/browser_context_proxy.h"

#include "libcef/browser/content_browser_client.h"
#include "libcef/browser/download_manager_delegate.h"
#include "libcef/browser/thread_util.h"
#include "libcef/browser/url_request_context_getter.h"
#include "libcef/browser/url_request_context_getter_proxy.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/threading/thread.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/storage_partition.h"

using content::BrowserThread;

class CefBrowserContextProxy::CefResourceContext : public content::ResourceContext {
 public:
  CefResourceContext() : getter_(NULL) {}
  virtual ~CefResourceContext() {}

  // ResourceContext implementation:
  virtual net::HostResolver* GetHostResolver() OVERRIDE {
    CHECK(getter_);
    return getter_->GetHostResolver();
  }
  virtual net::URLRequestContext* GetRequestContext() OVERRIDE {
    CHECK(getter_);
    return getter_->GetURLRequestContext();
  }
  virtual bool AllowMicAccess(const GURL& origin) OVERRIDE {
    return true;
  }
  virtual bool AllowCameraAccess(const GURL& origin) OVERRIDE {
    return true;
  }

  void set_url_request_context_getter(CefURLRequestContextGetterProxy* getter) {
    getter_ = getter;
  }

 private:
  CefURLRequestContextGetterProxy* getter_;

  DISALLOW_COPY_AND_ASSIGN(CefResourceContext);
};

CefBrowserContextProxy::CefBrowserContextProxy(
    CefRefPtr<CefRequestContextHandler> handler,
    CefBrowserContext* parent)
    : refct_(0),
      handler_(handler),
      parent_(parent),
      resource_context_(new CefResourceContext) {
}

CefBrowserContextProxy::~CefBrowserContextProxy() {
  if (resource_context_.get()) {
    BrowserThread::DeleteSoon(
        BrowserThread::IO, FROM_HERE, resource_context_.release());
  }
}

base::FilePath CefBrowserContextProxy::GetPath() const {
  return parent_->GetPath();
}

bool CefBrowserContextProxy::IsOffTheRecord() const {
  return parent_->IsOffTheRecord();
}

content::DownloadManagerDelegate*
    CefBrowserContextProxy::GetDownloadManagerDelegate() {
  DCHECK(!download_manager_delegate_.get());

  content::DownloadManager* manager = BrowserContext::GetDownloadManager(this);
  download_manager_delegate_.reset(new CefDownloadManagerDelegate(manager));
  return download_manager_delegate_.get();
}

net::URLRequestContextGetter* CefBrowserContextProxy::GetRequestContext() {
  CEF_REQUIRE_UIT();
  return GetDefaultStoragePartition(this)->GetURLRequestContext();
}

net::URLRequestContextGetter*
    CefBrowserContextProxy::GetRequestContextForRenderProcess(
        int renderer_child_id) {
  return GetRequestContext();
}

net::URLRequestContextGetter*
    CefBrowserContextProxy::GetMediaRequestContext() {
  return GetRequestContext();
}

net::URLRequestContextGetter*
    CefBrowserContextProxy::GetMediaRequestContextForRenderProcess(
        int renderer_child_id)  {
  return GetRequestContext();
}

net::URLRequestContextGetter*
    CefBrowserContextProxy::GetMediaRequestContextForStoragePartition(
        const base::FilePath& partition_path,
        bool in_memory) {
  return GetRequestContext();
}

content::ResourceContext* CefBrowserContextProxy::GetResourceContext() {
  return resource_context_.get();
}

content::BrowserPluginGuestManager* CefBrowserContextProxy::GetGuestManager() {
  return parent_->GetGuestManager();
}

quota::SpecialStoragePolicy*
    CefBrowserContextProxy::GetSpecialStoragePolicy() {
  return parent_->GetSpecialStoragePolicy();
}

content::PushMessagingService*
    CefBrowserContextProxy::GetPushMessagingService() {
  return parent_->GetPushMessagingService();
}

net::URLRequestContextGetter* CefBrowserContextProxy::CreateRequestContext(
    content::ProtocolHandlerMap* protocol_handlers,
    content::URLRequestInterceptorScopedVector request_interceptors) {
  DCHECK(!url_request_getter_);
  url_request_getter_ =
      new CefURLRequestContextGetterProxy(handler_,
          static_cast<CefURLRequestContextGetter*>(
              CefContentBrowserClient::Get()->request_context().get()));
  resource_context_->set_url_request_context_getter(url_request_getter_.get());
  return url_request_getter_.get();
}

net::URLRequestContextGetter*
    CefBrowserContextProxy::CreateRequestContextForStoragePartition(
        const base::FilePath& partition_path,
        bool in_memory,
        content::ProtocolHandlerMap* protocol_handlers,
        content::URLRequestInterceptorScopedVector request_interceptors) {
  return NULL;
}
