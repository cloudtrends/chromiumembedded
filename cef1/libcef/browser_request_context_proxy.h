// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef CEF_LIBCEF_BROWSER_REQUEST_CONTEXT_PROXY_H_
#define CEF_LIBCEF_BROWSER_REQUEST_CONTEXT_PROXY_H_
#pragma once

#include <string>
#include "include/cef_base.h"
#include "net/url_request/url_request_context.h"

class BrowserRequestContext;
class CefBrowserImpl;

// A URLRequestContext implementation that proxies cookie requests to the
// client.
class BrowserRequestContextProxy : public net::URLRequestContext {
 public:
  BrowserRequestContextProxy(BrowserRequestContext* context,
                             CefBrowserImpl* browser);

 private:
  BrowserRequestContext* context_;
  CefRefPtr<CefBrowserImpl> browser_;
};

#endif  // CEF_LIBCEF_BROWSER_REQUEST_CONTEXT_PROXY_H_
