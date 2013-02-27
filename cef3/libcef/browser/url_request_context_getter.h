// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CEF_LIBCEF_BROWSER_URL_REQUEST_CONTEXT_GETTER_H_
#define CEF_LIBCEF_BROWSER_URL_REQUEST_CONTEXT_GETTER_H_
#pragma once

#include <set>
#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "net/url_request/url_request_context_getter.h"
#include "net/url_request/url_request_job_factory.h"

class CefRequestInterceptor;
class CefURLRequestContextProxy;
class MessageLoop;

namespace net {
class HostResolver;
class ProxyConfigService;
class URLRequestContextStorage;
class URLRequestJobFactory;
class URLRequestJobFactoryImpl;
class URLSecurityManager;
}

/*
// Classes used in network request processing:
//
// RC = net::URLRequestContext
//   Owns various network-related objects including the global cookie manager.
//
// RCP = CefURLRequestContextProxy
//    Creates the CSP and forwards requests to the objects owned by RC.
//
// CSP = CefCookieStoreProxy
//    Gives the CefCookieManager associated with CefBrowserHostImpl an
//    opportunity to handle cookie requests. Otherwise forwards requests via RC
//    to the global cookie manager.
//
// RCG = CefURLRequestContextGetter
//    Creates the RC and manages RCP lifespan.
//
// RCGP = CefURLRequestContextGetterProxy
//    Causes the RCG to create and destroy browser-specific RCPs.
//
// Relationship diagram:
//    ref = reference (scoped_refptr)
//    own = ownership (scoped_ptr)
//    ptr = raw pointer
//
//                          global cookie manager, etc...
//                                      ^
//                                      |
//                              /-own-> RC <-ptr-\
//                             /                  \
//                            / /<-ptr-\           \
//                           / /        \           \
// CefBrowserContext -ref-> RCG --own-> RCP --ref-> CSP
//                           ^          ^           /
//                          ref        ptr         /
//                           |        /           /
// CefBrowserHostImpl -ref-> RCGP----/           /
//             ^                                /
//              \-ref--------------------------/
*/

class CefURLRequestContextGetter : public net::URLRequestContextGetter {
 public:
  CefURLRequestContextGetter(
      MessageLoop* io_loop,
      MessageLoop* file_loop,
      scoped_ptr<net::URLRequestJobFactory::ProtocolHandler>
          blob_protocol_handler,
      scoped_ptr<net::URLRequestJobFactory::ProtocolHandler>
          file_system_protocol_handler,
      scoped_ptr<net::URLRequestJobFactory::ProtocolHandler>
          developer_protocol_handler);
  virtual ~CefURLRequestContextGetter();

  // net::URLRequestContextGetter implementation.
  virtual net::URLRequestContext* GetURLRequestContext() OVERRIDE;
  virtual scoped_refptr<base::SingleThreadTaskRunner>
      GetNetworkTaskRunner() const OVERRIDE;

  net::HostResolver* host_resolver();
  net::URLRequestJobFactoryImpl* job_factory_impl() const {
    return job_factory_impl_;
  }

  void SetCookieStoragePath(const base::FilePath& path,
                            bool persist_session_cookies);
  void SetCookieSupportedSchemes(const std::vector<std::string>& schemes);

  // Manage URLRequestContext proxy objects. It's important that proxy objects
  // not be destroyed while any in-flight URLRequests exist. These methods
  // manage that requirement.
  CefURLRequestContextProxy* CreateURLRequestContextProxy();
  void ReleaseURLRequestContextProxy(CefURLRequestContextProxy* proxy);

 private:
  void CreateProxyConfigService();

  MessageLoop* io_loop_;
  MessageLoop* file_loop_;

  scoped_ptr<net::ProxyConfigService> proxy_config_service_;
  scoped_ptr<CefRequestInterceptor> request_interceptor_;
  scoped_ptr<net::URLRequestContextStorage> storage_;
  scoped_ptr<net::URLRequestContext> url_request_context_;
  scoped_ptr<net::URLSecurityManager> url_security_manager_;
  scoped_ptr<net::URLRequestJobFactory::ProtocolHandler>
      blob_protocol_handler_;
  scoped_ptr<net::URLRequestJobFactory::ProtocolHandler>
      file_system_protocol_handler_;
  scoped_ptr<net::URLRequestJobFactory::ProtocolHandler>
      developer_protocol_handler_;
  net::URLRequestJobFactoryImpl* job_factory_impl_;

  typedef std::set<CefURLRequestContextProxy*> RequestContextProxySet;
  RequestContextProxySet url_request_context_proxies_;

  base::FilePath cookie_store_path_;
  std::vector<std::string> cookie_supported_schemes_;

  DISALLOW_COPY_AND_ASSIGN(CefURLRequestContextGetter);
};

#endif  // CEF_LIBCEF_BROWSER_URL_REQUEST_CONTEXT_GETTER_H_
