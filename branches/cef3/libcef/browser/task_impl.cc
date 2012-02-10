// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "include/cef_task.h"
#include "content/public/browser/browser_thread.h"
#include "base/bind.h"

using content::BrowserThread;

namespace {

int GetThreadId(CefThreadId threadId) {
  switch (threadId) {
  case TID_UI: return BrowserThread::UI;
  case TID_DB: return BrowserThread::DB;
  case TID_FILE: return BrowserThread::FILE;
  case TID_FILE_USER_BLOCKING: return BrowserThread::FILE_USER_BLOCKING;
  case TID_PROCESS_LAUNCHER: return BrowserThread::PROCESS_LAUNCHER;
  case TID_CACHE: return BrowserThread::CACHE;
  case TID_IO: return BrowserThread::IO;
  };
  NOTREACHED();
  return -1;
}

}  // namespace

bool CefCurrentlyOn(CefThreadId threadId) {
  int id = GetThreadId(threadId);
  if (id < 0)
    return false;

  return BrowserThread::CurrentlyOn(static_cast<BrowserThread::ID>(id));
}

bool CefPostTask(CefThreadId threadId, CefRefPtr<CefTask> task) {
  int id = GetThreadId(threadId);
  if (id < 0)
    return false;

  return BrowserThread::PostTask(static_cast<BrowserThread::ID>(id), FROM_HERE,
      base::Bind(&CefTask::Execute, task, threadId));
}

bool CefPostDelayedTask(CefThreadId threadId, CefRefPtr<CefTask> task,
                        int64 delay_ms) {
  int id = GetThreadId(threadId);
  if (id < 0)
    return false;

  return BrowserThread::PostDelayedTask(static_cast<BrowserThread::ID>(id),
      FROM_HERE, base::Bind(&CefTask::Execute, task, threadId), delay_ms);
}
