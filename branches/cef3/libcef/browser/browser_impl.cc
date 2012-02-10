// Copyright (c) 2012 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/browser/browser_impl.h"

#include <string>

#include "libcef/browser/browser_context.h"
#include "libcef/browser/context.h"
#include "libcef/browser/devtools_delegate.h"
#include "libcef/browser/navigate_params.h"
#include "libcef/browser/request_impl.h"
#include "libcef/browser/thread_util.h"
#include "libcef/common/cef_messages.h"
#include "libcef/common/http_header_utils.h"
#include "libcef/common/main_delegate.h"

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "content/browser/renderer_host/render_view_host.h"
#include "content/browser/renderer_host/resource_dispatcher_host.h"
#include "content/browser/renderer_host/resource_dispatcher_host_request_info.h"
#include "content/browser/tab_contents/title_updated_details.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"

namespace {

class CreateBrowserHelper {
 public:
  CreateBrowserHelper(const CefWindowInfo& windowInfo,
                      CefRefPtr<CefClient> client,
                      const CefString& url,
                      const CefBrowserSettings& settings)
                      : window_info_(windowInfo),
                        client_(client),
                        url_(url),
                        settings_(settings) {}

  CefWindowInfo window_info_;
  CefRefPtr<CefClient> client_;
  CefString url_;
  CefBrowserSettings settings_;
};

void CreateBrowserWithHelper(CreateBrowserHelper* helper) {
  CefBrowser::CreateBrowserSync(helper->window_info_, helper->client_,
      helper->url_, helper->settings_);
  delete helper;
}

}  // namespace

// static
bool CefBrowser::CreateBrowser(const CefWindowInfo& windowInfo,
                               CefRefPtr<CefClient> client,
                               const CefString& url,
                               const CefBrowserSettings& settings) {
  // Verify that the context is in a valid state.
  if (!CONTEXT_STATE_VALID()) {
    NOTREACHED() << "context not valid";
    return false;
  }

  // Verify that the settings structure is a valid size.
  if (settings.size != sizeof(cef_browser_settings_t)) {
    NOTREACHED() << "invalid CefBrowserSettings structure size";
    return false;
  }

  // Create the browser on the UI thread.
  CreateBrowserHelper* helper =
      new CreateBrowserHelper(windowInfo, client, url, settings);
  CEF_POST_TASK(CEF_UIT, base::Bind(CreateBrowserWithHelper, helper));

  return true;
}

// static
CefRefPtr<CefBrowser> CefBrowser::CreateBrowserSync(
    const CefWindowInfo& windowInfo,
    CefRefPtr<CefClient> client,
    const CefString& url,
    const CefBrowserSettings& settings) {
  // Verify that the context is in a valid state.
  if (!CONTEXT_STATE_VALID()) {
    NOTREACHED() << "context not valid";
    return NULL;
  }

  // Verify that the settings structure is a valid size.
  if (settings.size != sizeof(cef_browser_settings_t)) {
    NOTREACHED() << "invalid CefBrowserSettings structure size";
    return NULL;
  }

  // Verify that this method is being called on the UI thread.
  if (!CEF_CURRENTLY_ON_UIT()) {
    NOTREACHED() << "called on invalid thread";
    return NULL;
  }

  CefRefPtr<CefBrowserImpl> browser =
      CefBrowserImpl::Create(windowInfo, settings, client, NULL, NULL);
  if (!url.empty())
    browser->LoadURL(CefFrameImpl::kMainFrameId, url);
  return browser.get();
}

// static
CefRefPtr<CefBrowserImpl> CefBrowserImpl::Create(
    const CefWindowInfo& window_info,
    const CefBrowserSettings& settings,
    CefRefPtr<CefClient> client,
    TabContents* tab_contents,
    CefWindowHandle opener) {
  CEF_REQUIRE_UIT();

  if (tab_contents == NULL) {
    tab_contents = new TabContents(
        _Context->browser_context(),
        NULL,
        MSG_ROUTING_NONE,
        NULL,
        NULL);
  }

  CefRefPtr<CefBrowserImpl> browser =
      new CefBrowserImpl(window_info, settings, client, tab_contents, opener);
  if (!browser->PlatformCreateWindow())
    return NULL;

  _Context->AddBrowser(browser);

  if (client.get()) {
    CefRefPtr<CefLifeSpanHandler> handler = client->GetLifeSpanHandler();
    if (handler.get())
      handler->OnAfterCreated(browser.get());
  }

  return browser;
}

// static
CefRefPtr<CefBrowserImpl> CefBrowserImpl::GetBrowserForHost(
    RenderViewHost* host) {
  DCHECK(host);
  CEF_REQUIRE_UIT();
  TabContents* tab_contents = static_cast<TabContents*>(host->delegate());
  if (tab_contents)
    return static_cast<CefBrowserImpl*>(tab_contents->GetDelegate());
  return NULL;
}

