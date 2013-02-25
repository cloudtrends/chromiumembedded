// Copyright (c) 2012 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtk/gtk.h>

#include "libcef/cef_context.h"
#include "libcef/browser_impl.h"
#include "libcef/browser_settings.h"

#include "third_party/WebKit/Source/Platform/chromium/public/WebRect.h"
#include "third_party/WebKit/Source/Platform/chromium/public/WebSize.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebView.h"
#include "webkit/glue/webpreferences.h"

using WebKit::WebRect;
using WebKit::WebSize;

namespace {

void window_destroyed(GtkWidget* widget, CefBrowserImpl* browser) {
  browser->UIT_DestroyBrowser();
}

gboolean button_press_event(
    GtkWidget* widget,
    GdkEventButton* event,
    CefBrowserImpl* browser) {
  browser->set_last_mouse_down(event);
  return FALSE;
}

gboolean focus_out_event(
    GtkWidget* widget,
    GdkEventFocus* focus,
    CefBrowserImpl* browser) {
  browser->set_last_mouse_down(NULL);
  return FALSE;
}

}  // namespace

void CefBrowserImpl::ParentWindowWillClose() {
  // TODO(port): Implement this method if necessary.
}

CefWindowHandle CefBrowserImpl::GetWindowHandle() {
  AutoLock lock_scope(this);
  return window_info_.m_Widget;
}

bool CefBrowserImpl::IsWindowRenderingDisabled() {
  // TODO(port): Add support for off-screen rendering.
  return false;
}

gfx::NativeView CefBrowserImpl::UIT_GetMainWndHandle() {
  REQUIRE_UIT();
  return window_info_.m_Widget;
}

void CefBrowserImpl::UIT_ClearMainWndHandle() {
  REQUIRE_UIT();
  window_info_.m_Widget = NULL;
}

bool CefBrowserImpl::UIT_CreateBrowser(const CefString& url) {
  REQUIRE_UIT();
  Lock();

  // Add a reference that will be released in UIT_DestroyBrowser().
  AddRef();

  // Add the new browser to the list maintained by the context
  _Context->AddBrowser(this);

  if (!settings_.developer_tools_disabled)
    dev_tools_agent_.reset(new BrowserDevToolsAgent());

  GtkWidget* window;
  GtkWidget* parentView = window_info_.m_ParentWidget;

  if (parentView == NULL) {
    // Create a new window.
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    parentView = gtk_vbox_new(FALSE, 0);

    gtk_container_add(GTK_CONTAINER(window), parentView);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_widget_show_all(GTK_WIDGET(window));

    window_info_.m_ParentWidget = parentView;
  }

  webkit_glue::WebPreferences prefs;
  BrowserToWebSettings(settings_, prefs);

  // Create the webview host object
  webviewhost_.reset(
      WebViewHost::Create(window_info_.m_ParentWidget, gfx::Rect(),
                          delegate_.get(), NULL, dev_tools_agent_.get(),
                          prefs));

  if (!settings_.developer_tools_disabled)
    dev_tools_agent_->SetWebView(webviewhost_->webview());

  webviewhost_->SetFrameRate(settings_.animation_frame_rate);

  window_info_.m_Widget = webviewhost_->view_handle();
  g_signal_connect(G_OBJECT(window_info_.m_Widget), "destroy",
                   G_CALLBACK(window_destroyed), this);

  if (!settings_.drag_drop_disabled) {
    g_signal_connect(G_OBJECT(window_info_.m_Widget), "button-press-event",
                     G_CALLBACK(button_press_event), this);
    g_signal_connect(G_OBJECT(window_info_.m_Widget), "focus-out-event",
                     G_CALLBACK(focus_out_event), this);

    delegate_->RegisterDragDrop();
  }

  Unlock();

  if (client_.get()) {
    CefRefPtr<CefLifeSpanHandler> handler = client_->GetLifeSpanHandler();
    if (handler.get()) {
      // Notify the handler that we're done creating the new window
      handler->OnAfterCreated(this);
    }
  }

  if (url.size() > 0)
    UIT_LoadURL(GetMainFrame(), url);

  return true;
}

void CefBrowserImpl::UIT_SetFocus(WebWidgetHost* host, bool enable) {
  REQUIRE_UIT();
  if (!host)
    return;

  if (enable)
    gtk_widget_grab_focus(host->view_handle());
}

bool CefBrowserImpl::UIT_ViewDocumentString(WebKit::WebFrame *frame) {
  REQUIRE_UIT();

  char buff[] = "/tmp/CEFSourceXXXXXX";
  int fd = mkstemp(buff);

  if (fd == -1)
    return false;

  FILE* srcOutput;
  srcOutput = fdopen(fd, "w+");

  if (!srcOutput)
    return false;

  std::string markup = frame->contentAsMarkup().utf8();
  if (fputs(markup.c_str(), srcOutput) < 0) {
    fclose(srcOutput);
    return false;
  }

  fclose(srcOutput);
  std::string newName(buff);
  newName.append(".txt");
  if (rename(buff, newName.c_str()) != 0)
    return false;

  std::string openCommand("xdg-open ");
  openCommand += newName;

  if (system(openCommand.c_str()) != 0)
    return false;

  return true;
}

void CefBrowserImpl::UIT_PrintPage(int page_number, int total_pages,
                                   const gfx::Size& canvas_size,
                                   WebKit::WebFrame* frame) {
  REQUIRE_UIT();

  // TODO(port): Add implementation.
  NOTIMPLEMENTED();
}

void CefBrowserImpl::UIT_PrintPages(WebKit::WebFrame* frame) {
  REQUIRE_UIT();

  // TODO(port): Add implementation.
  NOTIMPLEMENTED();
}

int CefBrowserImpl::UIT_GetPagesCount(WebKit::WebFrame* frame) {
  REQUIRE_UIT();

  // TODO(port): Add implementation.
  NOTIMPLEMENTED();
  return 0;
}

// static
void CefBrowserImpl::UIT_CloseView(gfx::NativeView view) {
  GtkWidget* window = gtk_widget_get_toplevel(GTK_WIDGET(view));
  gtk_signal_emit_by_name(GTK_OBJECT(window), "delete_event");
}

// static
bool CefBrowserImpl::UIT_IsViewVisible(gfx::NativeView view) {
  if (!view)
    return false;

  if (view->window)
    return gdk_window_is_visible(view->window)?true:false;
  else
    return false;
}

void CefBrowserImpl::set_last_mouse_down(GdkEventButton* event) {
  GdkEventButton* temp = NULL;
  if (event) {
    temp = reinterpret_cast<GdkEventButton*>(
        gdk_event_copy(reinterpret_cast<GdkEvent*>(event)));
  }

  if (last_mouse_down_)
    gdk_event_free(reinterpret_cast<GdkEvent*>(last_mouse_down_));

  last_mouse_down_ = temp;
}
