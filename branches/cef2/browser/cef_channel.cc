// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef_channel.h"
#include "cef_connector.h"
#include "cef_service.h"
#include "common/cef_messages.h"
#include "gfx/native_widget_types.h"


CefChannel::CefChannel(CefService* service, std::string channel_id)
  : service_(service), channel_id_(channel_id), channel_connected_(false)
{
}

CefChannel::~CefChannel()
{
  RemoveAllConnectors();

  AutoLock lock_scope(lock_);

  // Delete any messages on the queue.
  while(!pending_messages_.empty()) {
    delete pending_messages_.front();
    pending_messages_.pop();
  }
}

void CefChannel::CreateChannel()
{
  bool connected = false;
  {
    AutoLock lock_scope(lock_);

    channel_.reset(
        new IPC::Channel(channel_id_, IPC::Channel::MODE_CLIENT, this));
    connected = channel_->Connect();
  }

  if(!connected)
    OnChannelError();
}

void CefChannel::Cleanup()
{
  RemoveAllConnectors();
}

void CefChannel::Destroy()
{
  service_->RemoveChannel(channel_id_);
}

CefConnector* CefChannel::CreateConnector(gfx::NativeWindow parent,
                                          const gfx::Rect& bounds,
                                          const std::string& browser_id,
                                          Browser* browser)
{
  AutoLock lock_scope(lock_);

  scoped_refptr<CefConnector> connector(
      new CefConnector(this, parent, bounds, browser_id, browser));
  connectors_.insert(std::make_pair<std::string,scoped_refptr<CefConnector>>(
      browser_id, connector));
  return connector.get();
}

CefConnector* CefChannel::GetConnector(const std::string& browser_id)
{
  AutoLock lock_scope(lock_);

  ConnectorMap::const_iterator it = connectors_.find(browser_id);
  if(it != connectors_.end())
    return it->second;
  
  return NULL;
}

void CefChannel::RemoveConnector(const std::string& browser_id)
{
  scoped_refptr<CefConnector> connector;
  
  {
    AutoLock lock_scope(lock_);

    ConnectorMap::iterator it = connectors_.find(browser_id);
    if(it != connectors_.end()) {
      connector = it->second;
      connectors_.erase(it);
    }
  }

  if(connector.get())
    connector->Cleanup();
}

void CefChannel::RemoveAllConnectors()
{
  ConnectorMap connectors;

  {
    AutoLock lock_scope(lock_);
    connectors = connectors_;
    connectors_.clear();
  }

  ConnectorMap::const_iterator it = connectors.begin();
  for(; it != connectors.end(); ++it)
    it->second->Cleanup();
}

void CefChannel::OnMessageReceived(const IPC::Message& msg)
{
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(CefChannel, msg)
#if defined(IPC_MESSAGE_LOG_ENABLED)
    IPC_MESSAGE_HANDLER(CefMsg_SetIPCLoggingEnabled, OnSetIPCLoggingEnabled)
#endif
    IPC_MESSAGE_HANDLER(CefMsg_CreateTab, OnCreateTab)
    IPC_MESSAGE_HANDLER(CefMsg_TabCommand, OnTabCommand)
    IPC_MESSAGE_HANDLER(CefMsg_TabLoadAddress, OnTabLoadAddress)
    IPC_MESSAGE_HANDLER(CefMsg_TabExecuteJavaScript, OnTabExecuteJavaScript)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
}

void CefChannel::OnChannelConnected(int32 peer_pid)
{
  channel_connected_ = true;
  MessageQueue pending_messages;

  {
    AutoLock lock_scope(lock_);
    pending_messages = pending_messages_;
    pending_messages_.empty();
  }
  
  // Send the queued messages, if any.
  while(!pending_messages.empty()) {
    Send(pending_messages.front());
    pending_messages.pop();
  }
}

void CefChannel::OnChannelError()
{
  channel_connected_ = false;
  Destroy();
}

bool CefChannel::Send(IPC::Message* msg)
{
  if(!channel_connected_) {
    AutoLock lock_scope(lock_);

    // Queue messages if the channel is not yet connected.
    pending_messages_.push(msg);
    return true;
  }

  if (!channel_.get()) {
    delete msg;
    return false;
  }

  return channel_->Send(msg);
}

#if defined(IPC_MESSAGE_LOG_ENABLED)
void CefChannel::OnSetIPCLoggingEnabled(bool enable) {
  if (enable)
    IPC::Logging::current()->Enable();
  else
    IPC::Logging::current()->Disable();
}
#endif  //  IPC_MESSAGE_LOG_ENABLED

void CefChannel::OnCreateTab(const std::string& browser_id, int index,
                             const std::string& url,
                             const std::string& referrer, bool foreground)
{
  scoped_refptr<CefConnector> connector(GetConnector(browser_id));
  if(!connector.get()) {
    NOTREACHED();
    return;
  }

  ChromeThread::PostTask(ChromeThread::UI, FROM_HERE,
      NewRunnableMethod(connector.get(), &CefConnector::OnCreateTab, index, url,
          referrer, foreground));
}

void CefChannel::OnTabCommand(const std::string& browser_id, int index,
                              int command)
{
  scoped_refptr<CefConnector> connector(GetConnector(browser_id));
  if(!connector.get()) {
    NOTREACHED();
    return;
  }

  ChromeThread::PostTask(ChromeThread::UI, FROM_HERE,
      NewRunnableMethod(connector.get(), &CefConnector::OnTabCommand, index,
          command));
}

void CefChannel::OnTabLoadAddress(const std::string& browser_id, int index,
                                  const std::string& url,
                                  const std::string& referrer)
{
  scoped_refptr<CefConnector> connector(GetConnector(browser_id));
  if(!connector.get()) {
    NOTREACHED();
    return;
  }

  ChromeThread::PostTask(ChromeThread::UI, FROM_HERE,
      NewRunnableMethod(connector.get(), &CefConnector::OnTabLoadAddress, index,
          url, referrer));
}

void CefChannel::OnTabExecuteJavaScript(const std::string& browser_id,
                                        int index,
                                        const std::wstring& frame_xpath,
                                        const std::wstring& jscript)
{
  scoped_refptr<CefConnector> connector(GetConnector(browser_id));
  if(!connector.get()) {
    NOTREACHED();
    return;
  }

  ChromeThread::PostTask(ChromeThread::UI, FROM_HERE,
      NewRunnableMethod(connector.get(), &CefConnector::OnTabExecuteJavaScript,
          index, frame_xpath, jscript));
}