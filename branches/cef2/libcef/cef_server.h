// Copyright (c) 2010 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _CEF_SERVER_H
#define _CEF_SERVER_H

// Must be included early
#include "ipc/ipc_logging.h"

#include "cef_thread.h"

#include "base/basictypes.h"
#include "base/scoped_ptr.h"
#include "ipc/ipc_channel.h"

// Methods of this class should only be called on the IO thread.
class CefServer : public IPC::Channel::Listener,
                  public IPC::Message::Sender,
                  public base::RefCountedThreadSafe<CefServer,
                      CefThread::DeleteOnThread<CefThread::IO>>
{
public:
  class ServerListener : public IPC::Channel::Listener
  {
  public:
    virtual void OnChannelCreated() =0;
  };

  CefServer(ServerListener* observer);
  ~CefServer();

  // Create the channel.
  void CreateChannel();

  // IPC::Channel::Listener methods.
  virtual void OnMessageReceived(const IPC::Message& message);
  virtual void OnChannelConnected(int32 peer_pid);
  virtual void OnChannelError();

  // IPC::Channel::Sender methods.
  virtual bool Send(IPC::Message* msg);

  std::string channel_id() { return channel_id_; }

private:
  scoped_ptr<IPC::Channel> channel_;
  std::string channel_id_;

  ServerListener* listener_;
};

#endif // _CEF_SERVER_H
