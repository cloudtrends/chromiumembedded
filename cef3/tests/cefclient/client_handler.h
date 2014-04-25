// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_CLIENT_HANDLER_H_
#define CEF_TESTS_CEFCLIENT_CLIENT_HANDLER_H_
#pragma once

#include <list>
#include <map>
#include <set>
#include <string>
#include "include/cef_client.h"
#include "include/wrapper/cef_message_router.h"
#include "cefclient/util.h"


// Define this value to redirect all popup URLs to the main application browser
// window.
// #define TEST_REDIRECT_POPUP_URLS


// ClientHandler implementation.
class ClientHandler : public CefClient,
                      public CefContextMenuHandler,
                      public CefDisplayHandler,
                      public CefDownloadHandler,
                      public CefDragHandler,
                      public CefGeolocationHandler,
                      public CefKeyboardHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler,
                      public CefRequestHandler {
 public:
  typedef std::set<CefMessageRouterBrowserSide::Handler*> MessageHandlerSet;

  ClientHandler();
  virtual ~ClientHandler();

  // CefClient methods
  virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE {
    return this;
  }
  virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE {
    return this;
  }
  virtual CefRefPtr<CefDownloadHandler> GetDownloadHandler() OVERRIDE {
    return this;
  }
  virtual CefRefPtr<CefDragHandler> GetDragHandler() OVERRIDE {
    return this;
  }
  virtual CefRefPtr<CefGeolocationHandler> GetGeolocationHandler() OVERRIDE {
    return this;
  }
  virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() OVERRIDE {
    return this;
  }
  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE {
    return this;
  }
  virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE {
    return this;
  }
  virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE {
    return this;
  }
  virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                        CefProcessId source_process,
                                        CefRefPtr<CefProcessMessage> message)
                                        OVERRIDE;

  // CefContextMenuHandler methods
  virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefFrame> frame,
                                   CefRefPtr<CefContextMenuParams> params,
                                   CefRefPtr<CefMenuModel> model) OVERRIDE;
  virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefContextMenuParams> params,
                                    int command_id,
                                    EventFlags event_flags) OVERRIDE;

  // CefDisplayHandler methods
  virtual void OnAddressChange(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame,
                               const CefString& url) OVERRIDE;
  virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                             const CefString& title) OVERRIDE;
  virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
                                const CefString& message,
                                const CefString& source,
                                int line) OVERRIDE;

  // CefDownloadHandler methods
  virtual void OnBeforeDownload(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefDownloadItem> download_item,
      const CefString& suggested_name,
      CefRefPtr<CefBeforeDownloadCallback> callback) OVERRIDE;
  virtual void OnDownloadUpdated(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefDownloadItem> download_item,
      CefRefPtr<CefDownloadItemCallback> callback) OVERRIDE;

  // CefDragHandler methods
  virtual bool OnDragEnter(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefDragData> dragData,
                           DragOperationsMask mask) OVERRIDE;

  // CefGeolocationHandler methods
  virtual void OnRequestGeolocationPermission(
      CefRefPtr<CefBrowser> browser,
      const CefString& requesting_url,
      int request_id,
      CefRefPtr<CefGeolocationCallback> callback) OVERRIDE;

  // CefKeyboardHandler methods
  virtual bool OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
                             const CefKeyEvent& event,
                             CefEventHandle os_event,
                             bool* is_keyboard_shortcut) OVERRIDE;

  // CefLifeSpanHandler methods
  virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
  virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
  virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

  // CefLoadHandler methods
  virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
                                    bool isLoading,
                                    bool canGoBack,
                                    bool canGoForward) OVERRIDE;
  virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           ErrorCode errorCode,
                           const CefString& errorText,
                           const CefString& failedUrl) OVERRIDE;

  // CefRequestHandler methods
  virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              CefRefPtr<CefRequest> request,
                              bool is_redirect) OVERRIDE;
  virtual CefRefPtr<CefResourceHandler> GetResourceHandler(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefRefPtr<CefRequest> request) OVERRIDE;
  virtual bool OnQuotaRequest(CefRefPtr<CefBrowser> browser,
                              const CefString& origin_url,
                              int64 new_size,
                              CefRefPtr<CefQuotaCallback> callback) OVERRIDE;
  virtual void OnProtocolExecution(CefRefPtr<CefBrowser> browser,
                                   const CefString& url,
                                   bool& allow_os_execution) OVERRIDE;
  virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                         TerminationStatus status) OVERRIDE;

  void SetMainHwnd(CefWindowHandle hwnd);
  CefWindowHandle GetMainHwnd() { return m_MainHwnd; }
  void SetEditHwnd(CefWindowHandle hwnd);
  void SetButtonHwnds(CefWindowHandle backHwnd,
                      CefWindowHandle forwardHwnd,
                      CefWindowHandle reloadHwnd,
                      CefWindowHandle stopHwnd);

  CefRefPtr<CefBrowser> GetBrowser() { return m_Browser; }
  int GetBrowserId() { return m_BrowserId; }

  // Request that all existing browser windows close.
  void CloseAllBrowsers(bool force_close);

  // Returns true if the main browser window is currently closing. Used in
  // combination with DoClose() and the OS close notification to properly handle
  // 'onbeforeunload' JavaScript events during window close.
  bool IsClosing() { return m_bIsClosing; }

  std::string GetLogFile();

  void SetLastDownloadFile(const std::string& fileName);
  std::string GetLastDownloadFile();

  // Send a notification to the application. Notifications should not block the
  // caller.
  enum NotificationType {
    NOTIFY_CONSOLE_MESSAGE,
    NOTIFY_DOWNLOAD_COMPLETE,
    NOTIFY_DOWNLOAD_ERROR,
  };
  void SendNotification(NotificationType type);

  void ShowDevTools(CefRefPtr<CefBrowser> browser);
  void CloseDevTools(CefRefPtr<CefBrowser> browser);

  // Returns the startup URL.
  std::string GetStartupURL() { return m_StartupURL; }

  void BeginTracing();
  void EndTracing();

  bool Save(const std::string& path, const std::string& data);

 protected:
  void SetLoading(bool isLoading);
  void SetNavState(bool canGoBack, bool canGoForward);

  // Create all CefMessageRouterBrowserSide::Handler objects. They will be
  // deleted when the ClientHandler is destroyed.
  static void CreateMessageHandlers(MessageHandlerSet& handlers);

  // Test context menu creation.
  void BuildTestMenu(CefRefPtr<CefMenuModel> model);
  bool ExecuteTestMenu(int command_id);
  struct TestMenuState {
    TestMenuState() : check_item(true), radio_item(0) {}
    bool check_item;
    int radio_item;
  } m_TestMenuState;

  // Returns the full download path for the specified file, or an empty path to
  // use the default temp directory.
  std::string GetDownloadPath(const std::string& file_name);

  // The child browser window
  CefRefPtr<CefBrowser> m_Browser;

  // List of any popup browser windows. Only accessed on the CEF UI thread.
  typedef std::list<CefRefPtr<CefBrowser> > BrowserList;
  BrowserList m_PopupBrowsers;

  // The main frame window handle
  CefWindowHandle m_MainHwnd;

  // The child browser id
  int m_BrowserId;

  // True if the main browser window is currently closing.
  bool m_bIsClosing;

  // The edit window handle
  CefWindowHandle m_EditHwnd;

  // The button window handles
  CefWindowHandle m_BackHwnd;
  CefWindowHandle m_ForwardHwnd;
  CefWindowHandle m_StopHwnd;
  CefWindowHandle m_ReloadHwnd;

  // Support for logging.
  std::string m_LogFile;

  // Support for downloading files.
  std::string m_LastDownloadFile;

  // True if an editable field currently has focus.
  bool m_bFocusOnEditableField;

  // The startup URL.
  std::string m_StartupURL;

  // True if mouse cursor change is disabled.
  bool m_bMouseCursorChangeDisabled;

  // Handles the browser side of query routing. The renderer side is handled
  // in client_renderer.cpp.
  CefRefPtr<CefMessageRouterBrowserSide> message_router_;

  // Set of Handlers registered with the message router.
  MessageHandlerSet message_handler_set_;

  // Number of currently existing browser windows. The application will exit
  // when the number of windows reaches 0.
  static int m_BrowserCount;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(ClientHandler);
  // Include the default locking implementation.
  IMPLEMENT_LOCKING(ClientHandler);
};

#endif  // CEF_TESTS_CEFCLIENT_CLIENT_HANDLER_H_
