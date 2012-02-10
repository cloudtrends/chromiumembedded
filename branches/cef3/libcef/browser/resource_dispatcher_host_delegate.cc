// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "libcef/browser/resource_dispatcher_host_delegate.h"

CefResourceDispatcherHostDelegate::CefResourceDispatcherHostDelegate(
    ResourceDispatcherHost* resource_dispatcher_host)
    : resource_dispatcher_host_(resource_dispatcher_host) {
}

CefResourceDispatcherHostDelegate::~CefResourceDispatcherHostDelegate() {
}
