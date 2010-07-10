// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "cef_context.h"
#include "cef_thread.h"
#include "cef_process.h"

#include "base/file_util.h"

// Global CefContext pointer
CefRefPtr<CefContext> _Context;

bool CefInitialize(const std::wstring& chrome_path,
                   bool multi_threaded_message_loop)
{
  // Return true if the context is already initialized
  if(_Context.get())
    return true;

  // Translate |chrome_path| to a FilePath.
  FilePath exe_path;
  if(!chrome_path.empty()) {
    exe_path = FilePath(chrome_path);
    // Check if the path is an executable file.
    if(_wcsicmp(exe_path.Extension().c_str(), L".exe") != 0)
      return false;
  } else {
    // Retrieve the current executable path.
    wchar_t buffer[MAX_PATH];
    buffer[0] = 0;
    GetModuleFileName(NULL, buffer, MAX_PATH);
    exe_path = FilePath(buffer);
    exe_path = exe_path.DirName();
    exe_path = exe_path.Append(L"chrome.exe");
  }

  // Verify that the executable path exists and is not a directory.
  file_util::FileInfo fi;
  if(!file_util::GetFileInfo(exe_path, &fi) || fi.is_directory)
    return false;

  // Create the new global context object
  _Context = new CefContext();
  // Initialize the global context
  return _Context->Initialize(exe_path, multi_threaded_message_loop);
}

void CefShutdown()
{
  // Verify that the context is already initialized
  if(!_Context.get())
    return;

  // Shut down the global context
  _Context->Shutdown();
  // Delete the global context object
  _Context = NULL;
}

void CefDoMessageLoopWork()
{
  // Verify that the context is already initialized.
  if(!_Context.get() || !_Context->process())
    return;

  if(!_Context->process()->CalledOnValidThread())
    return;
  _Context->process()->DoMessageLoopIteration();
}

CefRefPtr<CefProfile> CefCreateProfile(const std::wstring& user_profile)
{
  // Verify that the context is already initialized.
  if(!_Context.get())
    return NULL;

  return _Context->GetOrCreateProfile(user_profile);
}


// CefContext

CefContext::CefContext() : process_(NULL)
{
  
}

CefContext::~CefContext()
{
  // Just in case CefShutdown() isn't called
  Shutdown();
}

bool CefContext::Initialize(const FilePath& chrome_path,
                            bool multi_threaded_message_loop)
{
  chrome_path_ = chrome_path;
  process_ = new CefProcess(multi_threaded_message_loop);
  process_->CreateChildThreads();

  return true;
}

void CefContext::Shutdown()
{
  // Remove all existing profiles.
  RemoveAllProfiles();

  // Deleting the process will destroy the child threads.
  process_ = NULL;
}

CefRefPtr<CefProfile> CefContext::GetOrCreateProfile(
  const std::wstring& user_profile)
{
  AutoLock lock_scope(this);
  
  // Check if the profile already exists.
  ProfileMap::const_iterator it = profiles_.find(user_profile);
  if(it != profiles_.end())
    return it->second.get();

  // Create and return a new profile object.
  CefRefPtr<CefProfileImpl> profilePtr(new CefProfileImpl(user_profile));
  profiles_.insert(std::make_pair<std::wstring, CefRefPtr<CefProfileImpl>>(
      user_profile, profilePtr));

  return profilePtr.get();
}

void CefContext::RemoveProfile(const std::wstring& user_profile)
{
  CefRefPtr<CefProfileImpl> profile;
  
  {
    AutoLock lock_scope(this);

    // Call cleanup and remove the profile if it exists.
    ProfileMap::iterator it = profiles_.find(user_profile);
    if(it != profiles_.end()) {
      profile = it->second;
      profiles_.erase(it);
    }
  }

  if(profile.get())
    profile->Cleanup();
}

void CefContext::RemoveAllProfiles()
{
  ProfileMap profiles;
  
  {
    AutoLock lock_scope(this);
    profiles = profiles_;
    profiles_.clear();
  }

  // Call cleanup on all existing profiles.
  ProfileMap::iterator it = profiles.begin();
  for(; it != profiles.end(); ++it)
    it->second->Cleanup();
}
