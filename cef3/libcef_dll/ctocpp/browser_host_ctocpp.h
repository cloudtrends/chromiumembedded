// Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
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

#ifndef CEF_LIBCEF_DLL_CTOCPP_BROWSER_HOST_CTOCPP_H_
#define CEF_LIBCEF_DLL_CTOCPP_BROWSER_HOST_CTOCPP_H_
#pragma once

#ifndef USING_CEF_SHARED
#pragma message("Warning: "__FILE__" may be accessed wrapper-side only")
#else  // USING_CEF_SHARED

#include <vector>
#include "include/cef_browser.h"
#include "include/capi/cef_browser_capi.h"
#include "include/cef_client.h"
#include "include/capi/cef_client_capi.h"
#include "libcef_dll/ctocpp/ctocpp.h"

// Wrap a C structure with a C++ class.
// This class may be instantiated and accessed wrapper-side only.
class CefBrowserHostCToCpp
    : public CefCToCpp<CefBrowserHostCToCpp, CefBrowserHost,
        cef_browser_host_t> {
 public:
  explicit CefBrowserHostCToCpp(cef_browser_host_t* str)
      : CefCToCpp<CefBrowserHostCToCpp, CefBrowserHost, cef_browser_host_t>(
          str) {}
  virtual ~CefBrowserHostCToCpp() {}

  // CefBrowserHost methods
  virtual CefRefPtr<CefBrowser> GetBrowser() OVERRIDE;
  virtual void CloseBrowser(bool force_close) OVERRIDE;
  virtual void SetFocus(bool focus) OVERRIDE;
  virtual void SetWindowVisibility(bool visible) OVERRIDE;
  virtual CefWindowHandle GetWindowHandle() OVERRIDE;
  virtual CefWindowHandle GetOpenerWindowHandle() OVERRIDE;
  virtual CefRefPtr<CefClient> GetClient() OVERRIDE;
  virtual CefRefPtr<CefRequestContext> GetRequestContext() OVERRIDE;
  virtual double GetZoomLevel() OVERRIDE;
  virtual void SetZoomLevel(double zoomLevel) OVERRIDE;
  virtual void RunFileDialog(FileDialogMode mode, const CefString& title,
      const CefString& default_file_name,
      const std::vector<CefString>& accept_types,
      CefRefPtr<CefRunFileDialogCallback> callback) OVERRIDE;
  virtual void StartDownload(const CefString& url) OVERRIDE;
  virtual void Print() OVERRIDE;
  virtual void Find(int identifier, const CefString& searchText, bool forward,
      bool matchCase, bool findNext) OVERRIDE;
  virtual void StopFinding(bool clearSelection) OVERRIDE;
  virtual void ShowDevTools(const CefWindowInfo& windowInfo,
      CefRefPtr<CefClient> client, const CefBrowserSettings& settings,
      const CefPoint& inspect_element_at) OVERRIDE;
  virtual void CloseDevTools() OVERRIDE;
  virtual void SetMouseCursorChangeDisabled(bool disabled) OVERRIDE;
  virtual bool IsMouseCursorChangeDisabled() OVERRIDE;
  virtual void ReplaceMisspelling(const CefString& word) OVERRIDE;
  virtual void AddWordToDictionary(const CefString& word) OVERRIDE;
  virtual bool IsWindowRenderingDisabled() OVERRIDE;
  virtual void WasResized() OVERRIDE;
  virtual void WasHidden(bool hidden) OVERRIDE;
  virtual void NotifyScreenInfoChanged() OVERRIDE;
  virtual void Invalidate(PaintElementType type) OVERRIDE;
  virtual void SendKeyEvent(const CefKeyEvent& event) OVERRIDE;
  virtual void SendMouseClickEvent(const CefMouseEvent& event,
      MouseButtonType type, bool mouseUp, int clickCount) OVERRIDE;
  virtual void SendMouseMoveEvent(const CefMouseEvent& event,
      bool mouseLeave) OVERRIDE;
  virtual void SendMouseWheelEvent(const CefMouseEvent& event, int deltaX,
      int deltaY) OVERRIDE;
  virtual void SendFocusEvent(bool setFocus) OVERRIDE;
  virtual void SendCaptureLostEvent() OVERRIDE;
  virtual void NotifyMoveOrResizeStarted() OVERRIDE;
  virtual CefTextInputContext GetNSTextInputContext() OVERRIDE;
  virtual void HandleKeyEventBeforeTextInputClient(
      CefEventHandle keyEvent) OVERRIDE;
  virtual void HandleKeyEventAfterTextInputClient(
      CefEventHandle keyEvent) OVERRIDE;
  virtual void DragTargetDragEnter(CefRefPtr<CefDragData> drag_data,
      const CefMouseEvent& event, DragOperationsMask allowed_ops) OVERRIDE;
  virtual void DragTargetDragOver(const CefMouseEvent& event,
      DragOperationsMask allowed_ops) OVERRIDE;
  virtual void DragTargetDragLeave() OVERRIDE;
  virtual void DragTargetDrop(const CefMouseEvent& event) OVERRIDE;
  virtual void DragSourceEndedAt(int x, int y, DragOperationsMask op) OVERRIDE;
  virtual void DragSourceSystemDragEnded() OVERRIDE;
};

#endif  // USING_CEF_SHARED
#endif  // CEF_LIBCEF_DLL_CTOCPP_BROWSER_HOST_CTOCPP_H_

