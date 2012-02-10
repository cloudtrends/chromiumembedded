// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/renderer/content_renderer_client.h"
#include "libcef/common/cef_messages.h"
#include "libcef/renderer/render_message_filter.h"
#include "libcef/renderer/render_view_observer.h"

#include "content/common/child_thread.h"
#include "content/public/renderer/render_view.h"
#include "ipc/ipc_sync_channel.h"
#include "v8/include/v8.h"

CefContentRendererClient::CefContentRendererClient() {
}

CefContentRendererClient::~CefContentRendererClient() {
}

void CefContentRendererClient::RenderThreadStarted() {
  ChildThread::current()->channel()->AddFilter(new CefRenderMessageFilter);
  ChildThread::current()->Send(new CefProcessHostMsg_RenderThreadStarted);
}

void CefContentRendererClient::RenderViewCreated(
    content::RenderView* render_view) {
  // Initialize the CefRenderViewObserver object for the new RenderView. It will
  // automatically register itself as an observer.
  new CefRenderViewObserver(render_view);
}

void CefContentRendererClient::SetNumberOfViews(int number_of_views) {
}

SkBitmap* CefContentRendererClient::GetSadPluginBitmap() {
  return NULL;
}

std::string CefContentRendererClient::GetDefaultEncoding() {
  return std::string();
}

bool CefContentRendererClient::OverrideCreatePlugin(
    content::RenderView* render_view,
    WebKit::WebFrame* frame,
    const WebKit::WebPluginParams& params,
    WebKit::WebPlugin** plugin) {
  return false;
}

bool CefContentRendererClient::HasErrorPage(int http_status_code,
                                            std::string* error_domain) {
  return false;
}

void CefContentRendererClient::GetNavigationErrorStrings(
    const WebKit::WebURLRequest& failed_request,
    const WebKit::WebURLError& error,
    std::string* error_html,
    string16* error_description) {
}

webkit_media::WebMediaPlayerImpl*
CefContentRendererClient::OverrideCreateWebMediaPlayer(
    content::RenderView* render_view,
    WebKit::WebFrame* frame,
    WebKit::WebMediaPlayerClient* client,
    base::WeakPtr<webkit_media::WebMediaPlayerDelegate> delegate,
    media::FilterCollection* collection,
    WebKit::WebAudioSourceProvider* audio_source_provider,
    media::MessageLoopFactory* message_loop_factory,
    webkit_media::MediaStreamClient* media_stream_client,
    media::MediaLog* media_log) {
  return NULL;
}

bool CefContentRendererClient::RunIdleHandlerWhenWidgetsHidden() {
  return true;
}

bool CefContentRendererClient::AllowPopup(const GURL& creator) {
  return false;
}

bool CefContentRendererClient::ShouldFork(WebKit::WebFrame* frame,
                                          const GURL& url,
                                          bool is_content_initiated,
                                          bool is_initial_navigation,
                                          bool* send_referrer) {
  return false;
}

bool CefContentRendererClient::WillSendRequest(WebKit::WebFrame* frame,
                                               const GURL& url,
                                               GURL* new_url) {
  return false;
}

bool CefContentRendererClient::ShouldPumpEventsDuringCookieMessage() {
  return false;
}

void CefContentRendererClient::DidCreateScriptContext(
    WebKit::WebFrame* frame, v8::Handle<v8::Context> context, int world_id) {
}

void CefContentRendererClient::WillReleaseScriptContext(
    WebKit::WebFrame* frame, v8::Handle<v8::Context> context, int world_id) {
}

unsigned long long CefContentRendererClient::VisitedLinkHash(
    const char* canonical_url, size_t length) {
  return 0LL;
}

bool CefContentRendererClient::IsLinkVisited(unsigned long long link_hash) {
  return false;
}

void CefContentRendererClient::PrefetchHostName(
    const char* hostname, size_t length) {
}

bool CefContentRendererClient::ShouldOverridePageVisibilityState(
    const content::RenderView* render_view,
    WebKit::WebPageVisibilityState* override_state) const {
  return false;
}

bool CefContentRendererClient::HandleGetCookieRequest(
    content::RenderView* sender,
    const GURL& url,
    const GURL& first_party_for_cookies,
    std::string* cookies) {
  return false;
}

bool CefContentRendererClient::HandleSetCookieRequest(
    content::RenderView* sender,
    const GURL& url,
    const GURL& first_party_for_cookies,
    const std::string& value) {
  return false;
}

void CefContentRendererClient::RegisterPPAPIInterfaceFactories(
    webkit::ppapi::PpapiInterfaceFactoryManager* factory_manager) {
}

bool CefContentRendererClient::AllowSocketAPI(const GURL& url) {
  return false;
}
