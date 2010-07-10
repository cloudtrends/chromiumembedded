// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef _CEF_CHANNEL_H
#define _CEF_CHANNEL_H

// Must be included early (e.g. before chrome/common/plugin_messages.h)
#include "ipc/ipc_logging.h"

#include "base/basictypes.h"
#include "base/linked_ptr.h"
#include "base/ref_counted.h"
#include "base/scoped_ptr.h"
#include "gfx/native_widget_types.h"
#include "gfx/rect.h"
#include "chrome/browser/chrome_thread.h"
#include "ipc/ipc_channel.h"

class Browser;
class CefConnector;
class CefService;

// Class used for communication with the host process.
class CefChannel : public IPC::Channel::Listener,
                   public IPC::Message::Sender,
                   public base::RefCountedThreadSafe<CefChannel,
                       ChromeThread::DeleteOnIOThread>
{
public:
  CefChannel(CefService* service, std::string channel_id);
  ~CefChannel();

  // Create the channel. This method must be called on the IO thread.
  void CreateChannel();

  // Called immediately before the channel is removed from the service. The
  // channel object may not be deleted until much later.
  void Cleanup();

  // Called to destroy the channel.
  void Destroy();

  // Create a new connector linking a Browser to this channel.
  CefConnector* CreateConnector(gfx::NativeWindow parent,
      const gfx::Rect& bounds, const std::string& browser_id,
      Browser* browser);

  // Return an existing connector.
  CefConnector* GetConnector(const std::string& browser_id);

  // Will be called by CefConnector::Destroy().
  void RemoveConnector(const std::string& browser_id);
  // Will be called by CefChannel::Cleanup().
  void RemoveAllConnectors();

  // IPC::Channel::Listener methods will be called on the IO thread.
  virtual void OnMessageReceived(const IPC::Message& message);
  virtual void OnChannelConnected(int32 peer_pid);
  virtual void OnChannelError();

  // IPC::Channel::Sender methods must be called on the IO thread.
  virtual bool Send(IPC::Message* msg);

private:
#if defined(IPC_MESSAGE_LOG_ENABLED)
  void OnSetIPCLoggingEnabled(bool enable);
#endif
  void OnCreateTab(const std::string& browser_id, int index,
      const std::string& url, const std::string& referrer, bool foreground);
  void OnTabCommand(const std::string& browser_id, int index, int command);
  void OnTabLoadAddress(const std::string& browser_id, int index,
      const std::string& url, const std::string& referrer);
  void OnTabExecuteJavaScript(const std::string& browser_id, int index,
      const std::wstring& frame_xpath, const std::wstring& jscript);
  
  // Class level lock.  Used to protect internal data structures that may be
  // accessed from other threads.
  Lock lock_;

  CefService* service_;
  std::string channel_id_;
  scoped_ptr<IPC::Channel> channel_;
  bool channel_connected_;

  typedef std::queue<IPC::Message*> MessageQueue;
  MessageQueue pending_messages_;

  // Map of connectors that currently exist.
  typedef std::map<std::string, scoped_refptr<CefConnector> > ConnectorMap;
  ConnectorMap connectors_;

  DISALLOW_COPY_AND_ASSIGN(CefChannel);
};

#endif // _CEF_CHANNEL_H
