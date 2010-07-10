// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef _CEF_LISTENER_MANAGER_H
#define _CEF_LISTENER_MANAGER_H

#include "../include/cef2.h"
#include "base/basictypes.h"
#include <vector>

// Template class that listener manager classes extend.
template <class ClassName>
class CefListenerManagerBase : public CefThreadSafeBase<ClassName>
{
public:
  CefListenerManagerBase() {}
  virtual ~CefListenerManagerBase()
  {
    RemoveAll();
  }

  bool Has(CefRefPtr<ClassName> listener)
  {
    AutoLock lock_scope(this);
    ListenerVector::const_iterator it = listeners_.begin();
    for(; it != listeners_.end(); ++it) {
      if(it->second.get() == listener.get())
        return true;
    }
    return false;
  }

  bool Has(const std::wstring& id_string)
  {
    AutoLock lock_scope(this);
    ListenerVector::const_iterator it = listeners_.begin();
    for(; it != listeners_.end(); ++it) {
      if(it->first == id_string)
        return true;
    }
    return false;
  }

  void Add(const std::wstring& id_string, CefRefPtr<ClassName> listener)
  {
    if(Has(listener))
      return;
    
    AutoLock lock_scope(this);
    listeners_.push_back(
        std::make_pair<std::wstring,CefRefPtr<ClassName>>(id_string, listener));
  }

  void Remove(CefRefPtr<ClassName> listener)
  {
    AutoLock lock_scope(this);
    ListenerVector::iterator it = listeners_.begin();
    for(; it != listeners_.end(); ++it) {
      if(it->second.get() == listener.get()) {
        listeners_.erase(it);
        return;
      }
    }
  }

  void Remove(const std::wstring& id_string)
  {
    AutoLock lock_scope(this);
    ListenerVector::iterator it = listeners_.begin();
    for(; it != listeners_.end(); ++it) {
      if(it->first == id_string) {
        listeners_.erase(it);
        return;
      }
    }
  }

  void RemoveAll()
  {
    AutoLock lock_scope(this);
    listeners_.empty();
  }

protected:
  typedef ClassName ListenerType;
  typedef std::vector<std::pair<std::wstring, CefRefPtr<ClassName>>>
      ListenerVector;
  ListenerVector listeners_;

private:
  DISALLOW_COPY_AND_ASSIGN(CefListenerManagerBase);
};

// Macro used to deliver notifications to listeners.
#define LISTENER_NOTIFY(method_call)                        \
  AutoLock lock_scope(this);                                \
  ListenerVector::const_iterator it = listeners_.begin();   \
  for(; it != listeners_.end(); ++it) {                     \
    it->second->method_call;                                \
  }


// Manage CefBrowserListeners
class CefBrowserListenerManager
    : public CefListenerManagerBase<CefBrowserListener>
{
public:
  virtual void OnCreate(CefRefPtr<CefBrowser> browser)
  {
    LISTENER_NOTIFY(OnCreate(browser));
  }

  virtual void OnDestroy(CefRefPtr<CefBrowser> browser)
  {
    LISTENER_NOTIFY(OnDestroy(browser));
  }
};


// Manage CefTabListeners
class CefTabListenerManager
    : public CefListenerManagerBase<CefTabListener>
{
public:
  virtual void OnCreate(CefRefPtr<CefTab> tab)
  {
    LISTENER_NOTIFY(OnCreate(tab));
  }

  virtual void OnDestroy(CefRefPtr<CefTab> tab)
  {
    LISTENER_NOTIFY(OnDestroy(tab));
  }
  
  virtual void OnAddressChange(CefRefPtr<CefTab> tab,
                               const std::wstring& url)
  {
    LISTENER_NOTIFY(OnAddressChange(tab, url));
  }

  virtual void OnTitleChange(CefRefPtr<CefTab> tab,
                             const std::wstring& title)
  {
    LISTENER_NOTIFY(OnTitleChange(tab, title));
  }

  virtual void OnNavStateChange(CefRefPtr<CefTab> tab, bool can_go_back,
                                bool can_go_forward)
  {
    LISTENER_NOTIFY(OnNavStateChange(tab, can_go_back, can_go_forward));
  }

  virtual void OnSelectChange(CefRefPtr<CefTab> tab, bool selected)
  {
    LISTENER_NOTIFY(OnSelectChange(tab, selected));
  }

  virtual void OnLoadStart(CefRefPtr<CefTab> tab)
  {
    LISTENER_NOTIFY(OnLoadStart(tab));
  }

  virtual void OnLoadStop(CefRefPtr<CefTab> tab)
  {
    LISTENER_NOTIFY(OnLoadStop(tab));
  }
};


// Class that holds all listener managers.
class CefListenerManager : public CefThreadSafeBase<CefBase>
{
public:
  CefListenerManager()
    : browser_(new CefBrowserListenerManager()),
      tab_(new CefTabListenerManager())
  {
  }
  
  CefRefPtr<CefBrowserListenerManager> browser() { return browser_; }
  CefRefPtr<CefTabListenerManager> tab() { return tab_; }

protected:
  CefRefPtr<CefBrowserListenerManager> browser_;
  CefRefPtr<CefTabListenerManager> tab_;

  DISALLOW_COPY_AND_ASSIGN(CefListenerManager);
};

#endif // _LISTENER_MANAGER_H
