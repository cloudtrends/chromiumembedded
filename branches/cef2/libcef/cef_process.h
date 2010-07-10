// Copyright (c) 2010 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This interface is for managing the global services of the application. Each
// service is lazily created when requested the first time. The service getters
// will return NULL if the service is not available, so callers must check for
// this condition.

#ifndef _CEF_PROCESS_H
#define _CEF_PROCESS_H

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/message_loop.h"
#include "base/non_thread_safe.h"
#include "base/ref_counted.h"
#include "base/scoped_ptr.h"
#include "ipc/ipc_message.h"

namespace base {
class Thread;
class WaitableEvent;
}

class CefProcessUIThread;
class CefMessageLoopForUI;

// NOT THREAD SAFE, call only from the main thread.
// These functions shouldn't return NULL unless otherwise noted.
class CefProcess : public base::RefCounted<CefProcess>, public NonThreadSafe {
 public:
  CefProcess(bool multi_threaded_message_loop);
  virtual ~CefProcess();

  // Creates key child threads. We need to do this explicitly since
  // CefThread::PostTask silently deletes a posted task if the target message
  // loop isn't created.
  void CreateChildThreads() {
    ui_thread();
    io_thread();
    file_thread();
    process_launcher_thread();
  }

  CefProcessUIThread* ui_thread() {
    DCHECK(CalledOnValidThread());
    if (!created_ui_thread_)
      CreateUIThread();
    return ui_thread_.get();
  }

  // Necessary to perform work on the UI thread if started without a multi
  // threaded message loop.
  void DoMessageLoopIteration();

  // Returns the thread that we perform I/O coordination on (network requests,
  // communication with renderers, etc.
  // NOTE: You should ONLY use this to pass to IPC or other objects which must
  // need a MessageLoop*.  If you just want to post a task, use
  // CefThread::PostTask (or other variants) as they take care of checking
  // that a thread is still alive, race conditions, lifetime differences etc.
  // If you still must use this, need to check the return value for NULL.
  base::Thread* io_thread() {
    DCHECK(CalledOnValidThread());
    if (!created_io_thread_)
      CreateIOThread();
    return io_thread_.get();
  }

  // Returns the thread that we perform random file operations on. For code
  // that wants to do I/O operations (not network requests or even file: URL
  // requests), this is the thread to use to avoid blocking the UI thread.
  // It might be nicer to have a thread pool for this kind of thing.
  base::Thread* file_thread() {
    DCHECK(CalledOnValidThread());
    if (!created_file_thread_)
      CreateFileThread();
    return file_thread_.get();
  }

  virtual base::Thread* process_launcher_thread() {
    DCHECK(CalledOnValidThread());
    if (!created_process_launcher_thread_)
      CreateProcessLauncherThread();
    return process_launcher_thread_.get();
  }

#if defined(IPC_MESSAGE_LOG_ENABLED)
  // Enable or disable IPC logging for the browser, all processes
  // derived from ChildProcess (plugin etc), and all
  // renderers.
  void SetIPCLoggingEnabled(bool enable);
#endif

 private:
  void CreateUIThread();
  void CreateIOThread();
  void CreateFileThread();
  void CreateProcessLauncherThread();

#if defined(IPC_MESSAGE_LOG_ENABLED)
  void SetIPCLoggingEnabledForChildProcesses(bool enabled);
#endif

  bool multi_threaded_message_loop_;

  bool created_ui_thread_;
  scoped_ptr<CefProcessUIThread> ui_thread_;
  scoped_ptr<CefMessageLoopForUI> ui_message_loop_;

  bool created_io_thread_;
  scoped_ptr<base::Thread> io_thread_;

  bool created_file_thread_;
  scoped_ptr<base::Thread> file_thread_;

  bool created_process_launcher_thread_;
  scoped_ptr<base::Thread> process_launcher_thread_;

  DISALLOW_COPY_AND_ASSIGN(CefProcess);
};

extern CefProcess* g_cef_process;

#endif  // _CEF_PROCESS_H
