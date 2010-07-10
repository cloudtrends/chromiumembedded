// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef_service.h"
#include "cef_connector.h"
#include "cef_channel.h"
#include "chrome/browser/chrome_thread.h"

CefService::CefService() : pending_parent_(NULL)
{
}

CefService::~CefService()
{
  RemoveAllChannels();
}

void CefService::SetPending(gfx::NativeWindow parent, const gfx::Rect& bounds,
                            const std::string& channel_id,
                            const std::string& browser_id)
{
  AutoLock lock_scope(lock_);

  pending_parent_ = parent;
  pending_bounds_ = bounds;
  pending_channel_id_ = channel_id;
  pending_browser_id_ = browser_id;
}

CefConnector* CefService::AssignPending(Browser* browser)
{
  gfx::NativeWindow pending_parent;
  gfx::Rect pending_bounds;
  std::string pending_channel_id;
  std::string pending_browser_id;
  
  {
    AutoLock lock_scope(lock_);
    if(pending_channel_id_.empty())
      return NULL;

    pending_parent = pending_parent_;
    pending_bounds = pending_bounds_;
    pending_channel_id = pending_channel_id_;
    pending_browser_id = pending_browser_id_;

    pending_parent_ = NULL;
    pending_bounds_.SetRect(0,0,0,0);
    pending_channel_id_.clear();
    pending_browser_id_.clear();
  }

  return GetOrCreateChannel(pending_channel_id)->CreateConnector(pending_parent,
      pending_bounds, pending_browser_id, browser);
}

void CefService::RemoveChannel(const std::string& channel_id)
{
  scoped_refptr<CefChannel> channel;
  
  {
    AutoLock lock_scope(lock_);

    ChannelMap::iterator it = channels_.find(channel_id);
    if(it != channels_.end()) {
      channel = it->second;
      channels_.erase(it);
    }
  }

  if(channel.get())
    channel->Cleanup();
}

void CefService::RemoveAllChannels()
{
  ChannelMap channels;
  
  {
    AutoLock lock_scope(lock_);
    channels = channels_;
    channels_.clear();
  }

  ChannelMap::iterator it = channels.begin();
  for(; it != channels.end(); ++it)
    it->second->Cleanup();
}

CefChannel* CefService::GetOrCreateChannel(const std::string& channel_id)
{
  AutoLock lock_scope(lock_);
  
  ChannelMap::const_iterator it = channels_.find(channel_id);
  if(it != channels_.end())
    return it->second.get();

  scoped_refptr<CefChannel> channel(new CefChannel(this, channel_id));
  channels_.insert(std::make_pair<std::string,scoped_refptr<CefChannel>>(
      channel_id, channel));

  ChromeThread::PostTask(ChromeThread::IO, FROM_HERE,
      NewRunnableMethod(channel.get(), &CefChannel::CreateChannel));
  
  return channel;
}
