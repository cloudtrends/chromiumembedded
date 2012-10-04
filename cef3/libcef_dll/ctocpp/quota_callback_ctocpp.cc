// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
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

#include "libcef_dll/ctocpp/quota_callback_ctocpp.h"


// VIRTUAL METHODS - Body may be edited by hand.

void CefQuotaCallbackCToCpp::Continue(bool allow) {
  if (CEF_MEMBER_MISSING(struct_, cont))
    return;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  struct_->cont(struct_,
      allow);
}

void CefQuotaCallbackCToCpp::Cancel() {
  if (CEF_MEMBER_MISSING(struct_, cancel))
    return;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  struct_->cancel(struct_);
}


#ifndef NDEBUG
template<> long CefCToCpp<CefQuotaCallbackCToCpp, CefQuotaCallback,
    cef_quota_callback_t>::DebugObjCt = 0;
#endif

