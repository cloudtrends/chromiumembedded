// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "cef_browser_impl.h"
#include "cef_profile_impl.h"
#include "cef_tab_impl.h"
#include "common/cef_messages.h"

CefBrowserImpl::CefBrowserImpl(CefRefPtr<CefProfileImpl> profile,
  const std::wstring& id_string)
  : profile_(profile), id_string_(id_string), parent_(NULL),
    initialized_(true), shutting_down_(false), handle_(NULL)
{
}

CefBrowserImpl::CefBrowserImpl(CefRefPtr<CefProfileImpl> profile,
  const std::wstring& id_string, CefWindowHandle parent, gfx::Rect rect,
  const std::wstring& command_line)
  : profile_(profile), id_string_(id_string), parent_(parent), rect_(rect),
    command_line_(command_line), initialized_(false), shutting_down_(false),
    handle_(NULL)
{
}

CefBrowserImpl::~CefBrowserImpl()
{
}

void CefBrowserImpl::Cleanup()
{
  RemoveAllTabs();

  CefWindowHandle handle = NULL;
  {
    AutoLock lock_scope(this);
    handle = handle_;

    // Remove pending tabs without calling Cleanup() since the tab was never
    // actually created.
    pending_tabs_.clear();
  }

  if(handle) {
    CefThread::PostTask(CefThread::UI, FROM_HERE,
        NewRunnableMethod(this, &CefBrowserImpl::OnNotifyDestroyed));
  }
}

void CefBrowserImpl::RemoveAllTabs()
{
  TabMap tabs;
  
  {
    AutoLock lock_scope(this);
    tabs = tabs_;
    tabs_.clear();
    selected_tab_ = NULL;
  }

  // Call cleanup on all existing tabs.
  TabMap::iterator it = tabs.begin();
  for(; it != tabs.end(); ++it)
    it->second->Cleanup();
}

CefRefPtr<CefTabImpl> CefBrowserImpl::GetOrCreateTab(int index)
{
  AutoLock lock_scope(this);

  // Locate the existing tab if any.
  TabMap::const_iterator it = tabs_.find(index);
  if(it != tabs_.end())
    return it->second;

  // Create and return a new tab object. The new tab object is assumed to
  // already be initialized.
  CefRefPtr<CefTabImpl> tabPtr(new CefTabImpl(this, index, true));
  tabs_.insert(std::make_pair<int, CefRefPtr<CefTabImpl>>(index, tabPtr));

  return tabPtr.get();
}

void CefBrowserImpl::OnCreated(CefWindowHandle handle)
{
  {
    AutoLock lock_scope(this);
    handle_ = handle;
    initialized_ = true;
  }
  
  CefThread::PostTask(CefThread::UI, FROM_HERE,
      NewRunnableMethod(this, &CefBrowserImpl::OnNotifyCreated));
}

void CefBrowserImpl::OnTabAdded(int index, bool foreground)
{
  AutoLock lock_scope(this);

  CefRefPtr<CefTabImpl> tab;

  // Check if a pending tab was created.
  if(!pending_tabs_.empty() && pending_tabs_.front()->index() == index) {
     tab = pending_tabs_.front();
     pending_tabs_.pop_front();
     tab->set_initialized(true);
  }

  if(!tabs_.empty()) {
    // Change the index of all tabs that come after the added tab.
    TabMap new_tabs;
    int new_index;

    TabMap::const_iterator it = tabs_.begin();
    for(; it != tabs_.end(); ++it) {
      new_index = it->first;
      if(new_index >= index) {
        new_index++;
        it->second->set_index(new_index);
      }

      new_tabs.insert(std::make_pair<int, CefRefPtr<CefTabImpl>>(
          new_index, it->second));
    }

    tabs_ = new_tabs;
  }

  
  if(!tab.get()) {
    // Create a new tab.
    tab = GetOrCreateTab(index);
  } else {
    // Add the pending tab to the tabs list.
    tabs_.insert(std::make_pair<int, CefRefPtr<CefTabImpl>>(index, tab));
  }

  tab->OnCreated();
}

