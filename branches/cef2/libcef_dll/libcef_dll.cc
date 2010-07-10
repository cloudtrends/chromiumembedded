// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "include/cef2.h"
#include "include/cef2_capi.h"
#include "cef_logging.h"
#include "cpptoc/browser_cpptoc.h"
#include "cpptoc/profile_cpptoc.h"
#include "cpptoc/tab_cpptoc.h"
#include "ctocpp/browser_listener_ctocpp.h"
#include "ctocpp/tab_listener_ctocpp.h"


CEF_EXPORT int cef_initialize(const wchar_t* chrome_path,
                              int multi_threaded_message_loop)
{
  std::wstring chromePath;
  if(chrome_path)
    chromePath = chrome_path;
  return CefInitialize(chromePath, (multi_threaded_message_loop?true:false));
}

CEF_EXPORT void cef_shutdown()
{
  CefShutdown();

#ifdef _DEBUG
  // Check that all wrapper objects have been destroyed
  DCHECK(CefBrowserCppToC::DebugObjCt == 0);
  DCHECK(CefProfileCppToC::DebugObjCt == 0);
  DCHECK(CefTabCppToC::DebugObjCt == 0);
  DCHECK(CefBrowserListenerCToCpp::DebugObjCt == 0);
  DCHECK(CefTabListenerCToCpp::DebugObjCt == 0);
#endif // _DEBUG
}

CEF_EXPORT void cef_do_message_loop_work()
{
  CefDoMessageLoopWork();
}

CEF_EXPORT struct _cef_profile_t* cef_create_profile(
    const wchar_t* user_profile)
{
  std::wstring userProfileStr;
  
  if(user_profile)
    userProfileStr = user_profile;
  
  CefRefPtr<CefProfile> profilePtr(CefCreateProfile(userProfileStr));
  if(profilePtr.get())
    return CefProfileCppToC::Wrap(profilePtr);
  return NULL;
}