// static
CefRefPtr<CefBrowserImpl> CefBrowserImpl::GetBrowserForContents(
    content::WebContents* contents) {
  DCHECK(contents);
  CEF_REQUIRE_UIT();
  return static_cast<CefBrowserImpl*>(contents->GetDelegate());
}

// static
CefRefPtr<CefBrowserImpl> CefBrowserImpl::GetBrowserForRequest(
    net::URLRequest* request) {
  DCHECK(request);
  CEF_REQUIRE_IOT();
  int render_process_id;
  int render_view_id;
  if (!ResourceDispatcherHost::RenderViewForRequest(request,
      &render_process_id, &render_view_id)) {
    return NULL;
  }
  return GetBrowserByRoutingID(render_process_id, render_view_id);
}

// static
CefRefPtr<CefBrowserImpl> CefBrowserImpl::GetBrowserByRoutingID(
      int render_process_id, int render_view_id) {
  if (CEF_CURRENTLY_ON_UIT()) {
    // Use the non-thread-safe but potentially faster approach.
    RenderViewHost* render_view_host =
        RenderViewHost::FromID(render_process_id, render_view_id);
    if (!render_view_host)
      return NULL;
    return GetBrowserForHost(render_view_host);
  } else {
    // Use the thread-safe approach.
    return _Context->GetBrowserByRoutingID(render_process_id, render_view_id);
  }
}

void CefBrowserImpl::CloseBrowser() {
  if (CEF_CURRENTLY_ON_UIT()) {
    PlatformCloseWindow();
  } else {
    CEF_POST_TASK(CEF_UIT,
        base::Bind(&CefBrowserImpl::PlatformCloseWindow, this));
  }
}

bool CefBrowserImpl::CanGoBack() {
  base::AutoLock lock_scope(state_lock_);
  return can_go_back_;
}

void CefBrowserImpl::GoBack() {
  if (CEF_CURRENTLY_ON_UIT()) {
    if (tab_contents_.get() && tab_contents_->GetController().CanGoBack())
      tab_contents_->GetController().GoBack();
  } else {
    CEF_POST_TASK(CEF_UIT,
        base::Bind(&CefBrowserImpl::GoBack, this));
  }
}

bool CefBrowserImpl::CanGoForward() {
  base::AutoLock lock_scope(state_lock_);
  return can_go_forward_;
}

void CefBrowserImpl::GoForward() {
  if (CEF_CURRENTLY_ON_UIT()) {
    if (tab_contents_.get() && tab_contents_->GetController().CanGoForward())
      tab_contents_->GetController().GoForward();
  } else {
    CEF_POST_TASK(CEF_UIT,
        base::Bind(&CefBrowserImpl::GoForward, this));
  }
}

bool CefBrowserImpl::IsLoading() {
  base::AutoLock lock_scope(state_lock_);
  return is_loading_;
}

void CefBrowserImpl::Reload() {
  if (CEF_CURRENTLY_ON_UIT()) {
    if (tab_contents_.get())
      tab_contents_->GetController().Reload(true);
  } else {
    CEF_POST_TASK(CEF_UIT,
        base::Bind(&CefBrowserImpl::Reload, this));
  }
}

void CefBrowserImpl::ReloadIgnoreCache() {
  if (CEF_CURRENTLY_ON_UIT()) {
    if (tab_contents_.get())
      tab_contents_->GetController().ReloadIgnoringCache(true);
  } else {
    CEF_POST_TASK(CEF_UIT,
        base::Bind(&CefBrowserImpl::ReloadIgnoreCache, this));
  }
}

void CefBrowserImpl::StopLoad() {
  if (CEF_CURRENTLY_ON_UIT()) {
    if (tab_contents_.get())
      tab_contents_->Stop();
  } else {
    CEF_POST_TASK(CEF_UIT,
        base::Bind(&CefBrowserImpl::StopLoad, this));
  }
}

void CefBrowserImpl::SetFocus(bool enable) {
  if (!enable)
    return;

  if (CEF_CURRENTLY_ON_UIT()) {
    if (tab_contents_.get())
      tab_contents_->Focus();
  } else {
    CEF_POST_TASK(CEF_UIT,
        base::Bind(&CefBrowserImpl::SetFocus, this, enable));
  }
}

void CefBrowserImpl::ParentWindowWillClose() {
}

CefWindowHandle CefBrowserImpl::GetWindowHandle() {
  return PlatformGetWindowHandle();
}

CefWindowHandle CefBrowserImpl::GetOpenerWindowHandle() {
  return opener_;
}

