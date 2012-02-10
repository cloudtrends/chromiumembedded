// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef CEF_LIBCEF_BROWSER_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
#define CEF_LIBCEF_BROWSER_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
#pragma once

#include "content/public/browser/resource_dispatcher_host_delegate.h"

class ResourceDispatcherHost;

// Implements ResourceDispatcherHostDelegate.
class CefResourceDispatcherHostDelegate
    : public content::ResourceDispatcherHostDelegate {
 public:
  // This class does not take ownership of the tracker but merely holds a
  // reference to it to avoid accessing g_browser_process.
  // |resource_dispatcher_host| must outlive |this|.
  CefResourceDispatcherHostDelegate(
      ResourceDispatcherHost* resource_dispatcher_host);
  virtual ~CefResourceDispatcherHostDelegate();

  // ResourceDispatcherHostDelegate methods.
  
 private:
  ResourceDispatcherHost* resource_dispatcher_host_;

  DISALLOW_COPY_AND_ASSIGN(CefResourceDispatcherHostDelegate);
};

#endif  // CEF_LIBCEF_BROWSER_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
