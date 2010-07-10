// Copyright (c) 2010 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cef_server.h"
#include "base/process_util.h"
#include "base/rand_util.h"
#include "base/string_util.h"
#include "cef_thread.h"

static std::string GenerateRandomProcessID(void* instance)
{
  // Note: the string must start with the current process id, this is how
  // child processes determine the pid of the parent.
  // Build the channel ID.  This is composed of a unique identifier for the
  // parent browser process, an identifier for the child instance, and a random
  // component. We use a random component so that a hacked child process can't
  // cause denial of service by causing future named pipe creation to fail.
  return StringPrintf("%d.%p.%d",
                      base::GetCurrentProcId(), instance,
                      base::RandInt(0, std::numeric_limits<int>::max()));
}

CefServer::CefServer(ServerListener* listener)
  : listener_(listener)
{
  DCHECK(listener_ != NULL);
	channel_id_ = GenerateRandomProcessID(this); 
}

CefServer::~CefServer()
{
}

void CefServer::CreateChannel()
{
  channel_.reset(new IPC::Channel(
      channel_id_, IPC::Channel::MODE_SERVER, this));
  if (!channel_->Connect()) {
    listener_->OnChannelError();
    return;
  }

  listener_->OnChannelCreated();
}

void CefServer::OnMessageReceived(const IPC::Message& msg)
{
#ifdef IPC_MESSAGE_LOG_ENABLED
  IPC::Logging* logger = IPC::Logging::current();
  if (msg.type() == IPC_LOGGING_ID) {
    logger->OnReceivedLoggingMessage(msg);
    return;
  }

  if (logger->Enabled())
    logger->OnPreDispatchMessage(msg);
#endif

  listener_->OnMessageReceived(msg);

#ifdef IPC_MESSAGE_LOG_ENABLED
  if (logger->Enabled())
    logger->OnPostDispatchMessage(msg, channel_id_);
#endif
}

void CefServer::OnChannelConnected(int32 peer_pid)
{
  listener_->OnChannelConnected(peer_pid);
}

void CefServer::OnChannelError()
{
  listener_->OnChannelError();
}

bool CefServer::Send(IPC::Message* msg)
{
  if (!channel_.get()) {
    delete msg;
    return false;
  }
  return channel_->Send(msg);
}