bool CefBrowserImpl::IsPopup() {
  return (opener_ != NULL);
}

bool CefBrowserImpl::HasDocument() {
  base::AutoLock lock_scope(state_lock_);
  return has_document_;
}

CefRefPtr<CefClient> CefBrowserImpl::GetClient() {
  return client_;
}

CefRefPtr<CefFrame> CefBrowserImpl::GetMainFrame() {
  return GetFrame(CefFrameImpl::kMainFrameId);
}

CefRefPtr<CefFrame> CefBrowserImpl::GetFocusedFrame() {
  return GetFrame(CefFrameImpl::kFocusedFrameId);
}

CefRefPtr<CefFrame> CefBrowserImpl::GetFrame(int64 identifier) {
  base::AutoLock lock_scope(state_lock_);

  if (main_frame_id_ == CefFrameImpl::kInvalidFrameId) {
    // A main frame does not exist yet. Return the placeholder frame that
    // provides limited functionality.
    return placeholder_frame_.get();
  }

  if (identifier == CefFrameImpl::kMainFrameId) {
    identifier = main_frame_id_;
  } else if (identifier == CefFrameImpl::kFocusedFrameId) {
    // Return the main frame if no focused frame is currently identified.
    if (focused_frame_id_ == CefFrameImpl::kInvalidFrameId)
      identifier = main_frame_id_;
    else
      identifier = focused_frame_id_;
  }

  if (identifier == CefFrameImpl::kInvalidFrameId)
    return NULL;

  FrameMap::const_iterator it = frames_.find(identifier);
  if (it != frames_.end())
    return it->second.get();

  return NULL;
}

CefRefPtr<CefFrame> CefBrowserImpl::GetFrame(const CefString& name) {
  base::AutoLock lock_scope(state_lock_);

  FrameMap::const_iterator it = frames_.begin();
  for (; it != frames_.end(); ++it) {
    if (it->second->GetName() == name)
      return it->second.get();
  }

  return NULL;
}

size_t CefBrowserImpl::GetFrameCount() {
  base::AutoLock lock_scope(state_lock_);
  return frames_.size();
}

void CefBrowserImpl::GetFrameIdentifiers(std::vector<int64>& identifiers) {
  base::AutoLock lock_scope(state_lock_);

  FrameMap::const_iterator it = frames_.begin();
  for (; it != frames_.end(); ++it)
    identifiers.push_back(it->first);
}

void CefBrowserImpl::GetFrameNames(std::vector<CefString>& names) {
  base::AutoLock lock_scope(state_lock_);

  FrameMap::const_iterator it = frames_.begin();
  for (; it != frames_.end(); ++it)
    names.push_back(it->second->GetName());
}

void CefBrowserImpl::Find(int identifier, const CefString& searchText,
                          bool forward, bool matchCase, bool findNext) {
  NOTIMPLEMENTED();
}

void CefBrowserImpl::StopFinding(bool clearSelection) {
  NOTIMPLEMENTED();
}

double CefBrowserImpl::GetZoomLevel() {
  NOTIMPLEMENTED();
  return 0;
}

void CefBrowserImpl::SetZoomLevel(double zoomLevel) {
  NOTIMPLEMENTED();
}

void CefBrowserImpl::ClearHistory() {
  NOTIMPLEMENTED();
}

void CefBrowserImpl::ShowDevTools() {
  NOTIMPLEMENTED();
}

void CefBrowserImpl::CloseDevTools() {
  NOTIMPLEMENTED();
}

void CefBrowserImpl::SetUniqueId(int unique_id) {
  CEF_REQUIRE_UIT();
  unique_id_ = unique_id;
  Send(new CefMsg_UpdateBrowserWindowId(routing_id(), unique_id));
}

void CefBrowserImpl::DestroyBrowser() {
  CEF_REQUIRE_UIT();

  if (client_.get()) {
    CefRefPtr<CefLifeSpanHandler> handler = client_->GetLifeSpanHandler();
    if (handler.get()) {
      // Notify the handler that the window is about to be closed.
      handler->OnBeforeClose(this);
    }
  }

  while (!queued_messages_.empty()) {
    delete queued_messages_.front();
    queued_messages_.pop();
  }

  registrar_.RemoveAll();
  content::WebContentsObserver::Observe(NULL);
  tab_contents_.reset(NULL);

  DetachAllFrames();

  // Remove the browser from the list maintained by the context.
  _Context->RemoveBrowser(this);
}

gfx::NativeView CefBrowserImpl::GetContentView() const {
  CEF_REQUIRE_UIT();
  if (!tab_contents_.get())
    return NULL;
  return tab_contents_->GetNativeView();
}

