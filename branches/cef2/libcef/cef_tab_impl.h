// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef _CEF_TAB_IMPL_H
#define _CEF_TAB_IMPL_H

#include "../include/cef2.h"
#include "cef_listener_manager.h"
#include "common/cef_messages.h"

#include "base/basictypes.h"
#include "gfx/rect.h"

class CefBrowserImpl;

class CefTabImpl : public CefThreadSafeBase<CefTab>
{
public:
  CefTabImpl(CefRefPtr<CefBrowserImpl> browser, int index, bool initialized);
  ~CefTabImpl();

  // Called immediately before the tab is removed from the browser. The
  // tab object may not be deleted until much later.
  void Cleanup();

  // Called on the IO thread as a result of IPC message events.
  void OnCreated();
  void OnChanged(const std::wstring& url, const std::wstring& title);
  void OnNavStatus(bool can_go_back, bool can_go_forward);
  void OnSelected(bool selected);
  void OnLoadStart();
  void OnLoadStop();

  // CefTab methods.
  virtual CefRefPtr<CefBrowser> GetBrowser();
  virtual void Destroy() { OnCommand(CEF_TC_TAB_DESTROY); }
  virtual int GetIndex() { return index(); }
  virtual bool IsSelected() { return selected(); }
  virtual void Select() { OnCommand(CEF_TC_TAB_SELECT); }
  virtual bool CanGoBack() { return can_go_back(); }
  virtual void GoBack() { OnCommand(CEF_TC_GOBACK); }
  virtual bool CanGoForward() { return can_go_forward(); }
  virtual void GoForward() { OnCommand(CEF_TC_GOFORWARD); }
  virtual bool IsLoading() { return loading(); }
  virtual void Reload() { OnCommand(CEF_TC_RELOAD); }
  virtual void StopLoad() { OnCommand(CEF_TC_STOPLOAD); }
  virtual std::wstring GetAddress();
  virtual std::wstring GetTitle();
  virtual void Undo() { OnCommand(CEF_TC_UNDO); }
  virtual void Redo() { OnCommand(CEF_TC_REDO); }
  virtual void Cut() { OnCommand(CEF_TC_CUT); }
  virtual void Copy() { OnCommand(CEF_TC_COPY); }
  virtual void Paste() { OnCommand(CEF_TC_PASTE); }
  virtual void Delete() { OnCommand(CEF_TC_DELETE); }
  virtual void SelectAll() { OnCommand(CEF_TC_SELECTALL); }
  virtual void Print() { OnCommand(CEF_TC_PRINT); }
  virtual void ViewSource() { OnCommand(CEF_TC_VIEWSOURCE); }
  virtual void LoadAddress(const std::wstring& url,
                           const std::wstring& referrer);
  virtual void ExecuteJavaScript(const std::wstring& frame_xpath, 
                                 const std::wstring& jscript);

  CefRefPtr<CefBrowserImpl> browser() { return browser_; }
  int index() { return index_; }
  void set_index(int index) { index_ = index; }
  bool initialized() { return initialized_; }
  void set_initialized(bool initialized) { initialized_ = initialized; }
  const std::wstring& url() { return url_; }
  const std::wstring& title() { return title_; }
  bool selected() { return selected_; }
  bool can_go_back() { return can_go_back_; }
  bool can_go_forward() { return can_go_forward_; }
  bool loading() { return loading_; }

protected:
  // Client notification handlers called on the UI thread.
  void OnNotifyCreate();
  void OnNotifyDestroy();
  void OnNotifyAddressChange(const std::wstring& url);
  void OnNotifyTitleChange(const std::wstring& title);
  void OnNotifyNavStateChange(bool can_go_back, bool can_go_forward); 
  void OnNotifySelectChange(bool selected);
  void OnNotifyLoadStart();
  void OnNotifyLoadStop();

  // Post commands to the IO thread for sending.
  void OnCommand(CefTabCommand command);

  // Message sending handlers called on the IO thread.
  void OnSendCommand(CefTabCommand command);
  void OnSendLoadAddress(const std::wstring& url, const std::wstring& referrer);
  void OnSendExecuteJavaScript(const std::wstring& frame_xpath, 
                               const std::wstring& jscript);

private:
  CefRefPtr<CefBrowserImpl> browser_;
  bool initialized_;
  long index_;
  std::wstring url_;
  std::wstring title_;
  bool selected_;
  bool can_go_back_;
  bool can_go_forward_;
  bool loading_;

  DISALLOW_COPY_AND_ASSIGN(CefTabImpl);
};

#endif // _CEF_TAB_IMPL_H
