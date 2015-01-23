// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>

#include <X11/Xlib.h>
#undef Status   // Definition conflicts with cef_urlrequest.h
#undef Success  // Definition conflicts with cef_message_router.h

#include <stdlib.h>
#include <unistd.h>
#include <string>

#include "include/base/cef_logging.h"
#include "include/base/cef_scoped_ptr.h"
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/wrapper/cef_helpers.h"
#include "cefclient/client_app.h"
#include "cefclient/client_handler.h"
#include "cefclient/client_switches.h"
#include "cefclient/main_context_impl.h"
#include "cefclient/main_message_loop_std.h"
#include "cefclient/osr_widget_gtk.h"
#include "cefclient/resource.h"
#include "cefclient/test_runner.h"

namespace client {
namespace {

// The global ClientHandler reference.
CefRefPtr<ClientHandler> g_handler;

// Height of the buttons at the top of the GTK window.
int g_toolbar_height = 0;

// Height of the integrated menu bar (if any) at the top of the GTK window.
int g_menubar_height = 0;


// Used by off-screen rendering to find the associated CefBrowser.
class MainBrowserProvider : public OSRBrowserProvider {
  virtual CefRefPtr<CefBrowser> GetBrowser() {
    if (g_handler.get())
      return g_handler->GetBrowser();

    return NULL;
  }
} g_main_browser_provider;


int XErrorHandlerImpl(Display *display, XErrorEvent *event) {
  LOG(WARNING)
        << "X error received: "
        << "type " << event->type << ", "
        << "serial " << event->serial << ", "
        << "error_code " << static_cast<int>(event->error_code) << ", "
        << "request_code " << static_cast<int>(event->request_code) << ", "
        << "minor_code " << static_cast<int>(event->minor_code);
  return 0;
}

int XIOErrorHandlerImpl(Display *display) {
  return 0;
}

void destroy(GtkWidget* widget, gpointer data) {
  // Quitting CEF is handled in ClientHandler::OnBeforeClose().
}

gboolean delete_event(GtkWidget* widget, GdkEvent* event,
                      GtkWindow* window) {
  if (g_handler.get() && !g_handler->IsClosing()) {
    CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
    if (browser.get()) {
      // Notify the browser window that we would like to close it. This
      // will result in a call to ClientHandler::DoClose() if the
      // JavaScript 'onbeforeunload' event handler allows it.
      browser->GetHost()->CloseBrowser(false);

      // Cancel the close.
      return TRUE;
    }
  }

  // Allow the close.
  return FALSE;
}

void TerminationSignalHandler(int signatl) {
  MainMessageLoop::Get()->Quit();
}

void VboxSizeAllocated(GtkWidget* widget,
                       GtkAllocation* allocation,
                       void* data) {
  if (g_handler) {
    CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
    if (browser && !browser->GetHost()->IsWindowRenderingDisabled()) {
      // Size the browser window to match the GTK widget.
      ::Display* xdisplay = cef_get_xdisplay();
      ::Window xwindow = browser->GetHost()->GetWindowHandle();
      XWindowChanges changes = {0};
      changes.width = allocation->width;
      changes.height = allocation->height -
                       (g_toolbar_height + g_menubar_height);
      changes.y = g_toolbar_height + g_menubar_height;
      XConfigureWindow(xdisplay, xwindow, CWHeight | CWWidth | CWY, &changes);
    }
  }
}

void ToolbarSizeAllocated(GtkWidget* widget,
                          GtkAllocation* allocation,
                          void* data) {
  g_toolbar_height = allocation->height;
}

void MenubarSizeAllocated(GtkWidget* widget,
                          GtkAllocation* allocation,
                          void* data) {
  g_menubar_height = allocation->height;
}

gboolean WindowFocusIn(GtkWidget* widget,
                       GdkEventFocus* event,
                       gpointer user_data) {
  if (g_handler && event->in) {
    CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
    if (browser) {
      if (browser->GetHost()->IsWindowRenderingDisabled()) {
        // Give focus to the off-screen browser.
        browser->GetHost()->SendFocusEvent(true);
      } else {
        // Give focus to the browser window.
        browser->GetHost()->SetFocus(true);
        return TRUE;
      }
    }
  }

  return FALSE;
}

gboolean WindowState(GtkWidget* widget,
                     GdkEventWindowState* event,
                     gpointer user_data) {
  if (!(event->changed_mask & GDK_WINDOW_STATE_ICONIFIED))
    return TRUE;

  if (!g_handler)
    return TRUE;
  CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
  if (!browser)
    return TRUE;

  const bool iconified = (event->new_window_state & GDK_WINDOW_STATE_ICONIFIED);
  if (browser->GetHost()->IsWindowRenderingDisabled()) {
    // Notify the off-screen browser that it was shown or hidden.
    browser->GetHost()->WasHidden(iconified);
  } else {
    // Forward the state change event to the browser window.
    ::Display* xdisplay = cef_get_xdisplay();
    ::Window xwindow = browser->GetHost()->GetWindowHandle();

    // Retrieve the atoms required by the below XChangeProperty call.
    const char* kAtoms[] = {
      "_NET_WM_STATE",
      "ATOM",
      "_NET_WM_STATE_HIDDEN"
    };
    Atom atoms[3];
    int result = XInternAtoms(xdisplay, const_cast<char**>(kAtoms), 3, false,
                              atoms);
    if (!result)
      NOTREACHED();

    if (iconified) {
      // Set the hidden property state value.
      scoped_ptr<Atom[]> data(new Atom[1]);
      data[0] = atoms[2];

      XChangeProperty(xdisplay,
                      xwindow,
                      atoms[0],  // name
                      atoms[1],  // type
                      32,  // size in bits of items in 'value'
                      PropModeReplace,
                      reinterpret_cast<const unsigned char*>(data.get()),
                      1);  // num items
    } else {
      // Set an empty array of property state values.
      XChangeProperty(xdisplay,
                      xwindow,
                      atoms[0],  // name
                      atoms[1],  // type
                      32,  // size in bits of items in 'value'
                      PropModeReplace,
                      NULL,
                      0);  // num items
    }
  }

  return TRUE;
}

gboolean WindowConfigure(GtkWindow* window, 
                         GdkEvent* event,
                         gpointer data) {
  // Called when size, position or stack order changes.
  if (g_handler) {
    CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
    if (browser) {
      // Notify the browser of move/resize events so that:
      // - Popup windows are displayed in the correct location and dismissed
      //   when the window moves.
      // - Drag&drop areas are updated accordingly.
      browser->GetHost()->NotifyMoveOrResizeStarted();
    }
  }

  return FALSE;  // Don't stop this message.
}

// Callback for Tests menu items.
gboolean MenuItemActivated(GtkWidget* widget, gpointer data) {
  if (g_handler.get() && g_handler->GetBrowserId())
    test_runner::RunTest(g_handler->GetBrowser(), GPOINTER_TO_INT(data));

  return FALSE;  // Don't stop this message.
}

// Callback for when you click the back button.
void BackButtonClicked(GtkButton* button) {
  if (g_handler.get() && g_handler->GetBrowserId())
    g_handler->GetBrowser()->GoBack();
}

// Callback for when you click the forward button.
void ForwardButtonClicked(GtkButton* button) {
  if (g_handler.get() && g_handler->GetBrowserId())
    g_handler->GetBrowser()->GoForward();
}

// Callback for when you click the stop button.
void StopButtonClicked(GtkButton* button) {
  if (g_handler.get() && g_handler->GetBrowserId())
    g_handler->GetBrowser()->StopLoad();
}

// Callback for when you click the reload button.
void ReloadButtonClicked(GtkButton* button) {
  if (g_handler.get() && g_handler->GetBrowserId())
    g_handler->GetBrowser()->Reload();
}

// Callback for when you press enter in the URL box.
void URLEntryActivate(GtkEntry* entry) {
  if (!g_handler.get() || !g_handler->GetBrowserId())
    return;

  const gchar* url = gtk_entry_get_text(entry);
  g_handler->GetBrowser()->GetMainFrame()->LoadURL(std::string(url).c_str());
}

gboolean URLEntryButtonPress(GtkWidget* widget,
                             GdkEventButton* event,
                             gpointer user_data) {
  // Give focus to the GTK window.
  GtkWidget* window = gtk_widget_get_ancestor(widget,
      GTK_TYPE_WINDOW);
  GdkWindow* gdk_window = gtk_widget_get_window(window);
  ::Display* xdisplay = GDK_WINDOW_XDISPLAY(gdk_window);
  ::Window xwindow = GDK_WINDOW_XID(gdk_window);
  XSetInputFocus(xdisplay, xwindow, RevertToParent, CurrentTime);

  return FALSE;
}

// GTK utility functions ----------------------------------------------

GtkWidget* AddMenuEntry(GtkWidget* menu_widget, const char* text, int id) {
  GtkWidget* entry = gtk_menu_item_new_with_label(text);
  g_signal_connect(entry, "activate", G_CALLBACK(MenuItemActivated),
      GINT_TO_POINTER(id));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_widget), entry);
  return entry;
}