TabContents* CefBrowserImpl::GetTabContents() const {
  CEF_REQUIRE_UIT();
  return tab_contents_.get();
}

CefRefPtr<CefFrame> CefBrowserImpl::GetFrameForRequest(
    net::URLRequest* request) {
  CEF_REQUIRE_IOT();
  ResourceDispatcherHostRequestInfo* info =
      ResourceDispatcherHost::InfoForRequest(request);
  if (!info)
    return NULL;
  return GetOrCreateFrame(info->frame_id(), info->parent_frame_id(),
                          info->is_main_frame(), string16(), GURL());
}

void CefBrowserImpl::Navigate(const CefNavigateParams& params) {
  // Only known frame ids and kMainFrameId are supported.
  DCHECK(params.frame_id >= CefFrameImpl::kMainFrameId);

  CefMsg_LoadRequest_Params request;
  request.url = params.url;
  if (!request.url.is_valid()) {
    DLOG(ERROR) << "Invalid URL passed to CefBrowserImpl::Navigate: " <<
        params.url;
    return;
  }

  request.method = params.method;
  request.referrer = params.referrer.url;
  request.referrer_policy = params.referrer.policy;
  request.frame_id = params.frame_id;
  request.first_party_for_cookies = params.first_party_for_cookies;
  request.headers = params.headers;
  request.load_flags = params.load_flags;
  request.upload_data = params.upload_data;

  Send(new CefMsg_LoadRequest(routing_id(), request));

  if (tab_contents_.get())
    tab_contents_->Focus();
}

void CefBrowserImpl::LoadRequest(int64 frame_id,
                                 CefRefPtr<CefRequest> request) {
  CefNavigateParams params(GURL(std::string(request->GetURL())),
                           content::PAGE_TRANSITION_TYPED);
  params.method = request->GetMethod();
  params.frame_id = frame_id;
  params.first_party_for_cookies =
    GURL(std::string(request->GetFirstPartyForCookies()));

  CefRequest::HeaderMap headerMap;
  request->GetHeaderMap(headerMap);
  if (!headerMap.empty())
    params.headers = HttpHeaderUtils::GenerateHeaders(headerMap);

  CefRefPtr<CefPostData> postData = request->GetPostData();
  if (postData.get()) {
    CefPostDataImpl* impl = static_cast<CefPostDataImpl*>(postData.get());
    params.upload_data = new net::UploadData();
    impl->Get(*params.upload_data.get());
  }

  params.load_flags = request->GetFlags();

  Navigate(params);
}

void CefBrowserImpl::LoadURL(int64 frame_id, const std::string& url) {
  if (frame_id == CefFrameImpl::kMainFrameId) {
    // Go through the navigation controller.
    if (CEF_CURRENTLY_ON_UIT()) {
      if (tab_contents_.get()) {
        GURL gurl = GURL(url);

        if (!gurl.is_valid() && !gurl.has_scheme()) {
          // Try to add "http://" at the beginning
          std::string new_url = std::string("http://") + url;
          gurl = GURL(new_url);
        }

        if (!gurl.is_valid()) {
          DLOG(ERROR) << "Invalid URL passed to CefBrowserImpl::LoadURL: " <<
              url;
          return;
        }

        tab_contents_->GetController().LoadURL(
            gurl,
            content::Referrer(),
            content::PAGE_TRANSITION_TYPED,
            std::string());
        tab_contents_->Focus();
      }
    } else {
      CEF_POST_TASK(CEF_UIT,
          base::Bind(&CefBrowserImpl::LoadURL, this, frame_id, url));
    }
  } else {
    CefNavigateParams params(GURL(url), content::PAGE_TRANSITION_TYPED);
    params.frame_id = frame_id;
    Navigate(params);
  }
}

void CefBrowserImpl::LoadString(int64 frame_id, const std::string& string,
                                const std::string& url) {
  // Only known frame ids or kMainFrameId are supported.
  DCHECK(frame_id >= CefFrameImpl::kMainFrameId);

  Send(new CefMsg_LoadString(routing_id(), frame_id, string, GURL(url)));
}

void CefBrowserImpl::SendCommand(
    int64 frame_id,
    const std::string& command,
    CefRefPtr<CommandResponseHandler> responseHandler) {
  // Only known frame ids are supported.
  DCHECK(frame_id > CefFrameImpl::kMainFrameId);

  // Execute on the UI thread because the CommandResponseHandler map is not
  // thread safe.
  if (CEF_CURRENTLY_ON_UIT()) {
    int request_id = -1;
    if (responseHandler.get())
      request_id = RegisterCommandResponseHandler(responseHandler);

    Send(new CefMsg_ExecuteCommand(routing_id(), request_id, frame_id,
                                   command));
  } else {
    CEF_POST_TASK(CEF_UIT,
        base::Bind(&CefBrowserImpl::SendCommand, this, frame_id, command,
                   responseHandler));
  }
}

