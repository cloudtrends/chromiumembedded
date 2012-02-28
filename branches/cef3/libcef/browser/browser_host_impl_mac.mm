// Copyright (c) 2012 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/browser/browser_host_impl.h"

bool CefBrowserHostImpl::PlatformViewText(const std::string& text) {
  NOTIMPLEMENTED();
  return false;
}

bool CefBrowserHostImpl::PlatformCreateWindow() {
  NOTIMPLEMENTED();
  return false;
}

void CefBrowserHostImpl::PlatformCloseWindow() {
  NOTIMPLEMENTED();
}

void CefBrowserHostImpl::PlatformSizeTo(int width, int height) {
  NOTIMPLEMENTED();
}

CefWindowHandle CefBrowserHostImpl::PlatformGetWindowHandle() {
  NOTIMPLEMENTED();
  return NULL;
}
