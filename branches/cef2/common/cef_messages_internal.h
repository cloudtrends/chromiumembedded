// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "base/shared_memory.h"
#include "build/build_config.h"
#include "gfx/native_widget_types.h"
#include "ipc/ipc_message_macros.h"

//-----------------------------------------------------------------------------
// Cef messages
// These are messages sent from the host process to the child process.
IPC_BEGIN_MESSAGES(Cef)
  // Tells the child process it should stop.
  IPC_MESSAGE_CONTROL0(CefMsg_AskBeforeShutdown)

  // Sent in response to CefHostMsg_ShutdownRequest to tell the child
  // process that it's safe to shutdown.
  IPC_MESSAGE_CONTROL0(CefMsg_Shutdown)

#if defined(IPC_MESSAGE_LOG_ENABLED)
  // Tell the child process to begin or end IPC message logging.
  // Like above, this is used by all ChildProcesses.
  IPC_MESSAGE_CONTROL1(CefMsg_SetIPCLoggingEnabled,
                       bool /* on or off */)
#endif

  // Create tab.
  IPC_MESSAGE_CONTROL5(CefMsg_CreateTab,
                       std::string /* browser_id */,
                       int /* tab_index */,
                       std::string /* url */,
                       std::string /* referrer */,
                       bool /* foreground */)

  // Execute tab command.
  IPC_MESSAGE_CONTROL3(CefMsg_TabCommand,
                       std::string /* browser_id */,
                       int /* tab_index */,
                       int /* command: 0 = go back, 1 = go forward, 2 = reload,
                                       3 = stop load, 4 = select tab, 5 = undo,
                                       6 = redo, 7 = cut, 8 = copy, 9 = paste,
                                       10 = delete, 11 = select all, 12 = print,
                                       13 = view source, 14 = destroy tab */)

  // Load address in tab.
  IPC_MESSAGE_CONTROL4(CefMsg_TabLoadAddress,
                       std::string /* browser_id */,
                       int /* tab_index */,
                       std::string /* url */,
                       std::string /* referrer */)

  // Execute JavaScript in tab.
  IPC_MESSAGE_CONTROL4(CefMsg_TabExecuteJavaScript,
                       std::string /* browser_id */,
                       int /* tab_index */,
                       std::wstring /* frame_xpath */,
                       std::wstring /* jscript */)

IPC_END_MESSAGES(Cef)

//-----------------------------------------------------------------------------
// CefHost messages
// These are messages sent from the child process to the host process.
IPC_BEGIN_MESSAGES(CefHost)
  IPC_MESSAGE_CONTROL0(CefHostMsg_ShutdownRequest)

  // Browser instance has been created.
  IPC_MESSAGE_CONTROL2(CefHostMsg_BrowserCreated,
                       std::string /* browser_id */,
                       gfx::NativeViewId /* browser_handle */)

  // Tab added.
  IPC_MESSAGE_CONTROL3(CefHostMsg_TabAdded,
                       std::string /* browser_id */,
                       int /* tab_index */,
                       bool /* foreground */)

  // Tab removed.
  IPC_MESSAGE_CONTROL2(CefHostMsg_TabRemoved,
                       std::string /* browser_id */,
                       int /* tab_index */)

  // Tab selected.
  IPC_MESSAGE_CONTROL3(CefHostMsg_TabSelected,
                       std::string /* browser_id */,
                       int /* tab_index */,
                       bool /* user_gesture */)

  // Tab contents have changed.
  IPC_MESSAGE_CONTROL4(CefHostMsg_TabChanged,
                       std::string /* browser_id */,
                       int /* tab_index */,
                       std::string /* url */,
                       std::wstring /* title */)

  // Tab navigation status.
  IPC_MESSAGE_CONTROL4(CefHostMsg_TabNavStatus,
                       std::string /* browser_id */,
                       int /* tab_index */,
                       bool /* can_go_back */,
                       bool /* can_go_forward */)

  // Tab load started.
  IPC_MESSAGE_CONTROL2(CefHostMsg_TabLoadStart,
                       std::string /* browser_id */,
                       int /* tab_index */)
  
  // Tab load stopped.
  IPC_MESSAGE_CONTROL2(CefHostMsg_TabLoadStop,
                       std::string /* browser_id */,
                       int /* tab_index */)

IPC_END_MESSAGES(CefHost)
