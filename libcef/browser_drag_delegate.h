// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _BROWSER_DRAG_DELEGATE_H
#define _BROWSER_DRAG_DELEGATE_H

#include "ui/base/dragdrop/drag_source.h"

namespace WebKit {
class WebView;
}

// A class that implements ui::DragSource for the browser webview
// delegate.
class BrowserDragDelegate : public ui::DragSource {
 public:
  BrowserDragDelegate(HWND source_hwnd, WebKit::WebView* webview)
      : ui::DragSource(),
        source_hwnd_(source_hwnd),
        webview_(webview) { }

 protected:
  // ui::DragSource
  virtual void OnDragSourceCancel();
  virtual void OnDragSourceDrop();
  virtual void OnDragSourceMove();

 private:
  WebKit::WebView* webview_;

  // A HWND for the source we are associated with, used for translating
  // mouse coordinates from screen to client coordinates.
  HWND source_hwnd_;
};

#endif  // _BROWSER_DRAG_DELEGATE_H

