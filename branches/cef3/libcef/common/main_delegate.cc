// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/common/main_delegate.h"
#include "libcef/browser/content_browser_client.h"
#include "libcef/plugin/content_plugin_client.h"
#include "libcef/renderer/content_renderer_client.h"
#include "libcef/utility/content_utility_client.h"

#include "base/command_line.h"
#include "base/file_path.h"
#include "base/file_util.h"
#include "base/path_service.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/common/content_switches.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"

CefMainDelegate::CefMainDelegate(const std::string& locale) : locale_(locale) {
}

CefMainDelegate::~CefMainDelegate() {
}

bool CefMainDelegate::BasicStartupComplete(int* exit_code) {
  // Pre-process the global command-line object.
  CommandLine* command_line = CommandLine::ForCurrentProcess();

  // TODO(cef): Figure out how to support the sandbox.
  if (!command_line->HasSwitch(switches::kNoSandbox))
    command_line->AppendSwitch(switches::kNoSandbox);

  return false;
}

void CefMainDelegate::PreSandboxStartup() {
  const CommandLine& command_line = *CommandLine::ForCurrentProcess();
  std::string process_type =
      command_line.GetSwitchValueASCII(switches::kProcessType);

  content::SetContentClient(&content_client_);
  InitializeContentClient(process_type);

  InitializeResourceBundle();
}

void CefMainDelegate::SandboxInitialized(const std::string& process_type) {
}

int CefMainDelegate::RunProcess(
    const std::string& process_type,
    const content::MainFunctionParams& main_function_params) {
  if (process_type.empty()) {
    // Use our own browser process runner.
    browser_runner_.reset(content::BrowserMainRunner::Create());

    // Initialize browser process state. Results in a call to
    // CefBrowserMain::GetMainMessageLoop().
    int exit_code = browser_runner_->Initialize(main_function_params);
    if (exit_code >= 0)
      return exit_code;

    return 0;
  }

  return -1;
}

void CefMainDelegate::ProcessExiting(const std::string& process_type) {
  ResourceBundle::CleanupSharedInstance();
}

#if defined(OS_MACOSX)
bool CefMainDelegate::ProcessRegistersWithSystemProcess(
    const std::string& process_type) {
  return false;
}

bool CefMainDelegate::ShouldSendMachPort(const std::string& process_type) {
  return false;
}

bool CefMainDelegate::DelaySandboxInitialization(
    const std::string& process_type) {
  return false;
}

#elif defined(OS_POSIX)
content::ZygoteForkDelegate* CefMainDelegate::ZygoteStarting() {
  return NULL;
}

void CefMainDelegate::ZygoteForked() {
  const CommandLine& command_line = *CommandLine::ForCurrentProcess();
  std::string process_type =
      command_line.GetSwitchValueASCII(switches::kProcessType);
  InitializeCefContentClient(process_type);
}
#endif  // OS_MACOSX

void CefMainDelegate::ShutdownBrowser() {
  if (browser_runner_.get()) {
    browser_runner_->Shutdown();
    browser_runner_.reset(NULL);
  }
}

void CefMainDelegate::InitializeContentClient(
    const std::string& process_type) {
  if (process_type.empty()) {
    browser_client_.reset(new CefContentBrowserClient);
    content::GetContentClient()->set_browser(browser_client_.get());

    // Single-process is an unsupported and not fully tested mode.
    const CommandLine& command_line = *CommandLine::ForCurrentProcess();
    if (command_line.HasSwitch(switches::kSingleProcess)) {
      content::RenderProcessHost::set_run_renderer_in_process(true);
      renderer_client_.reset(new CefContentRendererClient);
      content::GetContentClient()->set_renderer(renderer_client_.get());
    }
  } else if (process_type == switches::kRendererProcess) {
    renderer_client_.reset(new CefContentRendererClient);
    content::GetContentClient()->set_renderer(renderer_client_.get());
  } else if (process_type == switches::kPluginProcess) {
    plugin_client_.reset(new CefContentPluginClient);
    content::GetContentClient()->set_plugin(plugin_client_.get());
  } else if (process_type == switches::kUtilityProcess) {
    utility_client_.reset(new CefContentUtilityClient);
    content::GetContentClient()->set_utility(utility_client_.get());
  }
}

void CefMainDelegate::InitializeResourceBundle() {
  FilePath pak_dir;
  PathService::Get(base::DIR_MODULE, &pak_dir);

  FilePath pak_file = pak_dir.Append(FILE_PATH_LITERAL("cef.pak"));
  PathService::Override(ui::FILE_RESOURCES_PAK, pak_file);

  const std::string loaded_locale =
      ui::ResourceBundle::InitSharedInstanceWithLocale(locale_);
  CHECK(!loaded_locale.empty()) << "Locale could not be found for " << locale_;

#if defined(OS_WIN)
  // Explicitly load cef.pak on Windows.
  if (file_util::PathExists(pak_file))
    ResourceBundle::AddDataPackToSharedInstance(pak_file);
  else
    NOTREACHED() << "Could not load cef.pak";
#endif
}
