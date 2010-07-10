// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cef_connector.h"
#include "cef_channel.h"
#include "common/cef_messages.h"
#include "chrome/browser/browser.h"
#include "chrome/browser/renderer_host/render_view_host.h"
#include "chrome/browser/tab_contents/tab_contents.h"
#include "chrome/common/notification_service.h"

// CefConnector::BrowserListener class

CefConnector::BrowserListener::BrowserListener(CefConnector* connector)
  : connector_(connector)
{
}

CefConnector::BrowserListener::~BrowserListener()
{
}

void CefConnector::BrowserListener::TabInsertedAt(TabContents* contents,
                                                  int index, bool foreground)
{
  AddObservers(contents);
  
  ChromeThread::PostTask(ChromeThread::IO, FROM_HERE,
      NewRunnableMethod(connector_, &CefConnector::OnSendTabAdded, index,
          foreground));
}

void CefConnector::BrowserListener::TabDetachedAt(TabContents* contents,
                                                  int index)
{
  RemoveObservers(contents);
  
  ChromeThread::PostTask(ChromeThread::IO, FROM_HERE,
      NewRunnableMethod(connector_, &CefConnector::OnSendTabRemoved, index));
}

void CefConnector::BrowserListener::TabSelectedAt(TabContents* old_contents,
                                                  TabContents* new_contents,
                                                  int index, bool user_gesture)
{
  ChromeThread::PostTask(ChromeThread::IO, FROM_HERE,
      NewRunnableMethod(connector_, &CefConnector::OnSendTabSelected, index,
          user_gesture));
}

void CefConnector::BrowserListener::TabChangedAt(TabContents* contents,
                                                 int index,
                                                 TabChangeType change_type)
{
  std::string url = contents->GetURL().spec();
  std::wstring title = contents->GetTitle().c_str();

  ChromeThread::PostTask(ChromeThread::IO, FROM_HERE,
      NewRunnableMethod(connector_, &CefConnector::OnSendTabChanged, index,
          url, title));
}

void CefConnector::BrowserListener::TabReplacedAt(TabContents* old_contents,
                                                  TabContents* new_contents,
                                                  int index)
{
  RemoveObservers(old_contents);
  AddObservers(new_contents);
}

void CefConnector::BrowserListener::Observe(NotificationType type,
                                            const NotificationSource& source,
                                            const NotificationDetails& details)
{
  if(type == NotificationType::NAV_ENTRY_COMMITTED ||
     type == NotificationType::LOAD_START ||
     type == NotificationType::LOAD_STOP) {
    NavigationController* controller =
        Source<NavigationController>(source).ptr();
    int index = connector_->browser()->tabstrip_model()->GetIndexOfTabContents(
        controller->tab_contents());
    if(index < 0) {
      NOTREACHED();
      return;
    }

    if(type == NotificationType::NAV_ENTRY_COMMITTED) {
      bool can_go_back = controller->CanGoBack();
      bool can_go_forward = controller->CanGoForward();
      
      ChromeThread::PostTask(ChromeThread::IO, FROM_HERE,
        NewRunnableMethod(connector_, &CefConnector::OnSendTabNavStatus, index,
            can_go_back, can_go_forward));
    } else if(type == NotificationType::LOAD_START) {
      ChromeThread::PostTask(ChromeThread::IO, FROM_HERE,
        NewRunnableMethod(connector_, &CefConnector::OnSendTabLoadStart,
            index));
    } else if(type == NotificationType::LOAD_STOP) {
      ChromeThread::PostTask(ChromeThread::IO, FROM_HERE,
        NewRunnableMethod(connector_, &CefConnector::OnSendTabLoadStop, index));
    }
  }
}

void CefConnector::BrowserListener::AddObservers(TabContents* contents)
{
  registrar_.Add(this, NotificationType::NAV_ENTRY_COMMITTED,
      Source<NavigationController>(&contents->controller()));
  registrar_.Add(this, NotificationType::LOAD_START,
      Source<NavigationController>(&contents->controller()));
  registrar_.Add(this, NotificationType::LOAD_STOP,
      Source<NavigationController>(&contents->controller()));
}

void CefConnector::BrowserListener::RemoveObservers(TabContents* contents)
{
  registrar_.Remove(this, NotificationType::NAV_ENTRY_COMMITTED,
      Source<NavigationController>(&contents->controller()));
  registrar_.Remove(this, NotificationType::LOAD_START,
      Source<NavigationController>(&contents->controller()));
  registrar_.Remove(this, NotificationType::LOAD_STOP,
      Source<NavigationController>(&contents->controller()));
}


// CefConnector class

CefConnector::CefConnector(CefChannel* channel,
                           gfx::NativeWindow parent,
                           const gfx::Rect& bounds,
                           const std::string& browser_id,
                           Browser* browser)
  : channel_(channel), parent_(parent), bounds_(bounds),
    browser_id_(browser_id), browser_(browser),
    ALLOW_THIS_IN_INITIALIZER_LIST(browser_listener_(this))
{
  browser_->tabstrip_model()->AddObserver(&browser_listener_);
}

CefConnector::~CefConnector()
{
}

void CefConnector::Cleanup()
{
  Browser* browser;
  
  {
    AutoLock lock_scope(lock_);
    browser = browser_;
    browser_ = NULL;
  }

  if(browser) {
    // Clear the browser's CefConnector pointer so Destroy() isn't called twice.
    browser->set_cef_connector(NULL);
  }
}