GtkWidget* CreateMenu(GtkWidget* menu_bar, const char* text) {
  GtkWidget* menu_widget = gtk_menu_new();
  GtkWidget* menu_header = gtk_menu_item_new_with_label(text);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_header), menu_widget);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_header);
  return menu_widget;
}

GtkWidget* CreateMenuBar() {
  GtkWidget* menu_bar = gtk_menu_bar_new();
  GtkWidget* debug_menu = CreateMenu(menu_bar, "Tests");

  AddMenuEntry(debug_menu, "Get Source",    ID_TESTS_GETSOURCE);
  AddMenuEntry(debug_menu, "Get Text",      ID_TESTS_GETTEXT);
  AddMenuEntry(debug_menu, "Popup Window",  ID_TESTS_POPUP);
  AddMenuEntry(debug_menu, "Request",       ID_TESTS_REQUEST);
  AddMenuEntry(debug_menu, "Plugin Info",   ID_TESTS_PLUGIN_INFO);
  AddMenuEntry(debug_menu, "Zoom In",       ID_TESTS_ZOOM_IN);
  AddMenuEntry(debug_menu, "Zoom Out",      ID_TESTS_ZOOM_OUT);
  AddMenuEntry(debug_menu, "Zoom Reset",    ID_TESTS_ZOOM_RESET);
  AddMenuEntry(debug_menu, "Begin Tracing", ID_TESTS_TRACING_BEGIN);
  AddMenuEntry(debug_menu, "End Tracing",   ID_TESTS_TRACING_END);
  AddMenuEntry(debug_menu, "Print",         ID_TESTS_PRINT);
  AddMenuEntry(debug_menu, "Other Tests",   ID_TESTS_OTHER_TESTS);
  return menu_bar;
}

