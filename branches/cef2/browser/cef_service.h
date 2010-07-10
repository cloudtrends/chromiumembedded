// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef _CEF_SERVICE_H
#define _CEF_SERVICE_H

#include "base/basictypes.h"
#include "base/linked_ptr.h"
#include "base/lock.h"
#include "base/ref_counted.h"
#include "gfx/native_widget_types.h"
#include "gfx/rect.h"
#include <map>

class Browser;
class CefChannel;
class CefConnector;

// This class will be destroyed when the profile is destroyed.
class CefService
{
public:
  CefService();
  ~CefService();

  // Called before a Browser is created to set the pending state information.
  void SetPending(gfx::NativeWindow parent,
                  const gfx::Rect& bounds,
                  const std::string& channel_id,
                  const std::string& browser_id);

  // Called by the Browser constructor to create a connector for the Browser
  // using the pending state information.
  CefConnector* AssignPending(Browser* browser);

  // Will be called by CefChannel::Destroy().
  void RemoveChannel(const std::string& channel_id);
  // Will be called by the CefService destructor.
  void RemoveAllChannels();

private:
  // Retrieves the existing channel or creates a new one.
  CefChannel* GetOrCreateChannel(const std::string& channel_id);

  // Class level lock.  Used to protect internal data structures that may be
  // accessed from other threads.
  Lock lock_;

  gfx::NativeWindow pending_parent_;
  gfx::Rect pending_bounds_;
  std::string pending_channel_id_;
  std::string pending_browser_id_;

  // Map of channels that currently exist.
  typedef std::map<std::string, scoped_refptr<CefChannel> > ChannelMap;
  ChannelMap channels_;

  DISALLOW_COPY_AND_ASSIGN(CefService);
};

#endif // _CEF_SERVICE_H
