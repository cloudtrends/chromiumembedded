// Copyright (c) 2010 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cef_process_ui_thread.h"
#include "base/command_line.h"
#include "base/i18n/icu_util.h"
#include "build/build_config.h"

#if defined(OS_WIN)
#include <Objbase.h>
#endif

CefProcessUIThread::CefProcessUIThread()
      : CefThread(CefThread::UI) {}

CefProcessUIThread::CefProcessUIThread(MessageLoop* message_loop)
      : CefThread(CefThread::UI, message_loop) {}

CefProcessUIThread::~CefProcessUIThread() {
  // We cannot rely on our base class to stop the thread since we want our
  // CleanUp function to run.
  Stop();
}

void CefProcessUIThread::Init() {
#if defined(OS_WIN)
  // Initializes the COM library on the current thread.
  CoInitialize(NULL);
#endif

  // Initialize the global CommandLine object.
  CommandLine::Init(0, NULL);

  // Load ICU data tables.
  bool ret = icu_util::Initialize();
  DCHECK(ret);
}

void CefProcessUIThread::CleanUp() {
  // Flush any remaining messages.  This ensures that any accumulated
  // Task objects get destroyed before we exit, which avoids noise in
  // purify leak-test results.
  MessageLoop::current()->RunAllPending();

#if defined(OS_WIN)
  // Closes the COM library on the current thread. CoInitialize must
  // be balanced by a corresponding call to CoUninitialize.
  CoUninitialize();
#endif
}
