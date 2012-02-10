// Copyright (c) 2012 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/renderer/render_view_observer.h"

#include <string>
#include <vector>

#include "libcef/common/cef_messages.h"
#include "libcef/renderer/content_renderer_client.h"

#include "base/string16.h"
#include "base/utf_string_conversions.h"
#include "content/public/renderer/render_view.h"
#include "net/http/http_util.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebString.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebURL.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebDocument.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebElement.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebNode.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebScriptSource.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebSecurityPolicy.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebView.h"

using WebKit::WebFrame;
using WebKit::WebScriptSource;
using WebKit::WebString;
using WebKit::WebURL;
using WebKit::WebView;

namespace {

const int64 kInvalidBrowserId = -1;
const int64 kInvalidFrameId = -1;

std::string DumpDocumentText(WebFrame* frame) {
  // We use the document element's text instead of the body text here because
  // not all documents have a body, such as XML documents.
  WebKit::WebElement documentElement = frame->document().documentElement();
  if (documentElement.isNull())
    return std::string();
  return documentElement.innerText().utf8();
}

}  // namespace

CefRenderViewObserver::CefRenderViewObserver(content::RenderView* render_view)
    : content::RenderViewObserver(render_view),
      content::RenderViewObserverTracker<CefRenderViewObserver>(render_view),
      browser_window_id_(kInvalidBrowserId),
      last_focused_frame_id_(kInvalidFrameId) {
}

CefRenderViewObserver::~CefRenderViewObserver() {
}

void CefRenderViewObserver::DidStartProvisionalLoad(WebKit::WebFrame* frame) {
  AddWebFrame(frame);
}

void CefRenderViewObserver::FrameDetached(WebFrame* frame) {
  int64 frame_id = frame->identifier();

  // Remove the frame from the map.
  FrameMap::iterator it = frames_.find(frame_id);
  DCHECK(it != frames_.end());
  frames_.erase(it);

  // Notify the browser that the frame has detached.
  Send(new CefHostMsg_FrameDetached(routing_id(), frame_id));
}

void CefRenderViewObserver::FocusedNodeChanged(const WebKit::WebNode& node) {
  // TODO(cef): This method is being used as a work-around for identifying frame
  // focus changes. The ideal approach would be implementating delegation from
  // ChromeClientImpl::focusedFrameChanged().

  WebFrame* focused_frame = NULL;

  // Try to identify the focused frame from the node.
  if (!node.isNull()) {
    const WebKit::WebDocument& document = node.document();
    if (!document.isNull())
      focused_frame = document.frame();
  }

  if (focused_frame == NULL) {
    // Try to identify the global focused frame.
    focused_frame = render_view()->GetWebView()->focusedFrame();
  }

  int64 frame_id = kInvalidFrameId;
  if (focused_frame != NULL)
    frame_id = focused_frame->identifier();

  // Don't send a message if the focused frame has not changed.
  if (frame_id == last_focused_frame_id_)
    return;

  last_focused_frame_id_ = frame_id;
  Send(new CefHostMsg_FrameFocusChange(routing_id(), frame_id));
}

bool CefRenderViewObserver::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(CefRenderViewObserver, message)
    IPC_MESSAGE_HANDLER(CefMsg_UpdateBrowserWindowId,
                        OnUpdateBrowserWindowId)
    IPC_MESSAGE_HANDLER(CefMsg_Response, OnResponse)
    IPC_MESSAGE_HANDLER(CefMsg_LoadString, OnLoadString)
    IPC_MESSAGE_HANDLER(CefMsg_LoadRequest, OnLoadRequest)
    IPC_MESSAGE_HANDLER(CefMsg_ExecuteCommand, OnExecuteCommand)
    IPC_MESSAGE_HANDLER(CefMsg_ExecuteCode, OnExecuteCode)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void CefRenderViewObserver::OnUpdateBrowserWindowId(int window_id) {
  browser_window_id_ = window_id;
}

void CefRenderViewObserver::OnResponse(int request_id,
                                       bool success,
                                       const std::string& response,
                                       const std::string& error) {
  NOTIMPLEMENTED();
}

void CefRenderViewObserver::OnLoadString(int64 frame_id,
                                         const std::string& string,
                                         const GURL& url) {
  WebFrame* web_frame = GetWebFrame(frame_id);
  if (web_frame != NULL)
    web_frame->loadHTMLString(string, url);
}

