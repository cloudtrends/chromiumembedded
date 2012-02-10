// Copyright (c) 2012 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CEF_LIBCEF_RENDERER_RENDER_MESSAGE_FILTER_H_
#define CEF_LIBCEF_RENDERER_RENDER_MESSAGE_FILTER_H_

#include <string>
#include "base/memory/ref_counted.h"
#include "base/message_loop_proxy.h"
#include "ipc/ipc_channel_proxy.h"

// This class sends and receives control messages on the renderer process.
class CefRenderMessageFilter : public IPC::ChannelProxy::MessageFilter {
 public:
  CefRenderMessageFilter();
  virtual ~CefRenderMessageFilter();

  // IPC::ChannelProxy::MessageFilter implementation.
  virtual void OnFilterAdded(IPC::Channel* channel) OVERRIDE;
  virtual void OnFilterRemoved() OVERRIDE;
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

 private:
  // Message handlers.
  void OnRegisterScheme(const std::string& scheme_name,
                        bool is_standard,
                        bool is_local,
                        bool is_display_isolated);
  void OnModifyCrossOriginWhitelistEntry(bool add,
                                         const std::string& source_origin,
                                         const std::string& target_protocol,
                                         const std::string& target_domain,
                                         bool allow_target_subdomains);
  void OnClearCrossOriginWhitelist();

  void RegisterSchemeOnRenderThread(const std::string& scheme_name,
                                    bool is_local,
                                    bool is_display_isolated);
  void ModifyCrossOriginWhitelistEntryOnRenderThread(
      bool add,
      const std::string& source_origin,
      const std::string& target_protocol,
      const std::string& target_domain,
      bool allow_target_subdomains);
  void ClearCrossOriginWhitelistOnRenderThread();

  IPC::Channel* channel_;
  scoped_refptr<base::MessageLoopProxy> render_loop_;

  DISALLOW_COPY_AND_ASSIGN(CefRenderMessageFilter);
};


#endif  // CEF_LIBCEF_RENDERER_RENDER_MESSAGE_FILTER_H_
