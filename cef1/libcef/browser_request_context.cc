// Copyright (c) 2008 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/browser_request_context.h"

#if defined(OS_WIN)
#include <winhttp.h>
#endif

#include "libcef/browser_file_system.h"
#include "libcef/browser_resource_loader_bridge.h"
#include "libcef/cef_context.h"
#include "libcef/cef_thread.h"

#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/file_util.h"
#include "base/thread_task_runner_handle.h"
#include "base/threading/worker_pool.h"
#include "build/build_config.h"
#include "chrome/browser/net/sqlite_persistent_cookie_store.h"
#include "net/base/cert_verifier.h"
#include "net/cookies/cookie_monster.h"
#include "net/dns/host_resolver.h"
#include "net/ftp/ftp_network_layer.h"
#include "net/http/http_auth_handler_factory.h"
#include "net/http/http_server_properties_impl.h"
#include "net/proxy/proxy_config_service.h"
#include "net/proxy/proxy_config_service_fixed.h"
#include "net/proxy/proxy_resolver.h"
#include "net/proxy/proxy_service.h"
#include "net/ssl/default_server_bound_cert_store.h"
#include "net/ssl/server_bound_cert_service.h"
#include "net/ssl/ssl_config_service_defaults.h"
#include "net/url_request/http_user_agent_settings.h"
#include "net/url_request/url_request_job_factory_impl.h"
#include "third_party/WebKit/Source/Platform/chromium/public/Platform.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebKit.h"
#include "webkit/blob/blob_storage_controller.h"
#include "webkit/blob/blob_url_request_job_factory.h"
#include "webkit/fileapi/file_system_context.h"
#include "webkit/fileapi/file_system_url_request_job_factory.h"
#include "webkit/user_agent/user_agent.h"

#if defined(OS_WIN)
#pragma comment(lib, "winhttp.lib")
#endif

namespace {

#if defined(OS_WIN)

// ProxyConfigService implementation that does nothing.
class ProxyConfigServiceNull : public net::ProxyConfigService {
 public:
  ProxyConfigServiceNull() {}
  virtual void AddObserver(Observer* observer) OVERRIDE {}
  virtual void RemoveObserver(Observer* observer) OVERRIDE {}
  virtual ProxyConfigService::ConfigAvailability
      GetLatestProxyConfig(net::ProxyConfig* config) OVERRIDE
      { return ProxyConfigService::CONFIG_VALID; }
  virtual void OnLazyPoll() OVERRIDE {}
};

#endif  // defined(OS_WIN)

// ProxyResolver implementation that forewards resolution to a CefProxyHandler.
class CefProxyResolver : public net::ProxyResolver {
 public:
  explicit CefProxyResolver(CefRefPtr<CefProxyHandler> handler)
    : ProxyResolver(false),
      handler_(handler) {}
  virtual ~CefProxyResolver() {}

  virtual int GetProxyForURL(const GURL& url,
                             net::ProxyInfo* results,
                             const net::CompletionCallback& callback,
                             RequestHandle* request,
                             const net::BoundNetLog& net_log) OVERRIDE {
    CefProxyInfo proxy_info;
    handler_->GetProxyForUrl(url.spec(), proxy_info);
    if (proxy_info.IsDirect())
      results->UseDirect();
    else if (proxy_info.IsNamedProxy())
      results->UseNamedProxy(proxy_info.ProxyList());
    else if (proxy_info.IsPacString())
      results->UsePacString(proxy_info.ProxyList());

    return net::OK;
  }

  virtual int SetPacScript(
      const scoped_refptr<net::ProxyResolverScriptData>& pac_script,
      const net::CompletionCallback& callback) OVERRIDE {
    return net::OK;
  }

  virtual void CancelRequest(RequestHandle request) OVERRIDE {}
  virtual net::LoadState GetLoadState(RequestHandle request) const OVERRIDE {
    return net::LOAD_STATE_IDLE;
  }
  virtual void CancelSetPacScript() OVERRIDE {}

