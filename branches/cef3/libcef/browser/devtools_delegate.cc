// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/browser/devtools_delegate.h"

#include <algorithm>
#include <string>

#include "content/public/browser/devtools_http_handler.h"
#include "content/public/browser/web_contents.h"
#include "grit/cef_resources.h"
#include "net/url_request/url_request_context_getter.h"
#include "ui/base/resource/resource_bundle.h"


CefDevToolsDelegate::CefDevToolsDelegate(
    int port,
    net::URLRequestContextGetter* context_getter)
    : content::WebContentsObserver(),
      context_getter_(context_getter) {
  devtools_http_handler_ = content::DevToolsHttpHandler::Start(
      "127.0.0.1",
      port,
      "",
      this);
}

CefDevToolsDelegate::~CefDevToolsDelegate() {
}

void CefDevToolsDelegate::Stop() {
  // The call below destroys this.
  devtools_http_handler_->Stop();
}

void CefDevToolsDelegate::WebContentsDestroyed(content::WebContents* contents) {
  std::remove(web_contents_list_.begin(), web_contents_list_.end(), contents);
}

content::DevToolsHttpHandlerDelegate::InspectableTabs
CefDevToolsDelegate::GetInspectableTabs() {
  DevToolsHttpHandlerDelegate::InspectableTabs tabs;
  std::vector<content::WebContents*>::iterator it = web_contents_list_.begin();
  for (; it != web_contents_list_.end(); ++it) {
    tabs.push_back(*it);
  }
  return tabs;
}

std::string CefDevToolsDelegate::GetDiscoveryPageHTML() {
  return ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_CEF_DEVTOOLS_DISCOVERY_PAGE).as_string();
}

net::URLRequestContext*
CefDevToolsDelegate::GetURLRequestContext() {
  return context_getter_->GetURLRequestContext();
}

bool CefDevToolsDelegate::BundlesFrontendResources() {
  return true;
}

std::string CefDevToolsDelegate::GetFrontendResourcesBaseURL() {
  return "";
}

void CefDevToolsDelegate::AddWebContents(content::WebContents* web_contents) {
  web_contents_list_.push_back(web_contents);
  Observe(web_contents);
}
