// Copyright (c) 2012 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/browser/browser_impl.h"

bool CefBrowserImpl::PlatformViewText(const std::string& text) {
  NOTIMPLEMENTED();
  return false;
}

bool CefBrowserImpl::PlatformCreateWindow() {
  NOTIMPLEMENTED();
  return false;
}

void CefBrowserImpl::PlatformCloseWindow() {
  NOTIMPLEMENTED();
}

void CefBrowserImpl::PlatformSizeTo(int width, int height) {
  NOTIMPLEMENTED();
}

CefWindowHandle CefBrowserImpl::PlatformGetWindowHandle() {
  NOTIMPLEMENTED();
  return NULL;
}
