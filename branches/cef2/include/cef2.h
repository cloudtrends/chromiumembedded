// Copyright (c) 2010 Marshall A. Greenblatt. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the name Chromium Embedded
// Framework nor the names of its contributors may be used to endorse
// or promote products derived from this software without specific prior
// written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// ---------------------------------------------------------------------------
//
// The contents of this file must follow a specific format in order to
// support the CEF translator tool. See the translator.README.txt file in the
// tools directory for more information.
//


#ifndef _CEF2_H
#define _CEF2_H

#include "cef2_base.h"
#include "cef2_types.h"
#include <string>


// This function should be called once when the application is started to
// initialize CEF.  A return value of true indicates that it succeeded and false
// indicates that it failed. Set |chrome_path| to the full path of the
// chrome.exe file. If |chrome_path| is empty then chome.exe is assumed to
// exist in the current working directory. Set |multi_threaded_message_loop| to
// true to have the message loop run in a separate thread.  If
// |multi_threaded_message_loop| is false than the CefDoMessageLoopWork()
// function must be called from your message loop.
/*--cef()--*/
bool CefInitialize(const std::wstring& chrome_path,
                   bool multi_threaded_message_loop);

// This function should be called once before the application exits to shut down
// CEF and destroy any created windows.
/*--cef()--*/
void CefShutdown();

// Perform message loop processing.  Has no affect if the browser UI loop is
// running in a separate thread.
/*--cef()--*/
void CefDoMessageLoopWork();


class CefBrowser;
class CefFrame;
class CefProfile;
class CefTab;


// Create a new CefProfile object or return the existing CefProfile object
// that uses the specified |user_profile| directory.  If |user_profile| is empty
// the default directory will be used.
/*--cef()--*/
CefRefPtr<CefProfile> CefCreateProfile(const std::wstring& user_profile);


// Implement this interface to handle browser events.  All methods in this
// interface will be called on the UI thread.
/*--cef(source=client)--*/
class CefBrowserListener : public CefBase
{
public:
  // Called when a browser is created.
  /*--cef()--*/
  virtual void OnCreate(CefRefPtr<CefBrowser> browser) =0;

  // Called when a browser is destroyed.
  /*--cef()--*/
  virtual void OnDestroy(CefRefPtr<CefBrowser> browser) =0;
};


// Implement this interface to handle tab events.  All methods in this
// interface will be called on the UI thread.
/*--cef(source=client)--*/
class CefTabListener : public CefBase
{
public:
  // Called when a tab is created.
  /*--cef()--*/
  virtual void OnCreate(CefRefPtr<CefTab> tab) =0;

  // Called when a tab is destroyed.
  /*--cef()--*/
  virtual void OnDestroy(CefRefPtr<CefTab> tab) =0;

  // Called when the address changes.
  /*--cef()--*/
  virtual void OnAddressChange(CefRefPtr<CefTab> tab,
                               const std::wstring& url) =0;

  // Called when the page title changes.
  /*--cef()--*/
  virtual void OnTitleChange(CefRefPtr<CefTab> tab,
                             const std::wstring& title) =0;

  // Called when the navigation state changes.
  /*--cef()--*/
  virtual void OnNavStateChange(CefRefPtr<CefTab> tab, bool can_go_back,
                                bool can_go_forward) =0;

  // Called when the selection state changes.
  /*--cef()--*/
  virtual void OnSelectChange(CefRefPtr<CefTab> tab, bool selected) =0;

  // Called when the page load has started.
  /*--cef()--*/
  virtual void OnLoadStart(CefRefPtr<CefTab> tab) =0;

  // Called when the page load has stopped.
  /*--cef()--*/
  virtual void OnLoadStop(CefRefPtr<CefTab> tab) =0;
};


// In Chromium, each browser process is associated with a unique user profile.
// This class represents the connection between the current CEF host process and
// the Chromium browser process for a given user profile. Multiple CEF host
// processes can share the same Chromium browser process by passing the same
// user profile argument to CefCreateProfile(). The Chromium browser process for
// a given user profile will be created when the first browser window for that
// user profile is created and destroyed when the last browser window for that
// user profile is destroyed. All methods exposed by this class should be thread
// safe.
/*--cef(source=library)--*/
class CefProfile : public CefBase
{
public:
  // Add a browser listener.
  /*--cef()--*/
  virtual void AddBrowserListener(const std::wstring& id_string,
                                  CefRefPtr<CefBrowserListener> listener) =0;
  // Remove a browser listener.
  /*--cef()--*/
  virtual void RemoveBrowserListener(const std::wstring& id_string) =0;
  
  // Add a tab listener.
  /*--cef()--*/
  virtual void AddTabListener(const std::wstring& id_string,
                              CefRefPtr<CefTabListener> listener) =0;
  // Remove a tab listener.
  /*--cef()--*/
  virtual void RemoveTabListener(const std::wstring& id_string) =0;
  
  // Destroy the profile.
  /*--cef()--*/
  virtual void Destroy() =0;
  