void CefBrowserImpl::SendCode(
    int64 frame_id,
    bool is_javascript,
    const std::string& code,
    const std::string& script_url,
    int script_start_line,
    CefRefPtr<CommandResponseHandler> responseHandler) {
  // Only known frame ids are supported.
  DCHECK(frame_id >= CefFrameImpl::kMainFrameId);

  // Execute on the UI thread because the CommandResponseHandler map is not
  // thread safe.
  if (CEF_CURRENTLY_ON_UIT()) {
    CefMsg_ExecuteCode_Params params;

    if (responseHandler.get())
      params.request_id = RegisterCommandResponseHandler(responseHandler);
    else
      params.request_id = -1;

    params.frame_id = frame_id;
    params.is_javascript = is_javascript;
    params.code = code;
    params.script_url = GURL(script_url);
    params.script_start_line = script_start_line;

    Send(new CefMsg_ExecuteCode(routing_id(), params));
  } else {
    CEF_POST_TASK(CEF_UIT,
        base::Bind(&CefBrowserImpl::SendCode, this, frame_id, is_javascript,
                   code, script_url, script_start_line, responseHandler));
  }
}

bool CefBrowserImpl::ViewText(const std::string& text) {
  return PlatformViewText(text);
}

CefBrowserImpl::CefBrowserImpl(const CefWindowInfo& window_info,
                               const CefBrowserSettings& settings,
                               CefRefPtr<CefClient> client,
                               TabContents* tab_contents,
                               CefWindowHandle opener)
    : content::WebContentsObserver(tab_contents),
      window_info_(window_info),
      settings_(settings),
      client_(client),
      opener_(opener),
      render_process_id_(tab_contents->GetRenderProcessHost()->GetID()),
      render_view_id_(routing_id()),
      unique_id_(0),
      received_page_title_(false),
      is_loading_(false),
      can_go_back_(false),
      can_go_forward_(false),
      has_document_(false),
      queue_messages_(true),
      main_frame_id_(CefFrameImpl::kInvalidFrameId),
      focused_frame_id_(CefFrameImpl::kInvalidFrameId),
      next_request_id_(0) {
  tab_contents_.reset(tab_contents);
  tab_contents->SetDelegate(this);

  registrar_.Add(this, content::NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED,
                 content::Source<content::WebContents>(tab_contents));

  placeholder_frame_ =
      new CefFrameImpl(this, CefFrameImpl::kInvalidFrameId, true);

  CefDevToolsDelegate* devtools_delegate = _Context->devtools_delegate();
  if (devtools_delegate)
    devtools_delegate->AddWebContents(tab_contents);
}

content::WebContents* CefBrowserImpl::OpenURLFromTab(
    content::WebContents* source,
    const content::OpenURLParams& params) {
  // Start the new navigation.
  CefNavigateParams nav_params(params.url, params.transition);
  nav_params.referrer = params.referrer;
  nav_params.frame_id = params.source_frame_id;
  nav_params.disposition = params.disposition;
  nav_params.user_gesture = true;
  nav_params.override_encoding = params.override_encoding;
  nav_params.is_renderer_initiated = params.is_renderer_initiated;
  nav_params.transferred_global_request_id =
      params.transferred_global_request_id;
  Navigate(nav_params);

  return source;
}

void CefBrowserImpl::LoadingStateChanged(content::WebContents* source) {
  int current_index = tab_contents_->GetController().GetCurrentEntryIndex();
  int max_index = tab_contents_->GetController().GetEntryCount() - 1;

  bool is_loading, can_go_back, can_go_forward;

  {
    base::AutoLock lock_scope(state_lock_);
    is_loading = is_loading_ = tab_contents_->IsLoading();
    can_go_back = can_go_back_ = (current_index > 0);
    can_go_forward = can_go_forward_ = (current_index < max_index);
  }

  if (client_.get()) {
    CefRefPtr<CefDisplayHandler> handler = client_->GetDisplayHandler();
    if (handler.get()) {
      handler->OnLoadingStateChange(this, is_loading, can_go_back,
                                    can_go_forward);
    }
  }
}

