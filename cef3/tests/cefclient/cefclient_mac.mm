// Copyright (c) 2013 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#include <sstream>
#include "cefclient/cefclient.h"
#include "include/cef_app.h"
#import "include/cef_application_mac.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_runnable.h"
#include "cefclient/cefclient_osr_widget_mac.h"
#include "cefclient/client_handler.h"
#include "cefclient/client_switches.h"
#include "cefclient/resource_util.h"
#include "cefclient/scheme_test.h"
#include "cefclient/string_util.h"

// The global ClientHandler reference.
extern CefRefPtr<ClientHandler> g_handler;

class MainBrowserProvider : public OSRBrowserProvider {
  virtual CefRefPtr<CefBrowser> GetBrowser() {
    if (g_handler.get())
      return g_handler->GetBrowser();

    return NULL;
  }
} g_main_browser_provider;

char szWorkingDir[512];   // The current working directory

// Sizes for URL bar layout
#define BUTTON_HEIGHT 22
#define BUTTON_WIDTH 72
#define BUTTON_MARGIN 8
#define URLBAR_HEIGHT  32

// Content area size for newly created windows.
const int kWindowWidth = 800;
const int kWindowHeight = 600;

// Provide the CefAppProtocol implementation required by CEF.
@interface ClientApplication : NSApplication<CefAppProtocol> {
@private
  BOOL handlingSendEvent_;
}
@end

@implementation ClientApplication
- (BOOL)isHandlingSendEvent {
  return handlingSendEvent_;
}

- (void)setHandlingSendEvent:(BOOL)handlingSendEvent {
  handlingSendEvent_ = handlingSendEvent;
}

- (void)sendEvent:(NSEvent*)event {
  CefScopedSendingEvent sendingEventScoper;
  [super sendEvent:event];
}
@end


// Receives notifications from controls and the browser window. Will delete
// itself when done.
@interface ClientWindowDelegate : NSObject <NSWindowDelegate>
- (IBAction)goBack:(id)sender;
- (IBAction)goForward:(id)sender;
- (IBAction)reload:(id)sender;
- (IBAction)stopLoading:(id)sender;
- (IBAction)takeURLStringValueFrom:(NSTextField *)sender;
- (void)alert:(NSString*)title withMessage:(NSString*)message;
- (void)notifyConsoleMessage:(id)object;
- (void)notifyDownloadComplete:(id)object;
- (void)notifyDownloadError:(id)object;
@end

@implementation ClientWindowDelegate

- (IBAction)goBack:(id)sender {
  if (g_handler.get() && g_handler->GetBrowserId())
    g_handler->GetBrowser()->GoBack();
}

- (IBAction)goForward:(id)sender {
  if (g_handler.get() && g_handler->GetBrowserId())
    g_handler->GetBrowser()->GoForward();
}

- (IBAction)reload:(id)sender {
  if (g_handler.get() && g_handler->GetBrowserId())
    g_handler->GetBrowser()->Reload();
}

- (IBAction)stopLoading:(id)sender {
  if (g_handler.get() && g_handler->GetBrowserId())
    g_handler->GetBrowser()->StopLoad();
}

- (IBAction)takeURLStringValueFrom:(NSTextField *)sender {
  if (!g_handler.get() || !g_handler->GetBrowserId())
    return;

  NSString *url = [sender stringValue];

  // if it doesn't already have a prefix, add http. If we can't parse it,
  // just don't bother rather than making things worse.
  NSURL* tempUrl = [NSURL URLWithString:url];
  if (tempUrl && ![tempUrl scheme])
    url = [@"http://" stringByAppendingString:url];

  std::string urlStr = [url UTF8String];
  g_handler->GetBrowser()->GetMainFrame()->LoadURL(urlStr);
}

- (void)alert:(NSString*)title withMessage:(NSString*)message {
  NSAlert *alert = [NSAlert alertWithMessageText:title
                                   defaultButton:@"OK"
                                 alternateButton:nil
                                     otherButton:nil
                       informativeTextWithFormat:message];
  [alert runModal];
}

- (void)notifyConsoleMessage:(id)object {
  std::stringstream ss;
  ss << "Console messages will be written to " << g_handler->GetLogFile();
  NSString* str = [NSString stringWithUTF8String:(ss.str().c_str())];
  [self alert:@"Console Messages" withMessage:str];
}

- (void)notifyDownloadComplete:(id)object {
  std::stringstream ss;
  ss << "File \"" << g_handler->GetLastDownloadFile() <<
      "\" downloaded successfully.";
  NSString* str = [NSString stringWithUTF8String:(ss.str().c_str())];
  [self alert:@"File Download" withMessage:str];
}