void CefBrowserImpl::OnTabRemoved(int index)
{
  CefRefPtr<CefTabImpl> tab;
  
  {
    AutoLock lock_scope(this);

    // Call cleanup and remove the tab if it exists.
    TabMap::iterator it = tabs_.find(index);
    if(it != tabs_.end()) {
      tab = it->second;
      tabs_.erase(it);
    }

    if(!tabs_.empty()) {
      // Change the index of all tabs that come after the removed tab.
      TabMap new_tabs;
      int new_index;

      it = tabs_.begin();
      for(; it != tabs_.end(); ++it) {
        new_index = it->first;
        if(new_index > index) {
          new_index--;
          it->second->set_index(new_index);
        }

        new_tabs.insert(std::make_pair<int, CefRefPtr<CefTabImpl>>(
            new_index, it->second));
      }

      tabs_ = new_tabs;
    }

    if(selected_tab_.get() && selected_tab_->index() == index)
      selected_tab_ = NULL;
  }

  if(tab.get())
    tab->Cleanup();
}

void CefBrowserImpl::OnTabSelected(int index, bool user_gesture)
{
  AutoLock lock_scope(this);
  if(selected_tab_.get())
    selected_tab_->OnSelected(false);
  selected_tab_ = GetOrCreateTab(index);
  selected_tab_->OnSelected(true);
}

void CefBrowserImpl::OnTabChanged(int index, const std::wstring& url,
                                   const std::wstring& title)
{
  GetOrCreateTab(index)->OnChanged(url, title);
}

void CefBrowserImpl::OnTabNavStatus(int index, bool can_go_back,
                                    bool can_go_forward)
{
  GetOrCreateTab(index)->OnNavStatus(can_go_back, can_go_forward);
}

void CefBrowserImpl::OnTabLoadStart(int index)
{
  GetOrCreateTab(index)->OnLoadStart();
}

void CefBrowserImpl::OnTabLoadStop(int index)
{
  GetOrCreateTab(index)->OnLoadStop();
}

CefRefPtr<CefProfile> CefBrowserImpl::GetProfile()
{
  return profile_.get();
}

void CefBrowserImpl::Destroy()
{
  std::wstring id_string;
  {
    AutoLock lock_scope(this);

    if(shutting_down_)
      return;
    shutting_down_ = true;
    id_string = id_string_;
  }

   // Remove this browser from the profile.  This will result in a call to
  // CefBrowserImpl::Cleanup().
  profile_->RemoveBrowser(id_string);
}

CefRefPtr<CefTab> CefBrowserImpl::CreateTab(int index, const std::wstring& url,
                                            const std::wstring& referrer,
                                            bool foreground)
{
  CefRefPtr<CefTabImpl> tab;

  {
    AutoLock lock_scope(this);

    if(!initialized_ || shutting_down_)
      return NULL;
  
    // Create a new tab object that is not yet initialized.
    tab = new CefTabImpl(this, index, false);
    pending_tabs_.push_back(tab);
  }

  CefThread::PostTask(CefThread::IO, FROM_HERE,
      NewRunnableMethod(this, &CefBrowserImpl::OnSendCreateTab, index, url,
          referrer, foreground));

  return tab.get();
}

int CefBrowserImpl::GetTabCount()
{
  AutoLock lock_scope(this);
  return tabs_.size();
}

CefRefPtr<CefTab> CefBrowserImpl::GetTab(int index)
{
  AutoLock lock_scope(this);
  TabMap::const_iterator it = tabs_.find(index);
  if(it != tabs_.end())
    return it->second.get();
  return NULL;
}

CefRefPtr<CefTab> CefBrowserImpl::GetSelectedTab()
{
  return selected_tab_.get();
}

void CefBrowserImpl::OnNotifyCreated()
{
  profile_->listener_manager()->browser()->OnCreate(this);
}

void CefBrowserImpl::OnNotifyDestroyed()
{
  profile_->listener_manager()->browser()->OnDestroy(this);
}

void CefBrowserImpl::OnSendCreateTab(int index, const std::wstring& url,
                                     const std::wstring& referrer,
                                     bool foreground)
{
  profile_->Send(new CefMsg_CreateTab(WideToUTF8(id_string_), index,
      WideToUTF8(url), WideToUTF8(referrer), foreground));
}