bool CefBrowserImpl::ShouldCreateWebContents(
      content::WebContents* web_contents,
      int route_id,
      WindowContainerType window_container_type,
      const string16& frame_name) {
  // Start with the current browser window's client and settings.
  pending_client_ = client_;
  pending_settings_ = settings_;

  // TODO(cef): Figure out how to populate these values.
  // See: http://crbug.com/110510
  CefPopupFeatures features;
  pending_url_ = CefString();

  pending_window_info_ = CefWindowInfo();
#if defined(OS_WIN)
  pending_window_info_.SetAsPopup(NULL, CefString());
#endif

#if (defined(OS_WIN) || defined(OS_MACOSX))
  // Default to the size from the popup features.
  if (features.xSet)
    pending_window_info_.x = features.x;
  if (features.ySet)
    pending_window_info_.y = features.y;
  if (features.widthSet)
    pending_window_info_.width = features.width;
  if (features.heightSet)
    pending_window_info_.height = features.height;
#endif

  if (client_.get()) {
    CefRefPtr<CefLifeSpanHandler> handler = client_->GetLifeSpanHandler();
    // Give the handler an opportunity to modify window attributes, handler,
    // or cancel the window creation.
    if (handler.get() &&
        handler->OnBeforePopup(this, features, pending_window_info_,
                               pending_url_, pending_client_,
                               pending_settings_)) {
      pending_client_ = NULL;
      return false;
    }
  }

  return true;
}

void CefBrowserImpl::WebContentsCreated(content::WebContents* source_contents,
                                        int64 source_frame_id,
                                        const GURL& target_url,
                                        content::WebContents* new_contents) {
  CefWindowHandle opener = NULL;
  if (source_contents)
    opener = GetBrowserForContents(source_contents)->GetWindowHandle();

  CefRefPtr<CefBrowserImpl> browser = CefBrowserImpl::Create(
      pending_window_info_, pending_settings_, pending_client_,
      static_cast<TabContents*>(new_contents), opener);
  if (browser.get() && !pending_url_.empty())
    browser->LoadURL(CefFrameImpl::kMainFrameId, pending_url_);

  pending_client_ = NULL;
}

void CefBrowserImpl::DidNavigateMainFramePostCommit(content::WebContents* tab) {
  {
    base::AutoLock lock_scope(state_lock_);
    has_document_ = false;
  }

  // Allow the new page to set the title again.
  received_page_title_ = false;
}

content::JavaScriptDialogCreator* CefBrowserImpl::GetJavaScriptDialogCreator() {
  // TODO(cef): Provide a custom JavaScriptDialogCreator implementation to
  // support JavaScript alerts, prompts, etc.
  return content::WebContentsDelegate::GetJavaScriptDialogCreator();
}

void CefBrowserImpl::RunFileChooser(
    content::WebContents* tab,
    const content::FileChooserParams& params) {
  // TODO(cef): Implement this method to run the file chooser dialog.
}

void CefBrowserImpl::UpdatePreferredSize(content::WebContents* source,
                                         const gfx::Size& pref_size) {
  PlatformSizeTo(pref_size.width(), pref_size.height());
}

void CefBrowserImpl::RenderViewReady() {
  // Send the queued messages.
  queue_messages_ = false;
  while (!queued_messages_.empty()) {
    Send(queued_messages_.front());
    queued_messages_.pop();
  }
}

void CefBrowserImpl::RenderViewGone(base::TerminationStatus status) {
  queue_messages_ = true;
}

void CefBrowserImpl::DidCommitProvisionalLoadForFrame(
      int64 frame_id,
      bool is_main_frame,
      const GURL& url,
      content::PageTransition transition_type) {
  CefRefPtr<CefFrame> frame = GetOrCreateFrame(frame_id,
      CefFrameImpl::kUnspecifiedFrameId, is_main_frame, string16(), url);
  OnLoadStart(frame, url, transition_type);
  if (is_main_frame)
    OnAddressChange(frame, url);
}

void CefBrowserImpl::DidFailProvisionalLoad(
      int64 frame_id,
      bool is_main_frame,
      const GURL& validated_url,
      int error_code,
      const string16& error_description) {
  CefRefPtr<CefFrame> frame = GetOrCreateFrame(frame_id,
      CefFrameImpl::kUnspecifiedFrameId, is_main_frame, string16(),
      validated_url);
  OnLoadError(frame, validated_url, error_code, error_description);
}

void CefBrowserImpl::DocumentAvailableInMainFrame() {
  base::AutoLock lock_scope(state_lock_);
  has_document_ = true;
}

void CefBrowserImpl::DidFinishLoad(int64 frame_id,
                                   const GURL& validated_url,
                                   bool is_main_frame) {
  CefRefPtr<CefFrame> frame = GetOrCreateFrame(frame_id,
      CefFrameImpl::kUnspecifiedFrameId, is_main_frame, string16(),
      validated_url);
  OnLoadEnd(frame, validated_url);
}

