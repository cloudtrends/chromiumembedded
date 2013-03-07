// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef CEF_LIBCEF_BROWSER_CONTEXT_H_
#define CEF_LIBCEF_BROWSER_CONTEXT_H_
#pragma once

#include <list>
#include <map>
#include <string>

#include "include/cef_app.h"
#include "include/cef_base.h"

#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/scoped_ptr.h"
#include "base/threading/platform_thread.h"
#include "net/proxy/proxy_config_service.h"
#include "net/url_request/url_request_context_getter.h"

namespace base {
class WaitableEvent;
}

namespace content {
class ContentMainRunner;
}

class CefBrowserContext;
class CefBrowserHostImpl;
class CefDevToolsDelegate;
class CefMainDelegate;
class CefTraceSubscriber;
class PrefService;

class CefContext : public CefBase {
 public:
  typedef std::list<CefRefPtr<CefBrowserHostImpl> > BrowserList;

  CefContext();
  ~CefContext();

  // These methods will be called on the main application thread.
  bool Initialize(const CefMainArgs& args,
                  const CefSettings& settings,
                  CefRefPtr<CefApp> application);
  void Shutdown();

  // Returns true if the current thread is the initialization thread.
  bool OnInitThread();

  // Returns true if the context is initialized.
  bool initialized() { return initialized_; }

  // Returns true if the context is shutting down.
  bool shutting_down() { return shutting_down_; }

  CefRefPtr<CefBrowserHostImpl> GetBrowserByRoutingID(int render_process_id,
                                                      int render_view_id);

  // Retrieve the path at which cache data will be stored on disk.  If empty,
  // cache data will be stored in-memory.
  const base::FilePath& cache_path() const { return cache_path_; }

  const CefSettings& settings() const { return settings_; }
  CefRefPtr<CefApp> application() const;
  CefBrowserContext* browser_context() const;
  CefDevToolsDelegate* devtools_delegate() const;
  PrefService* pref_service() const;

  scoped_refptr<net::URLRequestContextGetter> request_context() const {
    return request_context_;
  }
  void set_request_context(scoped_refptr<net::URLRequestContextGetter> context) {
    request_context_ = context;
  }

  // Passes ownership.
  scoped_ptr<net::ProxyConfigService> proxy_config_service() const;
  
  CefTraceSubscriber* GetTraceSubscriber();

 private:
  void OnContextInitialized();

  // Performs shutdown actions that need to occur on the UI thread before any
  // threads are destroyed.
  void FinishShutdownOnUIThread(base::WaitableEvent* uithread_shutdown_event);

  // Destroys the main runner and related objects.
  void FinalizeShutdown();

  // Track context state.
  bool initialized_;
  bool shutting_down_;

  // The thread on which the context was initialized.
  base::PlatformThreadId init_thread_id_;

  CefSettings settings_;
  base::FilePath cache_path_;
  base::ScopedTempDir cache_temp_dir_;

  scoped_ptr<CefMainDelegate> main_delegate_;
  scoped_ptr<content::ContentMainRunner> main_runner_;
  scoped_ptr<CefTraceSubscriber> trace_subscriber_;

  scoped_refptr<net::URLRequestContextGetter> request_context_;

  IMPLEMENT_REFCOUNTING(CefContext);
  IMPLEMENT_LOCKING(CefContext);
};

// Global context object pointer.
extern CefRefPtr<CefContext> _Context;

// Helper macro that returns true if the global context is in a valid state.
#define CONTEXT_STATE_VALID() \
    (_Context.get() && _Context->initialized() && !_Context->shutting_down())

#endif  // CEF_LIBCEF_BROWSER_CONTEXT_H_
