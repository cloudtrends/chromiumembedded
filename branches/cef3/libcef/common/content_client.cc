// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/common/content_client.h"
#include "include/cef_version.h"
#include "libcef/common/cef_switches.h"

#include "base/command_line.h"
#include "base/string_piece.h"
#include "base/stringprintf.h"
#include "content/public/common/content_switches.h"
#include "ui/base/resource/resource_bundle.h"
#include "webkit/glue/user_agent.h"

CefContentClient::~CefContentClient() {
}

void CefContentClient::SetActiveURL(const GURL& url) {
}

void CefContentClient::SetGpuInfo(const content::GPUInfo& gpu_info) {
}

void CefContentClient::AddPepperPlugins(
    std::vector<content::PepperPluginInfo>* plugins) {
}

void CefContentClient::AddNPAPIPlugins(
    webkit::npapi::PluginList* plugin_list) {
}

bool CefContentClient::CanHandleWhileSwappedOut(const IPC::Message& msg) {
  return false;
}

std::string CefContentClient::GetUserAgent(bool* overriding) const {
  static CommandLine& command_line = *CommandLine::ForCurrentProcess();
  if (command_line.HasSwitch(switches::kUserAgent)) {
    *overriding = true;
    return command_line.GetSwitchValueASCII(switches::kUserAgent);
  } else {
    std::string product_version;

    if (command_line.HasSwitch(switches::kProductVersion)) {
      *overriding = true;
      product_version =
          command_line.GetSwitchValueASCII(switches::kProductVersion);
    } else {
      *overriding = false;
      product_version = base::StringPrintf("Chrome/%d.%d.%d.%d",
          CHROME_VERSION_MAJOR, CHROME_VERSION_MINOR, CHROME_VERSION_BUILD,
          CHROME_VERSION_PATCH);
    }

    return webkit_glue::BuildUserAgentFromProduct(product_version);
  }
}

string16 CefContentClient::GetLocalizedString(int message_id) const {
  return ResourceBundle::GetSharedInstance().GetLocalizedString(message_id);
}

base::StringPiece CefContentClient::GetDataResource(int resource_id) const {
  return ResourceBundle::GetSharedInstance().GetRawDataResource(resource_id);
}

#if defined(OS_WIN)
bool CefContentClient::SandboxPlugin(CommandLine* command_line,
                                       sandbox::TargetPolicy* policy) {
  return false;
}
#endif

#if defined(OS_MACOSX)
bool CefContentClient::GetSandboxProfileForSandboxType(
    int sandbox_type,
    int* sandbox_profile_resource_id) const {
  return false;
}
#endif
