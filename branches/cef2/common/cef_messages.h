// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

// Defines messages between the CEF host and Chromium browser processes.

#ifndef _CEF_MESSAGES_H
#define _CEF_MESSAGES_H

#include "base/basictypes.h"
#include "ipc/ipc_message_utils.h"

enum CefTabCommand {
  CEF_TC_TAB_DESTROY,
  CEF_TC_TAB_SELECT,
  CEF_TC_GOBACK,
  CEF_TC_GOFORWARD,
  CEF_TC_RELOAD,
  CEF_TC_STOPLOAD,
  CEF_TC_UNDO,
  CEF_TC_REDO,
  CEF_TC_CUT,
  CEF_TC_COPY,
  CEF_TC_PASTE,
  CEF_TC_DELETE,
  CEF_TC_SELECTALL,
  CEF_TC_PRINT,
  CEF_TC_VIEWSOURCE,
};

#define MESSAGES_INTERNAL_FILE "cef_messages_internal.h"
#include "ipc/ipc_message_macros.h"

#endif  // _CEF_MESSAGES_H
