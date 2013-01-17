// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// ---------------------------------------------------------------------------
//
// This file was generated by the CEF translator tool. If making changes by
// hand only do so within the body of existing method and function
// implementations. See the translator.README.txt file in the tools directory
// for more information.
//

#include "libcef_dll/cpptoc/domevent_listener_cpptoc.h"
#include "libcef_dll/ctocpp/domevent_ctocpp.h"


// MEMBER FUNCTIONS - Body may be edited by hand.

void CEF_CALLBACK domevent_listener_handle_event(
    struct _cef_domevent_listener_t* self, cef_domevent_t* event) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return;
  // Verify param: event; type: refptr_diff
  DCHECK(event);
  if (!event)
    return;

  // Execute
  CefDOMEventListenerCppToC::Get(self)->HandleEvent(
      CefDOMEventCToCpp::Wrap(event));
}


// CONSTRUCTOR - Do not edit by hand.

CefDOMEventListenerCppToC::CefDOMEventListenerCppToC(CefDOMEventListener* cls)
    : CefCppToC<CefDOMEventListenerCppToC, CefDOMEventListener,
        cef_domevent_listener_t>(cls) {
  struct_.struct_.handle_event = domevent_listener_handle_event;
}

#ifndef NDEBUG
template<> long CefCppToC<CefDOMEventListenerCppToC, CefDOMEventListener,
    cef_domevent_listener_t>::DebugObjCt = 0;
#endif

