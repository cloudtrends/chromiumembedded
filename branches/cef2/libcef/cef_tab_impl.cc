// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "cef_tab_impl.h"
#include "cef_browser_impl.h"
#include "cef_profile_impl.h"
#include "cef_thread.h"

CefTabImpl::CefTabImpl(CefRefPtr<CefBrowserImpl> browser, int index,
                       bool initialized)
  : browser_(browser), index_(index), initialized_(initialized),
    selected_(false), can_go_back_(false), can_go_forward_(false)
{
}

CefTabImpl::~CefTabImpl()
{
}

void CefTabImpl::Cleanup()
{
  CefThread::PostTask(CefThread::UI, FROM_HERE,
      NewRunnableMethod(this, &CefTabImpl::OnNotifyDestroy));
}

void CefTabImpl::OnCreated()
{
  CefThread::PostTask(CefThread::UI, FROM_HERE,
      NewRunnableMethod(this, &CefTabImpl::OnNotifyCreate));
}

void CefTabImpl::OnChanged(const std::wstring& url, const std::wstring& title)
{
  bool url_changed = false, title_changed = false;
  
  {
    AutoLock scope_lock(this);

    if(url.compare(url_) != 0) {
      url_ = url;
      url_changed = true;
    }

    if(title.compare(title_) != 0) {
      title_ = title;
      title_changed = true;
    }
  }

  if(url_changed) {
    CefThread::PostTask(CefThread::UI, FROM_HERE,
        NewRunnableMethod(this, &CefTabImpl::OnNotifyAddressChange, url));
  }
  if(title_changed) {
    CefThread::PostTask(CefThread::UI, FROM_HERE,
        NewRunnableMethod(this, &CefTabImpl::OnNotifyTitleChange, title));
  }
}

void CefTabImpl::OnNavStatus(bool can_go_back, bool can_go_forward)
{
  bool changed = false;
  
  {
    AutoLock scope_lock(this);

    if(can_go_back != can_go_back_) {
      can_go_back_ = can_go_back;
      changed = true;
    }

    if(can_go_forward != can_go_forward_) {
      can_go_forward_ = can_go_forward;
      changed = true;
    }
  }

  if(changed) {
    CefThread::PostTask(CefThread::UI, FROM_HERE,
        NewRunnableMethod(this, &CefTabImpl::OnNotifyNavStateChange,
            can_go_back, can_go_forward));
  }
}

void CefTabImpl::OnSelected(bool selected)
{
  bool changed = false;

  {
    AutoLock scope_lock(this);
    if(selected != selected_) {
      selected_ = selected;
      changed = true;
    }
  }

  if(changed && selected) {
    CefThread::PostTask(CefThread::UI, FROM_HERE,
        NewRunnableMethod(this, &CefTabImpl::OnNotifySelectChange, selected));
  }
}

void CefTabImpl::OnLoadStart()
{
  {
    AutoLock scope_lock(this);
    loading_ = true;
  }

  CefThread::PostTask(CefThread::UI, FROM_HERE,
      NewRunnableMethod(this, &CefTabImpl::OnNotifyLoadStart));
}

void CefTabImpl::OnLoadStop()
{
  {
    AutoLock scope_lock(this);
    loading_ = false;
  }

  CefThread::PostTask(CefThread::UI, FROM_HERE,
      NewRunnableMethod(this, &CefTabImpl::OnNotifyLoadStop));
}

CefRefPtr<CefBrowser> CefTabImpl::GetBrowser()
{
  return browser_.get();
}

std::wstring CefTabImpl::GetAddress()
{
  std::wstring url;

  {
    AutoLock lock_scope(this);
    url = url_;
  }

  return url;
}

std::wstring CefTabImpl::GetTitle()
{
  std::wstring title;

  {
    AutoLock lock_scope(this);
    title = title_;
  }

  return title;
}

void CefTabImpl::LoadAddress(const std::wstring& url,
                             const std::wstring& referrer)
{
  CefThread::PostTask(CefThread::IO, FROM_HERE,
      NewRunnableMethod(this, &CefTabImpl::OnSendLoadAddress, url, referrer));
}

void CefTabImpl::ExecuteJavaScript(const std::wstring& frame_xpath, 
                                   const std::wstring& jscript)
{
  CefThread::PostTask(CefThread::IO, FROM_HERE,
      NewRunnableMethod(this, &CefTabImpl::OnSendExecuteJavaScript, frame_xpath,
          jscript));
}

void CefTabImpl::OnNotifyCreate()
{
  browser_->profile()->listener_manager()->tab()->OnCreate(this);
}

void CefTabImpl::OnNotifyDestroy()
{
  browser_->profile()->listener_manager()->tab()->OnDestroy(this);
}

void CefTabImpl::OnNotifyAddressChange(const std::wstring& url)
{
  browser_->profile()->listener_manager()->tab()->OnAddressChange(this, url);
}

void CefTabImpl::OnNotifyTitleChange(const std::wstring& title)
{
  browser_->profile()->listener_manager()->tab()->OnTitleChange(this, title);
}

void CefTabImpl::OnNotifyNavStateChange(bool can_go_back, bool can_go_forward)
{
  browser_->profile()->listener_manager()->tab()->OnNavStateChange(this,
      can_go_back, can_go_forward);
}

void CefTabImpl::OnNotifySelectChange(bool selected)
{
   browser_->profile()->listener_manager()->tab()->OnSelectChange(this,
      selected);
}

void CefTabImpl::OnNotifyLoadStart()
{
   browser_->profile()->listener_manager()->tab()->OnLoadStart(this);
}

void CefTabImpl::OnNotifyLoadStop()
{
  browser_->profile()->listener_manager()->tab()->OnLoadStop(this);
}

void CefTabImpl::OnCommand(CefTabCommand command)
{
  CefThread::PostTask(CefThread::IO, FROM_HERE,
      NewRunnableMethod(this, &CefTabImpl::OnSendCommand, command));
}

void CefTabImpl::OnSendCommand(CefTabCommand command)
{
  browser_->profile()->Send(new CefMsg_TabCommand(
      WideToUTF8(browser_->id_string()), index_, command));
}

void CefTabImpl::OnSendLoadAddress(const std::wstring& url,
                                   const std::wstring& referrer)
{
  browser_->profile()->Send(new CefMsg_TabLoadAddress(
      WideToUTF8(browser_->id_string()), index_, WideToUTF8(url),
      WideToUTF8(referrer)));
}

void CefTabImpl::OnSendExecuteJavaScript(const std::wstring& frame_xpath,
                                         const std::wstring& jscript)
{
  browser_->profile()->Send(new CefMsg_TabExecuteJavaScript(
      WideToUTF8(browser_->id_string()), index_, frame_xpath, jscript));
}