- (void)notifyDownloadError:(id)object {
  std::stringstream ss;
  ss << "File \"" << g_handler->GetLastDownloadFile() <<
      "\" failed to download.";
  NSString* str = [NSString stringWithUTF8String:(ss.str().c_str())];
  [self alert:@"File Download" withMessage:str];
}

- (void)windowDidBecomeKey:(NSNotification*)notification {
  if (g_handler.get() && g_handler->GetBrowserId()) {
    // Give focus to the browser window.
    g_handler->GetBrowser()->GetHost()->SetFocus(true);
  }
}

// Called when the window is about to close. Perform the self-destruction
// sequence by getting rid of the window. By returning YES, we allow the window
// to be removed from the screen.
- (BOOL)windowShouldClose:(id)window {
  if (g_handler.get() && !g_handler->IsClosing()) {
    CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
    if (browser.get()) {
      // Notify the browser window that we would like to close it. This
      // will result in a call to ClientHandler::DoClose() if the
      // JavaScript 'onbeforeunload' event handler allows it.
      browser->GetHost()->CloseBrowser(false);

      // Cancel the close.
      return NO;
    }
  }

  // Try to make the window go away.
  [window autorelease];

  // Clean ourselves up after clearing the stack of anything that might have the
  // window on it.
  [self performSelectorOnMainThread:@selector(cleanup:)
                         withObject:window
                      waitUntilDone:NO];

  // Allow the close.
  return YES;
}

// Deletes itself.
- (void)cleanup:(id)window {
  [self release];
}

@end


NSButton* MakeButton(NSRect* rect, NSString* title, NSView* parent) {
  NSButton* button = [[[NSButton alloc] initWithFrame:*rect] autorelease];
  [button setTitle:title];
  [button setBezelStyle:NSSmallSquareBezelStyle];
  [button setAutoresizingMask:(NSViewMaxXMargin | NSViewMinYMargin)];
  [parent addSubview:button];
  rect->origin.x += BUTTON_WIDTH;
  return button;
}

// Receives notifications from the application. Will delete itself when done.
@interface ClientAppDelegate : NSObject
- (void)createApp:(id)object;
- (IBAction)testGetSource:(id)sender;
- (IBAction)testGetText:(id)sender;
- (IBAction)testPopupWindow:(id)sender;
- (IBAction)testRequest:(id)sender;
- (IBAction)testPluginInfo:(id)sender;
- (IBAction)testZoomIn:(id)sender;
- (IBAction)testZoomOut:(id)sender;
- (IBAction)testZoomReset:(id)sender;
- (IBAction)testBeginTracing:(id)sender;
- (IBAction)testEndTracing:(id)sender;
- (IBAction)testOtherTests:(id)sender;
@end

@implementation ClientAppDelegate

