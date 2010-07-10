// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef _CEF_CONTEXT_H
#define _CEF_CONTEXT_H

#include "../include/cef2.h"
#include "cef_profile_impl.h"

#include "base/at_exit.h"
#include "base/file_path.h"
#include "base/message_loop.h"
#include "base/ref_counted.h"
#include <map>

class CefProcess;

class CefContext : public CefThreadSafeBase<CefBase>
{
public:
  CefContext();
  ~CefContext();

  // These methods will be called on the main application thread.
  bool Initialize(const FilePath& chrome_path,
                  bool multi_threaded_message_loop);
  void Shutdown();

  CefProcess* process() { return process_.get(); }

  // Manage profiles.
  CefRefPtr<CefProfile> GetOrCreateProfile(const std::wstring& user_profile);
  void RemoveProfile(const std::wstring& user_profile);
  void RemoveAllProfiles();

  const FilePath& chrome_path() { return chrome_path_; }

private:
  // Manages the various process threads.
  scoped_refptr<CefProcess> process_;

  // Initialize the AtExitManager to avoid asserts and possible memory leaks.
  base::AtExitManager at_exit_manager_;

  // Chrome.exe path.
  FilePath chrome_path_;

  // Map of profiles that currently exist.
  typedef std::map<std::wstring, CefRefPtr<CefProfileImpl> > ProfileMap;
  ProfileMap profiles_;
};

// Global context object pointer
extern CefRefPtr<CefContext> _Context;

#endif // _CONTEXT_H
