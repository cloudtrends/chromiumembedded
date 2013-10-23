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

#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/scoped_ptr.h"
#include "base/threading/platform_thread.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"

namespace base {
class WaitableEvent;
}

namespace content {
class ContentMainRunner;
}

namespace printing {
class PrintJobManager;
}

class CefBrowserHostImpl;
class CefMainDelegate;
class CefTraceSubscriber;

class CefContext : public content::NotificationObserver {
 public:
  typedef std::list<CefRefPtr<CefBrowserHostImpl> > BrowserList;

  CefContext();
  ~CefContext();

  // Returns the singleton CefContext instance.
  static CefContext* Get();

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

  // Retrieve the path at which cache data will be stored on disk.
  const base::FilePath& cache_path() const { return cache_path_; }

  const CefSettings& settings() const { return settings_; }

  printing::PrintJobManager* print_job_manager() const {
    return print_job_manager_.get();
  }

  CefTraceSubscriber* GetTraceSubscriber();

 private:
  void OnContextInitialized();

  // Performs shutdown actions that need to occur on the UI thread before any
  // threads are destroyed.
  void FinishShutdownOnUIThread(base::WaitableEvent* uithread_shutdown_event);

  // Destroys the main runner and related objects.
  void FinalizeShutdown();

  // NotificationObserver implementation.
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

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

  // Only accessed on the UI Thread.
  scoped_ptr<content::NotificationRegistrar> registrar_;
  scoped_ptr<printing::PrintJobManager> print_job_manager_;
};

// Helper macro that returns true if the global context is in a valid state.
#define CONTEXT_STATE_VALID() \
    (CefContext::Get() && CefContext::Get()->initialized() && \
     !CefContext::Get()->shutting_down())

#endif  // CEF_LIBCEF_BROWSER_CONTEXT_H_
