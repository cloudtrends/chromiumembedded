// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// ---------------------------------------------------------------------------
//
// A portion of this file was generated by the CEF translator tool.  When
// making changes by hand only do so within the body of existing function
// implementations. See the translator.README.txt file in the tools directory
// for more information.
//

#include "libcef_dll/cpptoc/browser_cpptoc.h"
#include "libcef_dll/cpptoc/frame_cpptoc.h"
#include "libcef_dll/cpptoc/v8context_cpptoc.h"
#include "libcef_dll/cpptoc/v8value_cpptoc.h"


// GLOBAL FUNCTIONS - Body may be edited by hand.

CEF_EXPORT cef_v8context_t* cef_v8context_get_current_context()
{
  CefRefPtr<CefV8Context> 
      contextPtr( CefV8Context::GetCurrentContext() );
  if(contextPtr.get())
    return CefV8ContextCppToC::Wrap(contextPtr);
  return NULL;
}

CEF_EXPORT cef_v8context_t* cef_v8context_get_entered_context()
{
  CefRefPtr<CefV8Context> 
      contextPtr( CefV8Context::GetEnteredContext() );
  if(contextPtr.get())
    return CefV8ContextCppToC::Wrap(contextPtr);
  return NULL;
}

CEF_EXPORT int cef_v8context_in_context()
{
  return CefV8Context::InContext();
}


// MEMBER FUNCTIONS - Body may be edited by hand.

cef_browser_t* CEF_CALLBACK v8context_get_browser(struct _cef_v8context_t* self)
{
  DCHECK(self);
  if(!self)
    return NULL;

  CefRefPtr<CefV8Context> contextPtr = CefV8ContextCppToC::Get(self);
  CefRefPtr<CefBrowser> browserPtr = contextPtr->GetBrowser();
  if(browserPtr.get())
    return CefBrowserCppToC::Wrap(browserPtr);
  return NULL;
}

cef_frame_t* CEF_CALLBACK v8context_get_frame(struct _cef_v8context_t* self)
{
  DCHECK(self);
  if(!self)
    return NULL;

  CefRefPtr<CefV8Context> contextPtr = CefV8ContextCppToC::Get(self);
  CefRefPtr<CefFrame> framePtr = contextPtr->GetFrame();
  if(framePtr.get())
    return CefFrameCppToC::Wrap(framePtr);
  return NULL;
}

struct _cef_v8value_t* CEF_CALLBACK v8context_get_global(
    struct _cef_v8context_t* self)
{
  DCHECK(self);
  if(!self)
    return NULL;

  CefRefPtr<CefV8Context> contextPtr = CefV8ContextCppToC::Get(self);
  CefRefPtr<CefV8Value> globalPtr = contextPtr->GetGlobal();
  if(globalPtr.get())
    return CefV8ValueCppToC::Wrap(globalPtr);
  return NULL;
}

int CEF_CALLBACK v8context_enter(struct _cef_v8context_t* self)
{
  DCHECK(self);
  if(!self)
    return false;

  CefRefPtr<CefV8Context> contextPtr = CefV8ContextCppToC::Get(self);
  return contextPtr->Enter();
}

int CEF_CALLBACK v8context_exit(struct _cef_v8context_t* self)
{
  DCHECK(self);
  if(!self)
    return false;

  CefRefPtr<CefV8Context> contextPtr = CefV8ContextCppToC::Get(self);
  return contextPtr->Exit();
}


// CONSTRUCTOR - Do not edit by hand.

CefV8ContextCppToC::CefV8ContextCppToC(CefV8Context* cls)
    : CefCppToC<CefV8ContextCppToC, CefV8Context, cef_v8context_t>(cls)
{
  struct_.struct_.get_browser = v8context_get_browser;
  struct_.struct_.get_frame = v8context_get_frame;
  struct_.struct_.get_global = v8context_get_global;
  struct_.struct_.enter = v8context_enter;
  struct_.struct_.exit = v8context_exit;
}

#ifndef NDEBUG
template<> long CefCppToC<CefV8ContextCppToC, CefV8Context,
    cef_v8context_t>::DebugObjCt = 0;
#endif