  // Creates a new browser instance. If |parent| is NULL then the browser window
  // will be created as a framed "popup" window parented to the desktop. The
  // |xpos|, |ypos|, |width| and |height| values specify the positioning of the
  // browser window in parent window client coordinates. The |command_line|
  // argument will be passed to the Chromium process and supports all standard
  // Chromium command-line flags.  This method call will not block. All
  // registered BrowserListener objects will be notified on the UI thread once
  // the browser window has been created.
  /*--cef()--*/
  virtual CefRefPtr<CefBrowser> CreateBrowser(const std::wstring& id_string,
                                              CefWindowHandle parent, int xpos,
                                              int ypos, int width, int height,
                                              const std::wstring& command_line)
                                              =0;
};


// This class represents a browser window. Browser windows may have a tab strip
// that contains one or more tabs.
/*--cef(source=library)--*/
class CefBrowser : public CefBase
{
public:
  // Returns the CefProfile that owns this browser.
  /*--cef()--*/
  virtual CefRefPtr<CefProfile> GetProfile() =0;

  // Destroy the browser window.
  /*--cef()--*/
  virtual void Destroy() =0;

  // Returns the ID string associated with this browser.
  /*--cef()--*/
  virtual std::wstring GetIdString() =0;

  // Retrieve the window handle for this browser.
  /*--cef()--*/
  virtual CefWindowHandle GetWindowHandle() =0;

  // Create a new tab object. The tab will be created at the specified
  // zero-based index in the tab strip.  If |foreground| is true the new tab
  // will be selected.
  /*--cef()--*/
  virtual CefRefPtr<CefTab> CreateTab(int index, const std::wstring& url,
                                      const std::wstring& referrer,
                                      bool foreground) =0;

  // Returns the number of tabs that currently exist in the tab stip for this
  // browser.
  /*--cef()--*/
  virtual int GetTabCount() =0;

  // Return the tab at the specified zero-based index.
  /*--cef()--*/
  virtual CefRefPtr<CefTab> GetTab(int index) =0;

  // Return the currently selected tab.
  /*--cef()--*/
  virtual CefRefPtr<CefTab> GetSelectedTab() =0;
};


// This class represents a tab in a browser window's tab strip.
/*--cef(source=library)--*/
class CefTab : public CefBase
{
public:
  // Returns the CefBrowser that owns this CefTab object.
  /*--cef()--*/
  virtual CefRefPtr<CefBrowser> GetBrowser() =0;

  // Destroy the tab.
  /*--cef()--*/
  virtual void Destroy() =0;

  // Returns the index of this tab in the tab strip.
  /*--cef()--*/
  virtual int GetIndex() =0;

  // Returns true if this tab is selected in the tab strip.
  /*--cef()--*/
  virtual bool IsSelected() =0;
  // Select this tab in the tab strip.
  /*--cef()--*/
  virtual void Select() =0;

  // Returns true if this tab can navigate backwards.
  /*--cef()--*/
  virtual bool CanGoBack() =0;
  // Navigate backwards.
  /*--cef()--*/
  virtual void GoBack() =0;
  // Returns true if this tab can navigate forwards.
  /*--cef()--*/
  virtual bool CanGoForward() =0;
  // Navigate backwards.
  /*--cef()--*/
  virtual void GoForward() =0;
  // Returns trie of this tab is currently loading.
  /*--cef()--*/
  virtual bool IsLoading() =0;
  // Reload the current page.
  /*--cef()--*/
  virtual void Reload() =0;
  // Stop loading the page.
  /*--cef()--*/
  virtual void StopLoad() =0;

  // Returns the current address for this tab.
  /*--cef()--*/
  virtual std::wstring GetAddress() =0;
  
  // Returns the current title for this tab.
  /*--cef()--*/
  virtual std::wstring GetTitle() =0;
  
  // Execute undo in the currently active selection context.
  /*--cef()--*/
  virtual void Undo() =0;
  // Execute redo in the currently active selection context.
  /*--cef()--*/
  virtual void Redo() =0;
  // Execute cut in the currently active selection context.
  /*--cef()--*/
  virtual void Cut() =0;
  // Execute copy in the currently active selection context.
  /*--cef()--*/
  virtual void Copy() =0;
  // Execute paste in the currently active selection context.
  /*--cef()--*/
  virtual void Paste() =0;
  // Execute delete in the currently active selection context.
  /*--cef(capi_name=del)--*/
  virtual void Delete() =0;
  // Execute select all in the currently active selection context.
  /*--cef()--*/
  virtual void SelectAll() =0;

  // Execute printing in the currently active frame.
  /*--cef()--*/
  virtual void Print() =0;

  // Execute view source in the currently active frame.
  /*--cef()--*/
  virtual void ViewSource() =0;

  // Load the specified |url| in the main frame.
  /*--cef()--*/
  virtual void LoadAddress(const std::wstring& url,
                           const std::wstring& referrer) =0;
  
  // Execute a string of JavaScript code in the specified |frame_xpath|. Leave
  // |frame_xpath| empty to execute in the main frame.
  /*--cef()--*/
  virtual void ExecuteJavaScript(const std::wstring& frame_xpath, 
                                 const std::wstring& jscript) =0;
};


#endif // _CEF2_H
