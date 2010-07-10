// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef _CEF_CONNECTOR_H
#define _CEF_CONNECTOR_H

#include "base/basictypes.h"
#include "base/linked_ptr.h"
#include "base/lock.h"
#include "base/ref_counted.h"
#include "base/string16.h"
#include "chrome/browser/chrome_thread.h"
#include "chrome/browser/tabs/tab_strip_model.h"
#include "chrome/common/notification_registrar.h"
#include "gfx/native_widget_types.h"
#include "gfx/rect.h"
#include "googleurl/src/gurl.h"

class Browser;
class CefChannel;

class CefConnector : public base::RefCountedThreadSafe<CefConnector,
                                                ChromeThread::DeleteOnUIThread>
{
public:
  // Class that listens for browser events.
  class BrowserListener : public TabStripModelObserver,
                          public NotificationObserver
  {
  public:
    BrowserListener(CefConnector* connector);
    ~BrowserListener();

    // From TabStripModelObserver.
    virtual void TabInsertedAt(TabContents* contents,
                               int index,
                               bool foreground);
    virtual void TabDetachedAt(TabContents* contents, int index);
    virtual void TabSelectedAt(TabContents* old_contents,
                               TabContents* new_contents,
                               int index,
                               bool user_gesture);
    virtual void TabChangedAt(TabContents* contents, int index,
                              TabChangeType change_type);
    virtual void TabReplacedAt(TabContents* old_contents,
                               TabContents* new_contents, int index);

    // From NotificationObserver.
    virtual void Observe(NotificationType type,
                         const NotificationSource& source,
                         const NotificationDetails& details);

  private:
    // Add add observers for the specified tab.
    void AddObservers(TabContents* contents);
    // Remove all observers for the specified tab.
    void RemoveObservers(TabContents* contents);

    CefConnector* connector_;

    // Used to manage observers.
    NotificationRegistrar registrar_;
  };

  CefConnector(CefChannel* channel, gfx::NativeWindow parent,
      const gfx::Rect& bounds, const std::string& browser_id,
      Browser* browser);
  ~CefConnector();

  // Called immediately before the connector is removed from the channel. The
  // connector object may not be deleted until much later.
  void Cleanup();

  // Called from the Browser destructor.
  void Destroy();

  CefChannel* channel() { return channel_; }
  gfx::NativeWindow parent() { return parent_; }
  const gfx::Rect& bounds() { return bounds_; }
  const std::string& browser_id() { return browser_id_; }
  Browser* browser() { return browser_; }

  // Called on the UI thread by BrowserFrameWin::Init().
  void NotifyBrowserInitialized(gfx::NativeWindow window_handle);

  // Called on the UI thread as a result of IPC message events.
  void OnCreateTab(int index, const std::string& url,
      const std::string& referrer, bool foreground);
  void OnTabCommand(int index, int command);
  void OnTabLoadAddress(int index, const std::string& url,
      const std::string& referrer);
  void OnTabExecuteJavaScript(int index, const std::wstring& frame_xpath,
                              const std::wstring& jscript);

private:
  // Called on the IO thread.
  void OnSendBrowserInitialized(gfx::NativeWindow window_handle);
  void OnSendTabAdded(int index, bool foreground);
  void OnSendTabRemoved(int index);
  void OnSendTabSelected(int index, bool user_gesture);
  void OnSendTabChanged(int index, const std::string& url,
      const std::wstring& title);
  void OnSendTabNavStatus(int index, bool can_go_back, bool can_go_forward);
  void OnSendTabLoadStart(int index);
  void OnSendTabLoadStop(int index);

  // Class level lock.  Used to protect internal data structures that may be
  // accessed from other threads.
  Lock lock_;

  CefChannel* channel_;
  gfx::NativeWindow parent_;
  gfx::Rect bounds_;
  std::string browser_id_;
  Browser* browser_;

  BrowserListener browser_listener_;

  DISALLOW_COPY_AND_ASSIGN(CefConnector);
};

#endif // _CEF_CONNECTOR_H
