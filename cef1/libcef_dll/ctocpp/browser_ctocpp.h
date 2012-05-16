// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
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

#ifndef CEF_LIBCEF_DLL_CTOCPP_BROWSER_CTOCPP_H_
#define CEF_LIBCEF_DLL_CTOCPP_BROWSER_CTOCPP_H_
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
class CefBrowserCToCpp
    : public CefCToCpp<CefBrowserCToCpp, CefBrowser, cef_browser_t> {
 public:
  explicit CefBrowserCToCpp(cef_browser_t* str)
      : CefCToCpp<CefBrowserCToCpp, CefBrowser, cef_browser_t>(str) {}
  virtual ~CefBrowserCToCpp() {}

  // CefBrowser methods
  virtual void ParentWindowWillClose() OVERRIDE;
  virtual void CloseBrowser() OVERRIDE;
  virtual bool CanGoBack() OVERRIDE;
  virtual void GoBack() OVERRIDE;
  virtual bool CanGoForward() OVERRIDE;
  virtual void GoForward() OVERRIDE;
  virtual void Reload() OVERRIDE;
  virtual void ReloadIgnoreCache() OVERRIDE;
  virtual void StopLoad() OVERRIDE;
  virtual void SetFocus(bool enable) OVERRIDE;
  virtual CefWindowHandle GetWindowHandle() OVERRIDE;
  virtual CefWindowHandle GetOpenerWindowHandle() OVERRIDE;
  virtual bool IsPopup() OVERRIDE;
  virtual bool HasDocument() OVERRIDE;
  virtual CefRefPtr<CefClient> GetClient() OVERRIDE;
  virtual CefRefPtr<CefFrame> GetMainFrame() OVERRIDE;
  virtual CefRefPtr<CefFrame> GetFocusedFrame() OVERRIDE;
  virtual CefRefPtr<CefFrame> GetFrame(const CefString& name) OVERRIDE;
  virtual void GetFrameNames(std::vector<CefString>& names) OVERRIDE;
  virtual void Find(int identifier, const CefString& searchText, bool forward,
      bool matchCase, bool findNext) OVERRIDE;
  virtual void StopFinding(bool clearSelection) OVERRIDE;
  virtual double GetZoomLevel() OVERRIDE;
  virtual void SetZoomLevel(double zoomLevel) OVERRIDE;
  virtual void ClearHistory() OVERRIDE;
  virtual void ShowDevTools() OVERRIDE;
  virtual void CloseDevTools() OVERRIDE;
  virtual bool IsWindowRenderingDisabled() OVERRIDE;
  virtual bool GetSize(PaintElementType type, int& width, int& height) OVERRIDE;
  virtual void SetSize(PaintElementType type, int width, int height) OVERRIDE;
  virtual bool IsPopupVisible() OVERRIDE;
  virtual void HidePopup() OVERRIDE;
  virtual void Invalidate(const CefRect& dirtyRect) OVERRIDE;
  virtual bool GetImage(PaintElementType type, int width, int height,
      void* buffer) OVERRIDE;
  virtual void SendKeyEvent(KeyType type, const CefKeyInfo& keyInfo,
      int modifiers) OVERRIDE;
  virtual void SendMouseClickEvent(int x, int y, MouseButtonType type,
      bool mouseUp, int clickCount) OVERRIDE;
  virtual void SendMouseMoveEvent(int x, int y, bool mouseLeave) OVERRIDE;
  virtual void SendMouseWheelEvent(int x, int y, int deltaX,
      int deltaY) OVERRIDE;
  virtual void SendFocusEvent(bool setFocus) OVERRIDE;
  virtual void SendCaptureLostEvent() OVERRIDE;
};

#endif  // USING_CEF_SHARED
#endif  // CEF_LIBCEF_DLL_CTOCPP_BROWSER_CTOCPP_H_

