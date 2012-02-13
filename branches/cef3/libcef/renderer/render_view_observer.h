// Copyright (c) 2012 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CEF_LIBCEF_RENDERER_RENDER_VIEW_OBSERVER_H_
#define CEF_LIBCEF_RENDERER_RENDER_VIEW_OBSERVER_H_
#pragma once

#include <map>
#include <string>

#include "content/public/renderer/render_view_observer.h"
#include "content/public/renderer/render_view_observer_tracker.h"

class GURL;
struct CefMsg_ExecuteCode_Params;
struct CefMsg_LoadRequest_Params;
class CefContentRendererClient;

namespace base {
class ListValue;
}

namespace WebKit {
class WebFrame;
}

// Renderer plumbing for CEF features. There is a one-to-one relationship
// between RenderView on the renderer side and RenderViewHost on the browser
// side.
//
// RenderViewObserver: Interface for observing RenderView notifications and IPC
// messages. IPC messages received by the RenderView will be forwarded to this
// RenderViewObserver implementation. IPC messages sent using
// RenderViewObserver::Send() will be forwarded to the RenderView. Use
// RenderViewObserver::routing_id() when sending IPC messages.
class CefRenderViewObserver
    : public content::RenderViewObserver,
      public content::RenderViewObserverTracker<CefRenderViewObserver> {
 public:
  explicit CefRenderViewObserver(content::RenderView* render_view);
  virtual ~CefRenderViewObserver();

  int browser_window_id() const { return browser_window_id_; }

 private:
  // RenderViewObserver implementation.
  virtual void DidStartProvisionalLoad(WebKit::WebFrame* frame) OVERRIDE;
  virtual void FrameDetached(WebKit::WebFrame* frame) OVERRIDE;
  virtual void FocusedNodeChanged(const WebKit::WebNode& node) OVERRIDE;
  virtual void DidCreateDataSource(WebKit::WebFrame* frame,
                                   WebKit::WebDataSource* ds) OVERRIDE;
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  // Message handlers.
  void OnUpdateBrowserWindowId(int window_id);
  void OnResponse(int request_id, bool success,
                  const std::string& response,
                  const std::string& error);
  void OnLoadString(int64 frame_id, const std::string& string, const GURL& url);
  void OnLoadRequest(const CefMsg_LoadRequest_Params& params);
  void OnExecuteCommand(int request_id, int64 frame_id,
                        const std::string& command);
  void OnExecuteCode(const CefMsg_ExecuteCode_Params& params);

  void AddWebFrame(WebKit::WebFrame* frame);
  WebKit::WebFrame* GetWebFrame(int64 frame_id);

  // Id number of browser window which RenderView is attached to.
  int browser_window_id_;

  // Id of the last frame that had focus.
  int64 last_focused_frame_id_;

  // Map of unique frame ids to WebFrame pointers.
  typedef std::map<int64, WebKit::WebFrame*> FrameMap;
  FrameMap frames_;

  DISALLOW_COPY_AND_ASSIGN(CefRenderViewObserver);
};

#endif  // CEF_LIBCEF_RENDERER_RENDER_VIEW_OBSERVER_H_