// Create the application on the UI thread.
- (void)createApp:(id)object {
  [NSApplication sharedApplication];
  [NSBundle loadNibNamed:@"MainMenu" owner:NSApp];

  // Set the delegate for application events.
  [NSApp setDelegate:self];

  // Add the Tests menu.
  NSMenu* menubar = [NSApp mainMenu];
  NSMenuItem *testItem = [[[NSMenuItem alloc] initWithTitle:@"Tests"
                                                     action:nil
                                              keyEquivalent:@""] autorelease];
  NSMenu *testMenu = [[[NSMenu alloc] initWithTitle:@"Tests"] autorelease];
  [testMenu addItemWithTitle:@"Get Source"
                      action:@selector(testGetSource:)
               keyEquivalent:@""];
  [testMenu addItemWithTitle:@"Get Text"
                      action:@selector(testGetText:)
               keyEquivalent:@""];
  [testMenu addItemWithTitle:@"Popup Window"
                      action:@selector(testPopupWindow:)
               keyEquivalent:@""];
  [testMenu addItemWithTitle:@"Request"
                      action:@selector(testRequest:)
               keyEquivalent:@""];
  [testMenu addItemWithTitle:@"Plugin Info"
                      action:@selector(testPluginInfo:)
               keyEquivalent:@""];
  [testMenu addItemWithTitle:@"Zoom In"
                      action:@selector(testZoomIn:)
               keyEquivalent:@""];
  [testMenu addItemWithTitle:@"Zoom Out"
                      action:@selector(testZoomOut:)
               keyEquivalent:@""];
  [testMenu addItemWithTitle:@"Zoom Reset"
                      action:@selector(testZoomReset:)
               keyEquivalent:@""];
  [testMenu addItemWithTitle:@"Begin Tracing"
                      action:@selector(testBeginTracing:)
               keyEquivalent:@""];
  [testMenu addItemWithTitle:@"End Tracing"
                      action:@selector(testEndTracing:)
               keyEquivalent:@""];
  [testMenu addItemWithTitle:@"Other Tests"
                      action:@selector(testOtherTests:)
               keyEquivalent:@""];
  [testItem setSubmenu:testMenu];
  [menubar addItem:testItem];

  // Create the delegate for control and browser window events.
  ClientWindowDelegate* delegate = [[ClientWindowDelegate alloc] init];

  // Create the main application window.
  NSRect screen_rect = [[NSScreen mainScreen] visibleFrame];
  NSRect window_rect = { {0, screen_rect.size.height - kWindowHeight},
    {kWindowWidth, kWindowHeight} };
  NSWindow* mainWnd = [[UnderlayOpenGLHostingWindow alloc]
                       initWithContentRect:window_rect
                       styleMask:(NSTitledWindowMask |
                                  NSClosableWindowMask |
                                  NSMiniaturizableWindowMask |
                                  NSResizableWindowMask )
                       backing:NSBackingStoreBuffered
                       defer:NO];
  [mainWnd setTitle:@"cefclient"];
  [mainWnd setDelegate:delegate];

  // Rely on the window delegate to clean us up rather than immediately
  // releasing when the window gets closed. We use the delegate to do
  // everything from the autorelease pool so the window isn't on the stack
  // during cleanup (ie, a window close from javascript).
  [mainWnd setReleasedWhenClosed:NO];

  NSView* contentView = [mainWnd contentView];

  // Create the buttons.
  NSRect button_rect = [contentView bounds];
  button_rect.origin.y = window_rect.size.height - URLBAR_HEIGHT +
      (URLBAR_HEIGHT - BUTTON_HEIGHT) / 2;
  button_rect.size.height = BUTTON_HEIGHT;
  button_rect.origin.x += BUTTON_MARGIN;
  button_rect.size.width = BUTTON_WIDTH;

  NSButton* button = MakeButton(&button_rect, @"Back", contentView);
  [button setTarget:delegate];
  [button setAction:@selector(goBack:)];

  button = MakeButton(&button_rect, @"Forward", contentView);
  [button setTarget:delegate];
  [button setAction:@selector(goForward:)];

  button = MakeButton(&button_rect, @"Reload", contentView);
  [button setTarget:delegate];
  [button setAction:@selector(reload:)];

  button = MakeButton(&button_rect, @"Stop", contentView);
  [button setTarget:delegate];
  [button setAction:@selector(stopLoading:)];

  // Create the URL text field.
  button_rect.origin.x += BUTTON_MARGIN;
  button_rect.size.width = [contentView bounds].size.width -
      button_rect.origin.x - BUTTON_MARGIN;
  NSTextField* editWnd = [[NSTextField alloc] initWithFrame:button_rect];
  [contentView addSubview:editWnd];
  [editWnd setAutoresizingMask:(NSViewWidthSizable | NSViewMinYMargin)];
  [editWnd setTarget:delegate];
  [editWnd setAction:@selector(takeURLStringValueFrom:)];
  [[editWnd cell] setWraps:NO];
  [[editWnd cell] setScrollable:YES];

  // Create the handler.
  g_handler = new ClientHandler();
  g_handler->SetMainHwnd(contentView);
  g_handler->SetEditHwnd(editWnd);

  // Create the browser view.
  CefWindowInfo window_info;
  CefBrowserSettings settings;

  if (AppIsOffScreenRenderingEnabled()) {
    CefRefPtr<CefCommandLine> cmd_line = AppGetCommandLine();
    bool transparent =
        cmd_line->HasSwitch(cefclient::kTransparentPaintingEnabled);

    CefRefPtr<OSRWindow> osr_window =
        OSRWindow::Create(&g_main_browser_provider, transparent, contentView,
            CefRect(0, 0, kWindowWidth, kWindowHeight));
    window_info.SetAsOffScreen(osr_window->GetWindowHandle());
    window_info.SetTransparentPainting(transparent);
    g_handler->SetOSRHandler(osr_window->GetRenderHandler().get());
  } else {
    // Initialize window info to the defaults for a child window.
    window_info.SetAsChild(contentView, 0, 0, kWindowWidth, kWindowHeight);
  }

  CefBrowserHost::CreateBrowser(window_info, g_handler.get(),
                                g_handler->GetStartupURL(), settings);

  // Show the window.
  [mainWnd makeKeyAndOrderFront: nil];

  // Size the window.
  NSRect r = [mainWnd contentRectForFrameRect:[mainWnd frame]];
  r.size.width = kWindowWidth;
  r.size.height = kWindowHeight + URLBAR_HEIGHT;
  [mainWnd setFrame:[mainWnd frameRectForContentRect:r] display:YES];
}