 protected:
  CefRefPtr<CefProxyHandler> handler_;
};

net::ProxyConfigService* CreateProxyConfigService() {
#if defined(OS_POSIX) && !defined(OS_MACOSX)
  // Use no proxy to avoid ProxyConfigServiceLinux.
  // Enabling use of the ProxyConfigServiceLinux requires:
  // -Calling from a thread with a TYPE_UI MessageLoop,
  // -If at all possible, passing in a pointer to the IO thread's MessageLoop,
  // -Keep in mind that proxy auto configuration is also non-functional on linux
  //  in this context because of v8 threading issues.
  // TODO(port): rename "linux" to some nonspecific unix.
  return new net::ProxyConfigServiceFixed(net::ProxyConfig());
#else
  // Use the system proxy settings.
  return net::ProxyService::CreateSystemProxyConfigService(
      base::ThreadTaskRunnerHandle::Get(), NULL);
#endif
}

class CefHttpUserAgentSettings : public net::HttpUserAgentSettings {
 public:
  CefHttpUserAgentSettings() {}
  virtual ~CefHttpUserAgentSettings() {}

  // hard-code A-L and A-C for test shells
  virtual std::string GetAcceptLanguage() const OVERRIDE {
    return "en-us,en";
  }

  virtual std::string GetUserAgent(const GURL& url) const OVERRIDE {
    return webkit_glue::GetUserAgent(url);
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(CefHttpUserAgentSettings);
};

}  // namespace


BrowserRequestContext::BrowserRequestContext()
    : ALLOW_THIS_IN_INITIALIZER_LIST(storage_(this)) {
  Init(base::FilePath(), net::HttpCache::NORMAL, false);
}

BrowserRequestContext::BrowserRequestContext(
    const base::FilePath& cache_path,
    net::HttpCache::Mode cache_mode,
    bool no_proxy)
    : ALLOW_THIS_IN_INITIALIZER_LIST(storage_(this)) {
  Init(cache_path, cache_mode, no_proxy);
}

void BrowserRequestContext::Init(
    const base::FilePath& cache_path,
    net::HttpCache::Mode cache_mode,
    bool no_proxy) {
  SetCookieStoragePath(cache_path);

  storage_.set_server_bound_cert_service(new net::ServerBoundCertService(
      new net::DefaultServerBoundCertStore(NULL),
      base::WorkerPool::GetTaskRunner(true)));

  storage_.set_http_user_agent_settings(new CefHttpUserAgentSettings);

  CefRefPtr<CefApp> app = _Context->application();
  if (app.get()) {
    CefRefPtr<CefProxyHandler> handler = app->GetProxyHandler();
    if (handler) {
      // The client will provide proxy resolution.
      storage_.set_proxy_service(
          new net::ProxyService(CreateProxyConfigService(),
                                new CefProxyResolver(handler), NULL));
    }
  }

#if defined(OS_WIN)
  if (!proxy_service()) {
    const CefSettings& settings = _Context->settings();
    if (!settings.auto_detect_proxy_settings_enabled) {
      // Using the system proxy resolver on Windows when "Automatically detect
      // settings" (auto-detection) is checked under LAN Settings can hurt
      // resource loading performance because the call to WinHttpGetProxyForUrl
      // in proxy_resolver_winhttp.cc will block the IO thread.  This is
      // especially true for Windows 7 where auto-detection is checked by
      // default. To avoid slow resource loading on Windows we only use the
      // system proxy resolver if auto-detection is unchecked.
      WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ie_config = {0};
      if (WinHttpGetIEProxyConfigForCurrentUser(&ie_config)) {
        if (ie_config.fAutoDetect == TRUE) {
          storage_.set_proxy_service(
              net::ProxyService::CreateWithoutProxyResolver(
                  new ProxyConfigServiceNull(), NULL));
        }

        if (ie_config.lpszAutoConfigUrl)
          GlobalFree(ie_config.lpszAutoConfigUrl);
        if (ie_config.lpszProxy)
          GlobalFree(ie_config.lpszProxy);
        if (ie_config.lpszProxyBypass)
          GlobalFree(ie_config.lpszProxyBypass);
      }
    }
  }
#endif  // defined(OS_WIN)

  if (!proxy_service()) {
    storage_.set_proxy_service(
        net::ProxyService::CreateUsingSystemProxyResolver(
            CreateProxyConfigService(), 0, NULL));
  }

  storage_.set_host_resolver(net::HostResolver::CreateDefaultResolver(NULL));
  storage_.set_cert_verifier(net::CertVerifier::CreateDefault());
  storage_.set_ssl_config_service(new net::SSLConfigServiceDefaults);

  // Add support for single sign-on.
  url_security_manager_.reset(net::URLSecurityManager::Create(NULL, NULL));

  std::vector<std::string> supported_schemes;
  supported_schemes.push_back("basic");
  supported_schemes.push_back("digest");
  supported_schemes.push_back("ntlm");
  supported_schemes.push_back("negotiate");

  storage_.set_http_auth_handler_factory(
      net::HttpAuthHandlerRegistryFactory::Create(supported_schemes,
                                                  url_security_manager_.get(),
                                                  host_resolver(),
                                                  std::string(),
                                                  false,
                                                  false));
  storage_.set_http_server_properties(new net::HttpServerPropertiesImpl);

  net::HttpCache::DefaultBackend* backend = new net::HttpCache::DefaultBackend(
      cache_path.empty() ? net::MEMORY_CACHE : net::DISK_CACHE,
      cache_path, 0, BrowserResourceLoaderBridge::GetCacheThread());

  net::HttpNetworkSession::Params network_session_params;
  network_session_params.host_resolver = host_resolver();
  network_session_params.cert_verifier = cert_verifier();
  network_session_params.server_bound_cert_service =
      server_bound_cert_service();
  network_session_params.proxy_service = proxy_service();
  network_session_params.ssl_config_service = ssl_config_service();
  network_session_params.http_auth_handler_factory =
      http_auth_handler_factory();
  network_session_params.http_server_properties = http_server_properties();
  network_session_params.host_resolver = host_resolver();

  net::HttpCache* cache = new net::HttpCache(network_session_params, backend);
  cache->set_mode(cache_mode);
  storage_.set_http_transaction_factory(cache);

  storage_.set_ftp_transaction_factory(
      new net::FtpNetworkLayer(host_resolver()));

  net::URLRequestJobFactoryImpl* job_factory =
      new net::URLRequestJobFactoryImpl();
  job_factory_impl_ = job_factory;

  BrowserFileSystem* file_system = _Context->file_system();
  // Create the context if it doesn't already exist.
  file_system->CreateContext();
  if (file_system->file_system_context()) {
    job_factory->SetProtocolHandler(
        "filesystem",
        fileapi::CreateFileSystemProtocolHandler(
            file_system->file_system_context()));

    blob_storage_controller_.reset(new webkit_blob::BlobStorageController());
    job_factory->SetProtocolHandler(
        "blob",
        new webkit_blob::BlobProtocolHandler(
            blob_storage_controller_.get(),
            file_system->file_system_context(),
            CefThread::GetMessageLoopProxyForThread(CefThread::FILE)));
  }

  storage_.set_job_factory(job_factory);

  url_request_interceptor_.reset(
      BrowserResourceLoaderBridge::CreateRequestInterceptor());
}

BrowserRequestContext::~BrowserRequestContext() {
}

void BrowserRequestContext::SetCookieStoragePath(const base::FilePath& path) {
  REQUIRE_IOT();

  if (cookie_store() && ((cookie_store_path_.empty() && path.empty()) ||
                         cookie_store_path_ == path)) {
    // The path has not changed so don't do anything.
    return;
  }

  base::FilePath new_path = path;

  scoped_refptr<SQLitePersistentCookieStore> persistent_store;
  if (!new_path.empty()) {
    if (!file_util::PathExists(new_path) &&
        !file_util::CreateDirectory(new_path)) {
      NOTREACHED() << "Failed to create cookie storage directory";
      new_path.clear();
    } else {
      base::FilePath cookie_path =
          new_path.Append(FILE_PATH_LITERAL("Cookies"));
      persistent_store =
          new SQLitePersistentCookieStore(
              cookie_path,
              CefThread::GetMessageLoopProxyForThread(CefThread::IO),
              CefThread::GetMessageLoopProxyForThread(CefThread::FILE),
              false,
              NULL);
    }
  }

  // Set the new cookie store that will be used for all new requests. The old
  // cookie store, if any, will be automatically flushed and closed when no
  // longer referenced.
  storage_.set_cookie_store(
      new net::CookieMonster(persistent_store.get(), NULL));
  cookie_store_path_ = new_path;
}