void CefConnector::Destroy()
{
  std::string browser_id;
  {
    AutoLock lock_scope(lock_);

    // Called from the Browser destructor, so set the browser pointer to NULL.
    browser_->tabstrip_model()->RemoveObserver(&browser_listener_);
    browser_ = NULL;
    browser_id = browser_id_;
  }
  channel_->RemoveConnector(browser_id);
}

void CefConnector::NotifyBrowserInitialized(gfx::NativeWindow window_handle)
{
  ChromeThread::PostTask(ChromeThread::IO, FROM_HERE,
      NewRunnableMethod(this, &CefConnector::OnSendBrowserInitialized,
          window_handle));
}

void CefConnector::OnCreateTab(int index, const std::string& url,
                               const std::string& referrer, bool foreground)
{
  browser_->AddTabWithURL(GURL(url), GURL(referrer), PageTransition::TYPED,
      index, (foreground?Browser::ADD_SELECTED:Browser::ADD_NONE), NULL,
      std::string());
}

void CefConnector::OnTabCommand(int index, int command)
{
  TabContents* contents = browser_->tabstrip_model()->GetTabContentsAt(index);
  if(!contents) {
    NOTREACHED();
    return;
  }

  switch(command) {
    case CEF_TC_TAB_DESTROY:
      browser_->tabstrip_model()->CloseTabContentsAt(index);
      break;
    case CEF_TC_TAB_SELECT:
      browser_->tabstrip_model()->SelectTabContentsAt(index, false);
      break;
    case CEF_TC_GOBACK:
      contents->controller().GoBack();
      break;
    case CEF_TC_GOFORWARD:
      contents->controller().GoForward();
      break;
    case CEF_TC_RELOAD:
      contents->controller().Reload(true);
      break;
    case CEF_TC_STOPLOAD:
      contents->Stop();
      break;
    case CEF_TC_UNDO:
      contents->render_view_host()->Undo();
      break;
    case CEF_TC_REDO:
      contents->render_view_host()->Redo();
      break;
    case CEF_TC_CUT:
      contents->render_view_host()->Cut();
      break;
    case CEF_TC_COPY:
      contents->render_view_host()->Copy();
      break;
    case CEF_TC_PASTE:
      contents->render_view_host()->Paste();
      break;
    case CEF_TC_DELETE:
      contents->render_view_host()->Delete();
      break;
    case CEF_TC_SELECTALL:
      contents->render_view_host()->SelectAll();
      break;
    case CEF_TC_PRINT:
      contents->PrintNow();
      break;
    case CEF_TC_VIEWSOURCE:
      {
        NavigationEntry* entry = contents->controller().GetLastCommittedEntry();
        if (entry) {
          GURL url("view-source:" + entry->url().spec());
          browser_->OpenURL(url, GURL(), NEW_FOREGROUND_TAB,
              PageTransition::LINK);
        }
      } break;
    default:
      NOTREACHED();
  }
}

void CefConnector::OnTabLoadAddress(int index, const std::string& url,
                                    const std::string& referrer)
{
  TabContents* contents = browser_->tabstrip_model()->GetTabContentsAt(index);
  if(!contents) {
    NOTREACHED();
    return;
  }

  contents->OpenURL(GURL(url), GURL(referrer), CURRENT_TAB,
      PageTransition::TYPED);
}

void CefConnector::OnTabExecuteJavaScript(int index,
                                          const std::wstring& frame_xpath,
                                          const std::wstring& jscript)
{
  TabContents* contents = browser_->tabstrip_model()->GetTabContentsAt(index);
  if(!contents) {
    NOTREACHED();
    return;
  }

  contents->render_view_host()->ExecuteJavascriptInWebFrame(frame_xpath,
      jscript);
}

void CefConnector::OnSendBrowserInitialized(gfx::NativeWindow window_handle)
{
  channel_->Send(new CefHostMsg_BrowserCreated(browser_id_,
      gfx::IdFromNativeView(window_handle)));
}

void CefConnector::OnSendTabAdded(int index, bool foreground)
{
  channel_->Send(new CefHostMsg_TabAdded(browser_id_, index, foreground));
}

void CefConnector::OnSendTabRemoved(int index)
{
  channel_->Send(new CefHostMsg_TabRemoved(browser_id_, index));
}

void CefConnector::OnSendTabSelected(int index, bool user_gesture)
{
  channel_->Send(new CefHostMsg_TabSelected(browser_id_, index, user_gesture));
}

void CefConnector::OnSendTabChanged(int index, const std::string& url,
                                    const std::wstring& title)
{
  channel_->Send(new CefHostMsg_TabChanged(browser_id_, index, url, title));
}

void CefConnector::OnSendTabNavStatus(int index, bool can_go_back,
                                      bool can_go_forward)
{
  channel_->Send(new CefHostMsg_TabNavStatus(browser_id_, index, can_go_back,
      can_go_forward));
}

void CefConnector::OnSendTabLoadStart(int index)
{
  channel_->Send(new CefHostMsg_TabLoadStart(browser_id_, index));
}

void CefConnector::OnSendTabLoadStop(int index)
{
  channel_->Send(new CefHostMsg_TabLoadStop(browser_id_, index));
}
