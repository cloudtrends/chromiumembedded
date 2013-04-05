// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "include/cef_app.h"
#include "include/cef_task.h"
#include "tests/unittests/test_suite.h"
#include "base/bind.h"
#include "base/command_line.h"
#include "base/threading/thread.h"

static void OnRegisterCustomSchemes(
    CefRefPtr<CefSchemeRegistrar> registrar) {
  // Bring in the scheme handler tests.
  extern void RegisterSchemeHandlerCustomSchemes(
      CefRefPtr<CefSchemeRegistrar> registrar);
  RegisterSchemeHandlerCustomSchemes(registrar);

  // Bring in the cookie tests.
  extern void RegisterCookieCustomSchemes(
      CefRefPtr<CefSchemeRegistrar> registrar);
  RegisterCookieCustomSchemes(registrar);
}

namespace {

// Thread used to run the test suite.
class CefTestThread : public base::Thread {
 public:
  explicit CefTestThread(CefTestSuite* test_suite)
    : base::Thread("test_thread"),
      test_suite_(test_suite) {
  }

  void RunTests() {
    // Run the test suite.
    retval_ = test_suite_->Run();

    // Quit the CEF message loop.
    class QuitTask : public CefTask {
     public:
      QuitTask() {}
      virtual void Execute(CefThreadId threadId) { CefQuitMessageLoop(); }
      IMPLEMENT_REFCOUNTING(QuitTask);
    };
    CefPostTask(TID_UI, new QuitTask());
  }

  int retval() { return retval_; }

 protected:
  CefTestSuite* test_suite_;
  int retval_;
};

class ClientApp : public CefApp {
 public:
  ClientApp() {}

  virtual void OnRegisterCustomSchemes(
      CefRefPtr<CefSchemeRegistrar> registrar) OVERRIDE {
    ::OnRegisterCustomSchemes(registrar);
  }

 protected:
  IMPLEMENT_REFCOUNTING(ClientApp);
};

}  // namespace


int main(int argc, char** argv) {
  // Initialize the CommandLine object.
  CefTestSuite::InitCommandLine(argc, argv);

  CefSettings settings;
  CefTestSuite::GetSettings(settings);

#if defined(OS_MACOSX)
  // Platform-specific initialization.
  extern void PlatformInit();
  PlatformInit();
#endif

  // Initialize CEF.
  CefInitialize(settings, new ClientApp());

  // Create the test suite object.
  CefTestSuite test_suite(argc, argv);

  int retval;

  if (settings.multi_threaded_message_loop) {
    // Run the test suite on the main thread.
    retval = test_suite.Run();
  } else {
    // Create the test thread.
    scoped_ptr<CefTestThread> thread;
    thread.reset(new CefTestThread(&test_suite));
    if (!thread->Start())
      return 1;

    // Run the test suite on the test thread.
    thread->message_loop()->PostTask(FROM_HERE,
        base::Bind(&CefTestThread::RunTests, base::Unretained(thread.get())));

    // Run the CEF message loop.
    CefRunMessageLoop();

    // The test suite has completed.
    retval = thread->retval();

    // Terminate the test thread.
    thread.reset();
  }

  // Shut down CEF.
  CefShutdown();

#if defined(OS_MACOSX)
  // Platform-specific cleanup.
  extern void PlatformCleanup();
  PlatformCleanup();
#endif

  return retval;
}