- (IBAction)testGetSource:(id)sender {
  if (g_handler.get() && g_handler->GetBrowserId())
    RunGetSourceTest(g_handler->GetBrowser());
}

- (IBAction)testGetText:(id)sender {
  if (g_handler.get() && g_handler->GetBrowserId())
    RunGetTextTest(g_handler->GetBrowser());
}

- (IBAction)testPopupWindow:(id)sender {
  if (g_handler.get() && g_handler->GetBrowserId())
    RunPopupTest(g_handler->GetBrowser());
}

- (IBAction)testRequest:(id)sender {
  if (g_handler.get() && g_handler->GetBrowserId())
    RunRequestTest(g_handler->GetBrowser());
}

- (IBAction)testPluginInfo:(id)sender {
  if (g_handler.get() && g_handler->GetBrowserId())
    RunPluginInfoTest(g_handler->GetBrowser());
}

- (IBAction)testZoomIn:(id)sender {
  if (g_handler.get() && g_handler->GetBrowserId()) {
    CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
    browser->GetHost()->SetZoomLevel(browser->GetHost()->GetZoomLevel() + 0.5);
  }
}

- (IBAction)testZoomOut:(id)sender {
  if (g_handler.get() && g_handler->GetBrowserId()) {
    CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
    browser->GetHost()->SetZoomLevel(browser->GetHost()->GetZoomLevel() - 0.5);
  }
}

- (IBAction)testZoomReset:(id)sender {
  if (g_handler.get() && g_handler->GetBrowserId()) {
    CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
    browser->GetHost()->SetZoomLevel(0.0);
  }
}

- (IBAction)testBeginTracing:(id)sender {
  if (g_handler.get())
    g_handler->BeginTracing();
}

- (IBAction)testEndTracing:(id)sender {
  if (g_handler.get())
    g_handler->EndTracing();
}

- (IBAction)testOtherTests:(id)sender {
  if (g_handler.get() && g_handler->GetBrowserId())
    RunOtherTests(g_handler->GetBrowser());
}

// Called when the application's Quit menu item is selected.
- (NSApplicationTerminateReply)applicationShouldTerminate:
      (NSApplication *)sender {
  // Request that all browser windows close.
  if (g_handler.get())
    g_handler->CloseAllBrowsers(false);

  // Cancel the termination. The application will exit after all windows have
  // closed.
  return NSTerminateCancel;
}

// Sent immediately before the application terminates. This signal should not
// be called because we cancel the termination.
- (void)applicationWillTerminate:(NSNotification *)aNotification {
  ASSERT(false);  // Not reached.
}

@end


int main(int argc, char* argv[]) {
  CefMainArgs main_args(argc, argv);
  CefRefPtr<ClientApp> app(new ClientApp);

  // Execute the secondary process, if any.
  int exit_code = CefExecuteProcess(main_args, app.get());
  if (exit_code >= 0)
    return exit_code;

  // Retrieve the current working directory.
  getcwd(szWorkingDir, sizeof(szWorkingDir));

  // Initialize the AutoRelease pool.
  NSAutoreleasePool* autopool = [[NSAutoreleasePool alloc] init];

  // Initialize the ClientApplication instance.
  [ClientApplication sharedApplication];

  // Parse command line arguments.
  AppInitCommandLine(argc, argv);

  CefSettings settings;

  // Populate the settings based on command line arguments.
  AppGetSettings(settings);

  // Initialize CEF.
  CefInitialize(main_args, settings, app.get());

  // Register the scheme handler.
  scheme_test::InitTest();

  // Create the application delegate and window.
  NSObject* delegate = [[ClientAppDelegate alloc] init];
  [delegate performSelectorOnMainThread:@selector(createApp:) withObject:nil
                          waitUntilDone:NO];

  // Run the application message loop.
  CefRunMessageLoop();

  // Shut down CEF.
  CefShutdown();

  // Release the handler.
  g_handler = NULL;

  // Release the delegate.
  [delegate release];

  // Release the AutoRelease pool.
  [autopool release];

  return 0;
}


// Global functions

std::string AppGetWorkingDirectory() {
  return szWorkingDir;
}

void AppQuitMessageLoop() {
  CefQuitMessageLoop();
}