void CefRenderViewObserver::OnLoadRequest(
    const CefMsg_LoadRequest_Params& params) {
  WebFrame* web_frame = GetWebFrame(params.frame_id);
  if (web_frame == NULL)
    return;

  WebKit::WebURLRequest request(params.url);

  // Necessary for RenderViewImpl::decidePolicyForNavigation to avoid forking
  // this request.
  request.setRequestorID(-1);

  if (!params.method.empty())
    request.setHTTPMethod(ASCIIToUTF16(params.method));

  if (params.referrer.is_valid()) {
    WebString referrer = WebKit::WebSecurityPolicy::generateReferrerHeader(
        static_cast<WebKit::WebReferrerPolicy>(params.referrer_policy),
        params.url,
        WebString::fromUTF8(params.referrer.spec()));
    if (!referrer.isEmpty())
      request.setHTTPHeaderField(WebString::fromUTF8("Referer"), referrer);
  }

  if (params.first_party_for_cookies.is_valid())
    request.setFirstPartyForCookies(params.first_party_for_cookies);

  if (!params.headers.empty()) {
    for (net::HttpUtil::HeadersIterator i(params.headers.begin(),
                                          params.headers.end(), "\n");
         i.GetNext(); ) {
      request.addHTTPHeaderField(WebString::fromUTF8(i.name()),
                                 WebString::fromUTF8(i.values()));
    }
  }

  if (params.upload_data.get()) {
    string16 method = request.httpMethod();
    if (method == ASCIIToUTF16("GET") || method == ASCIIToUTF16("HEAD"))
      request.setHTTPMethod(ASCIIToUTF16("POST"));

    if (request.httpHeaderField(ASCIIToUTF16("Content-Type")).length() == 0) {
      request.setHTTPHeaderField(
          ASCIIToUTF16("Content-Type"),
          ASCIIToUTF16("application/x-www-form-urlencoded"));
    }

    WebKit::WebHTTPBody body;
    body.initialize();

    std::vector<net::UploadData::Element>* elements =
        params.upload_data->elements();
    std::vector<net::UploadData::Element>::const_iterator it =
        elements->begin();
    for (; it != elements->end(); ++it) {
      const net::UploadData::Element& element = *it;
      if (element.type() == net::UploadData::TYPE_BYTES) {
        WebKit::WebData data;
        data.assign(std::string(element.bytes().begin(),
                                element.bytes().end()).c_str(),
                    element.bytes().size());
        body.appendData(data);
      } else if (element.type() == net::UploadData::TYPE_FILE) {
        body.appendFile(element.file_path().value());
      } else {
        NOTREACHED();
      }
    }

    request.setHTTPBody(body);
  }

  web_frame->loadRequest(request);
}

void CefRenderViewObserver::OnExecuteCommand(int request_id,
                                             int64 frame_id,
                                             const std::string& command) {
  std::string response;

  WebFrame* web_frame = GetWebFrame(frame_id);
  if (web_frame == NULL)
    return;

  if (command == "GetSource") {
    response = web_frame->contentAsMarkup().utf8();
  } else if (command == "GetText") {
    response = DumpDocumentText(web_frame);
  } else {
    web_frame->executeCommand(WebString::fromUTF8(command));
  }

  if (request_id != -1) {
    // Send a response to the browser.
    Send(new CefHostMsg_ExecuteResponse(routing_id(), request_id, response));
  }
}

void CefRenderViewObserver::OnExecuteCode(
    const CefMsg_ExecuteCode_Params& params) {
  WebFrame* web_frame = GetWebFrame(params.frame_id);
  if (web_frame == NULL)
    return;

  std::string response;

  if (params.is_javascript) {
    web_frame->executeScript(WebScriptSource(UTF8ToUTF16(params.code),
                             WebURL(params.script_url),
                             params.script_start_line));
  } else {
    NOTIMPLEMENTED();
  }

  if (params.request_id != -1) {
    // Send a response to the browser.
    Send(new CefHostMsg_ExecuteResponse(routing_id(), params.request_id,
                                        response));
  }
}

void CefRenderViewObserver::AddWebFrame(WebKit::WebFrame* frame) {
  int64 frame_id = frame->identifier();

  // Frames are re-used between page loads. Only add the frame to the map once.
  if (frames_.find(frame_id) != frames_.end())
    return;

  frames_.insert(std::make_pair(frame_id, frame));

  int64 parent_id = frame->parent() == NULL ?
      kInvalidFrameId : frame->parent()->identifier();
  string16 name = frame->name();

  // Notify the browser that the frame has been identified.
  Send(new CefHostMsg_FrameIdentified(routing_id(), frame_id, parent_id, name));
}

WebKit::WebFrame* CefRenderViewObserver::GetWebFrame(int64 frame_id) {
  if (frame_id == kInvalidFrameId)
    return render_view()->GetWebView()->mainFrame();

  // Check if we already know about the frame.
  FrameMap::const_iterator it = frames_.find(frame_id);
  if (it != frames_.end())
    return it->second;

  // Check if the frame exists but we don't know about it yet.
  WebFrame* main_frame = render_view()->GetWebView()->mainFrame();
  WebFrame* cur = main_frame;
  do {
    if (cur->identifier() == frame_id) {
      AddWebFrame(cur);
      return cur;
    }
    cur = cur->traverseNext(true);
  } while (cur != main_frame);

  return NULL;
}
