// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef _CEF_PROFILE_IMPL_H
#define _CEF_PROFILE_IMPL_H

#include "../include/cef2.h"
#include "cef_browser_impl.h"
#include "cef_listener_manager.h"
#include "cef_server.h"

#include "base/basictypes.h"
#include "base/process_util.h"
#include "gfx/native_widget_types.h"

class CefProfileImpl : public CefServer::ServerListener,
                       public CefThreadSafeBase<CefProfile>
{
public:
  CefProfileImpl(const std::wstring& user_profile);
  ~CefProfileImpl();

  // Called immediately before the profile is removed from the context. The
  // profile object may not be deleted until much later.
  void Cleanup();

  // Called from CefBrowserImpl::Destroy().
  void RemoveBrowser(const std::wstring& id_string);
  // Called from CefProfileImpl::Cleanup().
  void RemoveAllBrowsers();

  // Called from IPC message handlers. If a message is received for a specified
  // browser then that browser is assumed to already be initialized even if an
  // object for it doesn't yet exist.
  CefRefPtr<CefBrowserImpl> GetOrCreateBrowser(const std::wstring& id_string);

  // CefProfile methods.
  virtual void AddBrowserListener(const std::wstring& id_string,
                                  CefRefPtr<CefBrowserListener> listener)
    { listener_manager_->browser()->Add(id_string, listener); }
  virtual void RemoveBrowserListener(const std::wstring& id_string)
    { listener_manager_->browser()->Remove(id_string); }
  virtual void AddTabListener(const std::wstring& id_string,
                              CefRefPtr<CefTabListener> listener)
    { listener_manager_->tab()->Add(id_string, listener); }
  virtual void RemoveTabListener(const std::wstring& id_string)
    { listener_manager_->tab()->Remove(id_string); }
  virtual void Destroy();
  virtual CefRefPtr<CefBrowser> CreateBrowser(const std::wstring& id_string,
      CefWindowHandle parent, int xpos, int ypos, int width, int height,
      const std::wstring& command_line);

  // CefServer::Listener methods will be called on the IO thread.
  virtual void OnChannelCreated();
  virtual void OnMessageReceived(const IPC::Message& message);
  virtual void OnChannelConnected(int32 peer_pid);
  virtual void OnChannelError();

  // Called on the IO thread to send an IPC message.
  bool Send(IPC::Message* msg);

  CefRefPtr<CefListenerManager> listener_manager()
    { return listener_manager_; }

protected:
  // Communication management methods called on the IO thread.
  void InitializeBrowser();
  void OnProcessLaunched(base::ProcessHandle handle);
  void OnProcessLaunchError();

  // Process creation methods called on the PROCESS_LAUNCHER thread.
  bool IsAllowedSwitch(const std::string& switch_string,
      const std::wstring& value_string);
  void CreateBrowserProcess(CefBrowserImpl* browser,
      const std::wstring& user_profile);

  // Browsers are added to the pending queue while their creation is pending.
  // These methods are only called while the object lock is being held.
  void RemovePendingBrowser(CefRefPtr<CefBrowserImpl> browser);
  void RemoveAllPendingBrowsers();
  void PushPendingBrowser(CefRefPtr<CefBrowserImpl> browser);
  bool PopPendingBrowser(CefRefPtr<CefBrowserImpl>& browser);

  // Called on the IO thread as a result of IPC message events.
  void OnBrowserCreated(const std::string& browser_id,
      gfx::NativeViewId browser_handle);
  void OnTabAdded(const std::string& browser_id, int index, bool foreground);
  void OnTabRemoved(const std::string& browser_id, int index);
  void OnTabSelected(const std::string& browser_id, int index,
      bool user_gesture);
  void OnTabChanged(const std::string& browser_id, int index,
      const std::string& url, const std::wstring& title);
  void OnTabNavStatus(const std::string& browser_id, int index,
      bool can_go_back, bool can_go_forward);
  void OnTabLoadStart(const std::string& browser_id, int index);
  void OnTabLoadStop(const std::string& browser_id, int index);
  
private:
  std::wstring user_profile_;
  bool shutting_down_; // True if the profile is shutting down.

  // Map of browsers that currently exist.
  typedef std::map<std::wstring, CefRefPtr<CefBrowserImpl> > BrowserMap;
  BrowserMap browsers_;

  // IPC server for communicating with the chrome process.
  scoped_refptr<CefServer> server_;
  bool channel_created_;
  bool channel_connected_;
  bool process_starting_; // True while the chrome process is starting.
  base::ProcessHandle process_handle_;

  typedef std::list<CefRefPtr<CefBrowserImpl> > BrowserList;
  BrowserList pending_browsers_;

  typedef std::queue<IPC::Message*> MessageQueue;
  MessageQueue pending_messages_;

  CefRefPtr<CefListenerManager> listener_manager_;

  DISALLOW_COPY_AND_ASSIGN(CefProfileImpl);
};

#endif // _PROFILE_IMPL_H
