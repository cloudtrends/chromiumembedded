// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefclient/client_handler.h"
#include <stdio.h>
#include <sstream>
#include <string>
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/wrapper/cef_stream_resource_handler.h"
#include "cefclient/binding_test.h"
#include "cefclient/cefclient.h"
#include "cefclient/resource_util.h"
#include "cefclient/string_util.h"


ClientHandler::ClientHandler()
  : m_MainHwnd(NULL),
    m_BrowserId(0),
    m_EditHwnd(NULL),
    m_BackHwnd(NULL),
    m_ForwardHwnd(NULL),
    m_StopHwnd(NULL),
    m_ReloadHwnd(NULL),
    m_bFormElementHasFocus(false) {
  CreateProcessMessageDelegates(process_message_delegates_);
  CreateRequestDelegates(request_delegates_);
}

ClientHandler::~ClientHandler() {
}

bool ClientHandler::OnProcessMessageRecieved(
    CefRefPtr<CefBrowser> browser,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {
  bool handled = false;

  // Execute delegate callbacks.
  ProcessMessageDelegateSet::iterator it = process_message_delegates_.begin();
  for (; it != process_message_delegates_.end() && !handled; ++it) {
    handled = (*it)->OnProcessMessageRecieved(this, browser, source_process,
                                              message);
  }

  return handled;
}

void ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  REQUIRE_UI_THREAD();

  AutoLock lock_scope(this);
  if (!m_Browser.get())   {
    // We need to keep the main child window, but not popup windows
    m_Browser = browser;
    m_BrowserId = browser->GetIdentifier();
  }
}

bool ClientHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  REQUIRE_UI_THREAD();

  if (m_BrowserId == browser->GetIdentifier()) {
    // Since the main window contains the browser window, we need to close
    // the parent window instead of the browser window.
    CloseMainWindow();

    // Return true here so that we can skip closing the browser window
    // in this pass. (It will be destroyed due to the call to close
    // the parent above.)
    return true;
  }

  // A popup browser window is not contained in another window, so we can let
  // these windows close by themselves.
  return false;
}

void ClientHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  REQUIRE_UI_THREAD();

  if (m_BrowserId == browser->GetIdentifier()) {
    // Free the browser pointer so that the browser can be destroyed
    m_Browser = NULL;
  }
}

void ClientHandler::OnLoadStart(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame) {
  REQUIRE_UI_THREAD();

  if (m_BrowserId == browser->GetIdentifier() && frame->IsMain()) {
    // We've just started loading a page
    SetLoading(true);
  }
}

void ClientHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              int httpStatusCode) {
  REQUIRE_UI_THREAD();

  if (m_BrowserId == browser->GetIdentifier() && frame->IsMain()) {
    // We've just finished loading a page
    SetLoading(false);
  }
}

void ClientHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl) {
  REQUIRE_UI_THREAD();

  // Display a load error message.
  std::stringstream ss;
  ss << "<html><body><h2>Failed to load URL " << std::string(failedUrl) <<
        " with error " << std::string(errorText) << " (" << errorCode <<
        ").</h2></body></html>";
  frame->LoadString(ss.str(), failedUrl);
}

CefRefPtr<CefResourceHandler> ClientHandler::GetResourceHandler(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefRefPtr<CefRequest> request) {
  std::string url = request->GetURL();
  if (url == "http://tests/request") {
    // Show the request contents
    std::string dump;
    DumpRequestContents(request, dump);
    CefRefPtr<CefStreamReader> stream =
        CefStreamReader::CreateForData(
            static_cast<void*>(const_cast<char*>(dump.c_str())),
            dump.size());
    ASSERT(stream.get());
    return new CefStreamResourceHandler("text/plain", stream);
  } else if (url == "http://tests/dialogs") {
    // Show the dialogs contents
    CefRefPtr<CefStreamReader> stream =
        GetBinaryResourceReader("dialogs.html");
    ASSERT(stream.get());
    return new CefStreamResourceHandler("text/html", stream);
  } else if (url == "http://tests/localstorage") {
    // Show the localstorage contents
    CefRefPtr<CefStreamReader> stream =
        GetBinaryResourceReader("localstorage.html");
    ASSERT(stream.get());
    return new CefStreamResourceHandler("text/html", stream);
  } else if (url == "http://tests/xmlhttprequest") {
    // Show the xmlhttprequest contents
    CefRefPtr<CefStreamReader> stream =
       GetBinaryResourceReader("xmlhttprequest.html");
    ASSERT(stream.get());
    return new CefStreamResourceHandler("text/html", stream);
  }

  CefRefPtr<CefResourceHandler> handler;

  // Execute delegate callbacks.
  RequestDelegateSet::iterator it = request_delegates_.begin();
  for (; it != request_delegates_.end() && !handler.get(); ++it)
    handler = (*it)->GetResourceHandler(this, browser, frame, request);

  return handler;
}

void ClientHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
                                         bool isLoading,
                                         bool canGoBack,
                                         bool canGoForward) {
  REQUIRE_UI_THREAD();
  SetLoading(isLoading);
  SetNavState(canGoBack, canGoForward);
}

bool ClientHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser,
                                     const CefString& message,
                                     const CefString& source,
                                     int line) {
  REQUIRE_UI_THREAD();

  bool first_message;
  std::string logFile;

  {
    AutoLock lock_scope(this);

    first_message = m_LogFile.empty();
    if (first_message) {
      std::stringstream ss;
      ss << AppGetWorkingDirectory();
#if defined(OS_WIN)
      ss << "\\";
#else
      ss << "/";
#endif
      ss << "console.log";
      m_LogFile = ss.str();
    }
    logFile = m_LogFile;
  }

  FILE* file = fopen(logFile.c_str(), "a");
  if (file) {
    std::stringstream ss;
    ss << "Message: " << std::string(message) << "\r\nSource: " <<
        std::string(source) << "\r\nLine: " << line <<
        "\r\n-----------------------\r\n";
    fputs(ss.str().c_str(), file);
    fclose(file);

    if (first_message)
      SendNotification(NOTIFY_CONSOLE_MESSAGE);
  }

  return false;
}

void ClientHandler::OnRequestGeolocationPermission(
      CefRefPtr<CefBrowser> browser,
      const CefString& requesting_url,
      int request_id,
      CefRefPtr<CefGeolocationCallback> callback) {
  // Allow geolocation access from all websites.
  callback->Continue(true);
}

void ClientHandler::SetMainHwnd(CefWindowHandle hwnd) {
  AutoLock lock_scope(this);
  m_MainHwnd = hwnd;
}

void ClientHandler::SetEditHwnd(CefWindowHandle hwnd) {
  AutoLock lock_scope(this);
  m_EditHwnd = hwnd;
}

void ClientHandler::SetButtonHwnds(CefWindowHandle backHwnd,
                                   CefWindowHandle forwardHwnd,
                                   CefWindowHandle reloadHwnd,
                                   CefWindowHandle stopHwnd) {
  AutoLock lock_scope(this);
  m_BackHwnd = backHwnd;
  m_ForwardHwnd = forwardHwnd;
  m_ReloadHwnd = reloadHwnd;
  m_StopHwnd = stopHwnd;
}

std::string ClientHandler::GetLogFile() {
  AutoLock lock_scope(this);
  return m_LogFile;
}

void ClientHandler::SetLastDownloadFile(const std::string& fileName) {
  AutoLock lock_scope(this);
  m_LastDownloadFile = fileName;
}

std::string ClientHandler::GetLastDownloadFile() {
  AutoLock lock_scope(this);
  return m_LastDownloadFile;
}

void ClientHandler::ShowDevTools() {
}

void ClientHandler::CloseDevTools() {
}

// static
void ClientHandler::CreateProcessMessageDelegates(
      ProcessMessageDelegateSet& delegates) {
  // Create the binding test delegates.
  binding_test::CreateProcessMessageDelegates(delegates);
}

// static
void ClientHandler::CreateRequestDelegates(RequestDelegateSet& delegates) {
  // Create the binding test delegates.
  binding_test::CreateRequestDelegates(delegates);
}