void CefBrowserImpl::DidFailLoad(int64 frame_id,
                                 const GURL& validated_url,
                                 bool is_main_frame,
                                 int error_code,
                                 const string16& error_description) {
  CefRefPtr<CefFrame> frame = GetOrCreateFrame(frame_id,
      CefFrameImpl::kUnspecifiedFrameId, is_main_frame, string16(),
      validated_url);
  OnLoadError(frame, validated_url, error_code, error_description);
  OnLoadEnd(frame, validated_url);
}

bool CefBrowserImpl::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(CefBrowserImpl, message)
    IPC_MESSAGE_HANDLER(CefHostMsg_FrameIdentified, OnFrameIdentified)
    IPC_MESSAGE_HANDLER(CefHostMsg_FrameDetached, DetachFrame)
    IPC_MESSAGE_HANDLER(CefHostMsg_FrameFocusChange, SetFocusedFrame)
    IPC_MESSAGE_HANDLER(CefHostMsg_ExecuteResponse, OnExecuteResponse)
    IPC_MESSAGE_HANDLER(CefHostMsg_Request, OnRequest)
    IPC_MESSAGE_HANDLER(CefHostMsg_RequestForIOThread, OnRequestForIOThread)
    IPC_MESSAGE_HANDLER(CefHostMsg_ResponseAck, OnResponseAck)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

bool CefBrowserImpl::Send(IPC::Message* message) {
  if (CEF_CURRENTLY_ON_UIT()) {
    if (queue_messages_) {
      queued_messages_.push(message);
      return true;
    } else {
      return content::WebContentsObserver::Send(message);
    }
  } else {
    CEF_POST_TASK(CEF_UIT,
        base::Bind(base::IgnoreResult(&CefBrowserImpl::Send), this, message));
    return true;
  }
}

void CefBrowserImpl::OnFrameIdentified(int64 frame_id,
                                       int64 parent_frame_id,
                                       string16 name) {
  bool is_main_frame = (parent_frame_id == CefFrameImpl::kMainFrameId);
  GetOrCreateFrame(frame_id, parent_frame_id, is_main_frame, name, GURL());
}

void CefBrowserImpl::OnExecuteResponse(int request_id,
                                       const std::string& response) {
  CefRefPtr<CommandResponseHandler> handler =
      PopCommandResponseHandler(request_id);
  DCHECK(handler.get());
  if (handler.get())
    handler->OnResponse(response);
}

void CefBrowserImpl::OnRequest(const CefHostMsg_Request_Params& params) {
  NOTIMPLEMENTED();
}

void CefBrowserImpl::OnRequestForIOThread(
    int routing_id,
    const CefHostMsg_Request_Params& params) {
  NOTIMPLEMENTED();
}

void CefBrowserImpl::OnResponseAck(int routing_id) {
  NOTIMPLEMENTED();
}

void CefBrowserImpl::Observe(int type,
                             const content::NotificationSource& source,
                             const content::NotificationDetails& details) {
  DCHECK(type == content::NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED);

  if (type == content::NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED) {
    TitleUpdatedDetails* title =
        content::Details<TitleUpdatedDetails>(details).ptr();

    if (received_page_title_)
      return;

    if (title->entry()) {
      if (client_.get()) {
        CefRefPtr<CefDisplayHandler> handler = client_->GetDisplayHandler();
        if (handler.get()) {
          CefString title_str = title->entry()->GetTitleForDisplay("");
          handler->OnTitleChange(this, title_str);
        }
      }

      received_page_title_ = title->explicit_set();
    }
  }
}

CefRefPtr<CefFrame> CefBrowserImpl::GetOrCreateFrame(int64 frame_id,
                                                     int64 parent_frame_id,
                                                     bool is_main_frame,
                                                     string16 frame_name,
                                                     const GURL& frame_url) {
  DCHECK(frame_id > CefFrameImpl::kInvalidFrameId);
  if (frame_id <= CefFrameImpl::kInvalidFrameId)
    return NULL;

  CefString url;
  if (frame_url.is_valid())
    url = frame_url.spec();

  CefString name;
  if (!frame_name.empty())
    name = frame_name;

  base::AutoLock lock_scope(state_lock_);

  if (is_main_frame)
    main_frame_id_ = frame_id;

  CefRefPtr<CefFrameImpl> frame;

  // Check if a frame object already exists.
  FrameMap::const_iterator it = frames_.find(frame_id);
  if (it != frames_.end())
    frame = it->second.get();

  if (!frame.get()) {
    // Create a new frame object.
    frame = new CefFrameImpl(this, frame_id, is_main_frame);
    frames_.insert(std::make_pair(frame_id, frame));
  }

  if (!url.empty())
    frame->SetURL(url);
  if (!name.empty())
    frame->SetName(name);
  if (parent_frame_id != CefFrameImpl::kUnspecifiedFrameId)
    frame->SetParentId(parent_frame_id);

  return frame.get();
}

