// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "include/cef2.h"
#include "include/cef2_capi.h"
#include "libcef_dll/cpptoc/browser_listener_cpptoc.h"
#include "libcef_dll/cpptoc/tab_listener_cpptoc.h"
#include "libcef_dll/ctocpp/browser_ctocpp.h"
#include "libcef_dll/ctocpp/profile_ctocpp.h"
#include "libcef_dll/ctocpp/tab_ctocpp.h"


bool CefInitialize(const std::wstring& chrome_path,
                   bool multi_threaded_message_loop)
{
  return cef_initialize(chrome_path.c_str(), multi_threaded_message_loop)?
      true:false;
}

void CefShutdown()
{
  cef_shutdown();

#ifdef _DEBUG
  // Check that all wrapper objects have been destroyed
  DCHECK(CefBrowserListenerCppToC::DebugObjCt == 0);
  DCHECK(CefTabListenerCppToC::DebugObjCt == 0);
  DCHECK(CefBrowserCToCpp::DebugObjCt == 0);
  DCHECK(CefProfileCToCpp::DebugObjCt == 0);
  DCHECK(CefTabCToCpp::DebugObjCt == 0);
#endif // _DEBUG
}

void CefDoMessageLoopWork()
{
  cef_do_message_loop_work();
}

CefRefPtr<CefProfile> CefCreateProfile(const std::wstring& user_profile)
{
  cef_profile_t* impl = cef_create_profile(user_profile.c_str());
  if(impl)
    return CefProfileCToCpp::Wrap(impl);
  return NULL;
}
