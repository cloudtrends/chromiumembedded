// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "libcef/browser/frame_impl.h"
#include "include/cef_stream.h"
#include "include/cef_request.h"
#include "libcef/browser/browser_impl.h"

namespace {

// Implementation of CommandResponseHandler for calling a CefStringVisitor.
class StringVisitHandler : public CefBrowserImpl::CommandResponseHandler {
 public:
  StringVisitHandler(CefRefPtr<CefStringVisitor> visitor)
      : visitor_(visitor) {
  }
  virtual void OnResponse(const std::string& response) OVERRIDE {
    visitor_->Visit(response);
  }
 private:
  CefRefPtr<CefStringVisitor> visitor_;

  IMPLEMENT_REFCOUNTING(StringVisitHandler);
};

// Implementation of CommandResponseHandler for calling ViewText().
class ViewTextHandler : public CefBrowserImpl::CommandResponseHandler {
 public:
  ViewTextHandler(CefRefPtr<CefFrameImpl> frame)
      : frame_(frame) {
  }
  virtual void OnResponse(const std::string& response) OVERRIDE {
    CefRefPtr<CefBrowser> browser = frame_->GetBrowser();
    if (browser.get())
      static_cast<CefBrowserImpl*>(browser.get())->ViewText(response);
  }
 private:
  CefRefPtr<CefFrameImpl> frame_;

  IMPLEMENT_REFCOUNTING(ViewTextHandler);
};

}  // namespace

CefFrameImpl::CefFrameImpl(CefBrowserImpl* browser,
                           int64 frame_id,
                           bool is_main_frame)
    : frame_id_(frame_id),
      is_main_frame_(is_main_frame),
      browser_(browser),
      is_focused_(false),
      parent_frame_id_(kInvalidFrameId) {
}

CefFrameImpl::~CefFrameImpl() {
}

bool CefFrameImpl::IsValid() {
  base::AutoLock lock_scope(state_lock_);
  return (browser_ != NULL);
}

void CefFrameImpl::Undo() {
  base::AutoLock lock_scope(state_lock_);
  if (browser_ && frame_id_ != kInvalidFrameId)
    browser_->SendCommand(frame_id_, "Undo", NULL);
}

void CefFrameImpl::Redo() {
  base::AutoLock lock_scope(state_lock_);
  if (browser_ && frame_id_ != kInvalidFrameId)
    browser_->SendCommand(frame_id_, "Redo", NULL);
}

void CefFrameImpl::Cut() {
  base::AutoLock lock_scope(state_lock_);
  if (browser_ && frame_id_ != kInvalidFrameId)
    browser_->SendCommand(frame_id_, "Cut", NULL);
}

void CefFrameImpl::Copy() {
  base::AutoLock lock_scope(state_lock_);
  if (browser_ && frame_id_ != kInvalidFrameId)
    browser_->SendCommand(frame_id_, "Copy", NULL);
}

void CefFrameImpl::Paste() {
  base::AutoLock lock_scope(state_lock_);
  if (browser_ && frame_id_ != kInvalidFrameId)
    browser_->SendCommand(frame_id_, "Paste", NULL);
}

void CefFrameImpl::Delete() {
  base::AutoLock lock_scope(state_lock_);
  if (browser_ && frame_id_ != kInvalidFrameId)
    browser_->SendCommand(frame_id_, "Delete", NULL);
}

void CefFrameImpl::SelectAll() {
  base::AutoLock lock_scope(state_lock_);
  if (browser_ && frame_id_ != kInvalidFrameId)
    browser_->SendCommand(frame_id_, "SelectAll", NULL);
}

void CefFrameImpl::Print() {
  NOTIMPLEMENTED();
}

void CefFrameImpl::ViewSource() {
  base::AutoLock lock_scope(state_lock_);
  if (browser_ && frame_id_ != kInvalidFrameId)
    browser_->SendCommand(frame_id_, "GetSource", new ViewTextHandler(this));
}

void CefFrameImpl::GetSource(CefRefPtr<CefStringVisitor> visitor) {
  base::AutoLock lock_scope(state_lock_);
  if (browser_ && frame_id_ != kInvalidFrameId) {
    browser_->SendCommand(frame_id_, "GetSource",
                          new StringVisitHandler(visitor));
  }
}

void CefFrameImpl::GetText(CefRefPtr<CefStringVisitor> visitor) {
  base::AutoLock lock_scope(state_lock_);
  if (browser_ && frame_id_ != kInvalidFrameId) {
    browser_->SendCommand(frame_id_, "GetText",
                          new StringVisitHandler(visitor));
  }
}

void CefFrameImpl::LoadRequest(CefRefPtr<CefRequest> request) {
  base::AutoLock lock_scope(state_lock_);
  if (browser_)
    browser_->LoadRequest((is_main_frame_ ? kMainFrameId : frame_id_), request);
}

void CefFrameImpl::LoadURL(const CefString& url) {
  base::AutoLock lock_scope(state_lock_);
  if (browser_)
    browser_->LoadURL((is_main_frame_ ? kMainFrameId : frame_id_), url);
}

void CefFrameImpl::LoadString(const CefString& string,
                              const CefString& url) {
  base::AutoLock lock_scope(state_lock_);
  if (browser_) {
    browser_->LoadString((is_main_frame_ ? kMainFrameId : frame_id_), string,
                         url);
  }
}

void CefFrameImpl::ExecuteJavaScript(const CefString& jsCode,
                                     const CefString& scriptUrl,
                                     int startLine) {
  base::AutoLock lock_scope(state_lock_);
  if (browser_) {
    browser_->SendCode((is_main_frame_ ? kMainFrameId : frame_id_), true,
                       jsCode, scriptUrl, startLine, NULL);
  }
}

bool CefFrameImpl::IsMain() {
  return is_main_frame_;
}

bool CefFrameImpl::IsFocused() {
  base::AutoLock lock_scope(state_lock_);
  return is_focused_;
}

CefString CefFrameImpl::GetName() {
  base::AutoLock lock_scope(state_lock_);
  return name_;
}

int64 CefFrameImpl::GetIdentifier() {
  return frame_id_;
}

CefRefPtr<CefFrame> CefFrameImpl::GetParent() {
  base::AutoLock lock_scope(state_lock_);

  if (is_main_frame_ || parent_frame_id_ == kInvalidFrameId)
    return NULL;

  if (browser_)
    return browser_->GetFrame(parent_frame_id_);

  return NULL;
}

CefString CefFrameImpl::GetURL() {
  base::AutoLock lock_scope(state_lock_);
  return url_;
}

CefRefPtr<CefBrowser> CefFrameImpl::GetBrowser() {
  base::AutoLock lock_scope(state_lock_);
  return browser_;
}

void CefFrameImpl::SetFocused(bool focused) {
  base::AutoLock lock_scope(state_lock_);
  is_focused_ = focused;
}

void CefFrameImpl::SetURL(const CefString& url) {
  base::AutoLock lock_scope(state_lock_);
  url_ = url;
}

void CefFrameImpl::SetName(const CefString& name) {
  base::AutoLock lock_scope(state_lock_);
  name_ = name;
}

void CefFrameImpl::SetParentId(int64 frame_id) {
  base::AutoLock lock_scope(state_lock_);
  parent_frame_id_ = frame_id;
}

void CefFrameImpl::Detach() {
  base::AutoLock lock_scope(state_lock_);
  browser_ = NULL;
}