void CefBrowserImpl::DetachFrame(int64 frame_id) {
  base::AutoLock lock_scope(state_lock_);

  FrameMap::iterator it = frames_.find(frame_id);
  if (it != frames_.end()) {
    it->second->Detach();
    frames_.erase(it);
  }

  if (main_frame_id_ == frame_id)
    main_frame_id_ = CefFrameImpl::kInvalidFrameId;
  if (focused_frame_id_ ==  frame_id)
    focused_frame_id_ = CefFrameImpl::kInvalidFrameId;
}

void CefBrowserImpl::DetachAllFrames() {
  base::AutoLock lock_scope(state_lock_);

  FrameMap::const_iterator it = frames_.begin();
  for (; it != frames_.end(); ++it)
    it->second->Detach();

  frames_.clear();

  if (main_frame_id_ != CefFrameImpl::kInvalidFrameId)
    main_frame_id_ = CefFrameImpl::kInvalidFrameId;
  if (focused_frame_id_ != CefFrameImpl::kInvalidFrameId)
    focused_frame_id_ = CefFrameImpl::kInvalidFrameId;
}

void CefBrowserImpl::SetFocusedFrame(int64 frame_id) {
  base::AutoLock lock_scope(state_lock_);

  if (focused_frame_id_ != CefFrameImpl::kInvalidFrameId) {
    // Unfocus the previously focused frame.
    FrameMap::const_iterator it = frames_.find(frame_id);
    if (it != frames_.end())
      it->second->SetFocused(false);
  }

  if (frame_id != CefFrameImpl::kInvalidFrameId) {
    // Focus the newly focused frame.
    FrameMap::iterator it = frames_.find(frame_id);
    if (it != frames_.end()) {
      it->second->SetFocused(true);
      focused_frame_id_ = frame_id;
      return;
    }
  }

  // No valid frame found.
  focused_frame_id_ = CefFrameImpl::kInvalidFrameId;
}

int CefBrowserImpl::GetNextRequestId() {
  return CefAtomicIncrement(&next_request_id_);
}

int CefBrowserImpl::RegisterCommandResponseHandler(
    CefRefPtr<CommandResponseHandler> handler) {
  CEF_REQUIRE_UIT();
  int request_id = GetNextRequestId();
  command_response_handlers_.insert(std::make_pair(request_id, handler));
  return request_id;
}

CefRefPtr<CefBrowserImpl::CommandResponseHandler>
CefBrowserImpl::PopCommandResponseHandler(
    int request_id) {
  CEF_REQUIRE_UIT();
  CefRefPtr<CefBrowserImpl::CommandResponseHandler> handler;
  CommandResponseHandlerMap::iterator it =
      command_response_handlers_.find(request_id);
  if (it != command_response_handlers_.end()) {
    handler = it->second;
    command_response_handlers_.erase(it);
  }
  return handler;
}

void CefBrowserImpl::OnAddressChange(CefRefPtr<CefFrame> frame,
                                     const GURL& url) {
  if (client_.get()) {
    CefRefPtr<CefDisplayHandler> handler = client_->GetDisplayHandler();
    if (handler.get()) {
      // Notify the handler of an address change.
      handler->OnAddressChange(this, GetMainFrame(), url.spec());
    }
  }
}

void CefBrowserImpl::OnLoadStart(CefRefPtr<CefFrame> frame,
                                 const GURL& url,
                                 content::PageTransition transition_type) {
  if (client_.get()) {
    CefRefPtr<CefLoadHandler> handler = client_->GetLoadHandler();
    if (handler.get()) {
      // Notify the handler that loading has started.
      handler->OnLoadStart(this, frame);
    }
  }
}

void CefBrowserImpl::OnLoadError(CefRefPtr<CefFrame> frame,
                                 const GURL& url,
                                 int error_code,
                                 const string16& error_description) {
  if (client_.get()) {
    CefRefPtr<CefLoadHandler> handler = client_->GetLoadHandler();
    if (handler.get()) {
      // Notify the handler that loading has failed.
      handler->OnLoadError(this, frame,
          static_cast<cef_handler_errorcode_t>(error_code),
          CefString(error_description),
          url.spec());
    }
  }
}

void CefBrowserImpl::OnLoadEnd(CefRefPtr<CefFrame> frame,
                               const GURL& url) {
  if (client_.get()) {
    CefRefPtr<CefLoadHandler> handler = client_->GetLoadHandler();
    if (handler.get()) {
      // Notify the handler that loading has ended.
      // TODO(cef): Identify the HTTP status code.
      handler->OnLoadEnd(this, frame, 200);
    }
  }
}