int RunMain(int argc, char* argv[]) {
  // Create a copy of |argv| on Linux because Chromium mangles the value
  // internally (see issue #620).
  CefScopedArgArray scoped_arg_array(argc, argv);
  char** argv_copy = scoped_arg_array.array();

  CefMainArgs main_args(argc, argv);
  CefRefPtr<ClientApp> app(new ClientApp);

  // Execute the secondary process, if any.
  int exit_code = CefExecuteProcess(main_args, app.get(), NULL);
  if (exit_code >= 0)
    return exit_code;

  // Create the main context object.
  scoped_ptr<MainContextImpl> context(new MainContextImpl(argc, argv));

  CefSettings settings;

  // Populate the settings based on command line arguments.
  context->PopulateSettings(&settings);

  // Install xlib error handlers so that the application won't be terminated
  // on non-fatal errors.
  XSetErrorHandler(XErrorHandlerImpl);
  XSetIOErrorHandler(XIOErrorHandlerImpl);

  // Create the main message loop object.
  scoped_ptr<MainMessageLoop> message_loop(new MainMessageLoopStd);

  // Initialize CEF.
  CefInitialize(main_args, settings, app.get(), NULL);

  // The Chromium sandbox requires that there only be a single thread during
  // initialization. Therefore initialize GTK after CEF.
  gtk_init(&argc, &argv_copy);

  // Perform gtkglext initialization required by the OSR example.
  gtk_gl_init(&argc, &argv_copy);

  // Register scheme handlers.
  test_runner::RegisterSchemeHandlers();

  GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
  g_signal_connect(window, "focus-in-event",
                   G_CALLBACK(WindowFocusIn), NULL);
  g_signal_connect(window, "window-state-event", 
                   G_CALLBACK(WindowState), NULL);
  g_signal_connect(G_OBJECT(window), "configure-event",
                   G_CALLBACK(WindowConfigure), NULL);

  GtkWidget* vbox = gtk_vbox_new(FALSE, 0);
  g_signal_connect(vbox, "size-allocate",
                   G_CALLBACK(VboxSizeAllocated), NULL);
  gtk_container_add(GTK_CONTAINER(window), vbox);

  GtkWidget* menu_bar = CreateMenuBar();
  g_signal_connect(menu_bar, "size-allocate",
                   G_CALLBACK(MenubarSizeAllocated), NULL);

  gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);

  GtkWidget* toolbar = gtk_toolbar_new();
  // Turn off the labels on the toolbar buttons.
  gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
  g_signal_connect(toolbar, "size-allocate",
                   G_CALLBACK(ToolbarSizeAllocated), NULL);

  GtkToolItem* back = gtk_tool_button_new_from_stock(GTK_STOCK_GO_BACK);
  g_signal_connect(back, "clicked",
                   G_CALLBACK(BackButtonClicked), NULL);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), back, -1 /* append */);

  GtkToolItem* forward = gtk_tool_button_new_from_stock(GTK_STOCK_GO_FORWARD);
  g_signal_connect(forward, "clicked",
                   G_CALLBACK(ForwardButtonClicked), NULL);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), forward, -1 /* append */);

  GtkToolItem* reload = gtk_tool_button_new_from_stock(GTK_STOCK_REFRESH);
  g_signal_connect(reload, "clicked",
                   G_CALLBACK(ReloadButtonClicked), NULL);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), reload, -1 /* append */);

  GtkToolItem* stop = gtk_tool_button_new_from_stock(GTK_STOCK_STOP);
  g_signal_connect(stop, "clicked",
                   G_CALLBACK(StopButtonClicked), NULL);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), stop, -1 /* append */);

  GtkWidget* entry = gtk_entry_new();
  g_signal_connect(entry, "activate",
                   G_CALLBACK(URLEntryActivate), NULL);
  g_signal_connect(entry, "button-press-event",
                   G_CALLBACK(URLEntryButtonPress), NULL);

  GtkToolItem* tool_item = gtk_tool_item_new();
  gtk_container_add(GTK_CONTAINER(tool_item), entry);
  gtk_tool_item_set_expand(tool_item, TRUE);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tool_item, -1);  // append

  gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);

  g_signal_connect(G_OBJECT(window), "destroy",
                   G_CALLBACK(destroy), NULL);
  g_signal_connect(G_OBJECT(window), "delete_event",
                   G_CALLBACK(delete_event), window);

  // Create the handler.
  g_handler = new ClientHandler();
  g_handler->SetMainWindowHandle(vbox);
  g_handler->SetEditWindowHandle(entry);
  g_handler->SetButtonWindowHandles(GTK_WIDGET(back), GTK_WIDGET(forward),
                                    GTK_WIDGET(reload), GTK_WIDGET(stop));

  CefWindowInfo window_info;
  CefBrowserSettings browserSettings;

  // Populate the browser settings based on command line arguments.
  context->PopulateBrowserSettings(&browserSettings);

  CefRefPtr<CefCommandLine> command_line =
      CefCommandLine::GetGlobalCommandLine();
  if (command_line->HasSwitch(switches::kOffScreenRenderingEnabled)) {
    const bool transparent =
        command_line->HasSwitch(switches::kTransparentPaintingEnabled);
    const bool show_update_rect =
        command_line->HasSwitch(switches::kShowUpdateRect);

    // Create the GTKGL surface.
    CefRefPtr<OSRWindow> osr_window =
        OSRWindow::Create(&g_main_browser_provider, transparent,
                          show_update_rect, vbox);

    // Show the GTK window.
    gtk_widget_show_all(GTK_WIDGET(window));

    // The GTK window must be visible before we can retrieve the XID.
    ::Window xwindow =
        GDK_WINDOW_XID(gtk_widget_get_window(osr_window->GetWindowHandle()));
    window_info.SetAsWindowless(xwindow, transparent);
    g_handler->SetOSRHandler(osr_window.get());
  } else {
    // Show the GTK window.
    gtk_widget_show_all(GTK_WIDGET(window));

    // The GTK window must be visible before we can retrieve the XID.
    ::Window xwindow = GDK_WINDOW_XID(gtk_widget_get_window(window));
    window_info.SetAsChild(xwindow,
        CefRect(0, g_toolbar_height, 800, 600 - g_toolbar_height));
  }

  // Create the browser window.
  CefBrowserHost::CreateBrowserSync(
      window_info, g_handler.get(),
      g_handler->GetStartupURL(), browserSettings, NULL);

  // Install a signal handler so we clean up after ourselves.
  signal(SIGINT, TerminationSignalHandler);
  signal(SIGTERM, TerminationSignalHandler);

  // Run the message loop. This will block until Quit() is called.
  int result = message_loop->Run();

  // Shut down CEF.
  CefShutdown();

  // Release objects in reverse order of creation.
  message_loop.reset();
  context.reset();

  return result;
}

}  // namespace
}  // namespace client


// Program entry point function.
int main(int argc, char* argv[]) {
  return client::RunMain(argc, argv);
}
