// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef _CEF_BROWSER_IMPL_H
#define _CEF_BROWSER_IMPL_H

#include "../include/cef2.h"
#include "cef_listener_manager.h"
#include "cef_tab_impl.h"

#include "base/basictypes.h"
#include "gfx/rect.h"
#include <list>
#include <map>
#include <queue>

class CefProfileImpl;

class CefBrowserImpl : public CefThreadSafeBase<CefBrowser>
{
public:
  // This constructor is used for wrapping existing browser connections.
  CefBrowserImpl(CefRefPtr<CefProfileImpl> profile,
      const std::wstring& id_string);

  // This constructor is used for creating new browser connections.
  CefBrowserImpl(CefRefPtr<CefProfileImpl> profile,
      const std::wstring& id_string, CefWindowHandle parent, gfx::Rect rect,
      const std::wstring& command_line);
  ~CefBrowserImpl();

  // Called immediately before the browser is removed from the profile. The
  // browser object may not be deleted until much later.
  void Cleanup();

  // Called from CefBrowserImpl::Cleanup().
  void RemoveAllTabs();
  
  // Called from IPC message handlers. If a message is received for a specified
  // tab then that tab is assumed to already be initialized even if an object
  // for it doesn't yet exist.
  CefRefPtr<CefTabImpl> GetOrCreateTab(int index);

  // Called on the IO thread as a result of IPC message events.
  void OnCreated(CefWindowHandle handle);
  void OnTabAdded(int index, bool foreground);
  void OnTabRemoved(int index);
  void OnTabSelected(int index, bool user_gesture);
  void OnTabChanged(int index, const std::wstring& url,
      const std::wstring& title);
  void OnTabNavStatus(int index, bool can_go_back, bool can_go_forward);
  void OnTabLoadStart(int index);
  void OnTabLoadStop(int index);

  // CefBrowser methods.
  virtual CefRefPtr<CefProfile> GetProfile();
  virtual void Destroy();
  virtual std::wstring GetIdString() { return id_string(); }
  virtual CefWindowHandle GetWindowHandle() { return handle(); }
  virtual CefRefPtr<CefTab> CreateTab(int index, const std::wstring& url,
      const std::wstring& referrer, bool foreground);
  virtual int GetTabCount();
  virtual CefRefPtr<CefTab> GetTab(int index);
  virtual CefRefPtr<CefTab> GetSelectedTab();

  CefRefPtr<CefProfileImpl> profile() { return profile_; }
  const std::wstring& id_string() { return id_string_; }
  CefWindowHandle parent() { return parent_; }
  const gfx::Rect& rect() { return rect_; }
  const std::wstring& command_line() { return command_line_; }
  CefWindowHandle handle() { return handle_; }
  CefRefPtr<CefTabImpl> selected_tab() { return selected_tab_; }

protected:
  // Client notification handlers called on the UI thread.
  void OnNotifyCreated();
  void OnNotifyDestroyed();

  // Message sending handlers called on the IO thread.
  void OnSendCreateTab(int index, const std::wstring& url,
      const std::wstring& referrer, bool foreground);

private:
  CefRefPtr<CefProfileImpl> profile_;
  std::wstring id_string_;
  CefWindowHandle parent_;
  gfx::Rect rect_;
  std::wstring command_line_;
  CefWindowHandle handle_;  // Handle for the chrome window returned via IPC.
  bool initialized_;
  bool shutting_down_;

  // Currently selected tab.
  CefRefPtr<CefTabImpl> selected_tab_;

  // Map of tabs that currently exist.
  typedef std::map<int, CefRefPtr<CefTabImpl> > TabMap;
  TabMap tabs_;

  // List of tabs whose creation is still pending.
  typedef std::list<CefRefPtr<CefTabImpl> > TabList;
  TabList pending_tabs_;

  DISALLOW_COPY_AND_ASSIGN(CefBrowserImpl);
};

#endif // _BROWSER_IMPL_H
