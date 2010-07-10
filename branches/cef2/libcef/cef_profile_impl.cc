// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "cef_profile_impl.h"
#include "cef_context.h"
#include "common/cef_messages.h"

#include "base/command_line.h"
#include "base/rand_util.h"
#include "base/string_util.h"
#include "gfx/native_widget_types.h"

CefProfileImpl::CefProfileImpl(const std::wstring& user_profile)
  : user_profile_(user_profile), shutting_down_(false), channel_created_(false),
    channel_connected_(false), process_starting_(false),
    process_handle_(base::kNullProcessHandle),
    listener_manager_(new CefListenerManager)
{
}

CefProfileImpl::~CefProfileImpl()
{
}

void CefProfileImpl::Cleanup()
{
  RemoveAllBrowsers();

  AutoLock lock_scope(this);
  
  channel_created_ = false;
  channel_connected_ = false;
  process_starting_ = false;

  // Delete any messages on the queue.
  while(!pending_messages_.empty()) {
    delete pending_messages_.front();
    pending_messages_.pop();
  }

  if(process_handle_) {
    base::CloseProcessHandle(process_handle_);
    process_handle_ = base::kNullProcessHandle;
  }

  // The IPC server object will be destroyed on the IO thread.
  server_ = NULL;
}

void CefProfileImpl::RemoveBrowser(const std::wstring& id_string)
{
  CefRefPtr<CefBrowserImpl> browser;
  
  {
    AutoLock lock_scope(this);

    // Call cleanup and remove the browser if it exists.
    BrowserMap::iterator it = browsers_.find(id_string);
    if(it != browsers_.end()) {
      RemovePendingBrowser(it->second);
      browser = it->second;
      browsers_.erase(it);
    }
  }

  if(browser.get())
    browser->Cleanup();
}

void CefProfileImpl::RemoveAllBrowsers()
{
  BrowserMap browsers;
  
  {
    AutoLock lock_scope(this);
    browsers = browsers_;
    browsers_.clear();
    RemoveAllPendingBrowsers();
  }

  // Call cleanup on all existing browsers.
  BrowserMap::iterator it = browsers.begin();
  for(; it != browsers.end(); ++it)
    it->second->Cleanup();
}

CefRefPtr<CefBrowserImpl> CefProfileImpl::GetOrCreateBrowser(
    const std::wstring& id_string)
{
  AutoLock lock_scope(this);

  // Locate the existing browser if any.
  BrowserMap::const_iterator it = browsers_.find(id_string);
  if(it != browsers_.end())
    return it->second;

  // Create and return a new browser object. The new browser object is assumed
  // to already be initialized.
  CefRefPtr<CefBrowserImpl> browserPtr(new CefBrowserImpl(this, id_string));
  browsers_.insert(std::make_pair<std::wstring, CefRefPtr<CefBrowserImpl>>(
      id_string, browserPtr));

  return browserPtr.get();
}

void CefProfileImpl::Destroy()
{
  std::wstring user_profile;
  {
    AutoLock lock_scope(this);

    if(shutting_down_)
      return;
    shutting_down_ = true;
    user_profile = user_profile_;
  }

  // Remove this profile from the context.  This will result in a call to
  // CefProfileImpl::Cleanup().
  _Context->RemoveProfile(user_profile);
}

static std::wstring GenerateRandomBrowserID(void* instance)
{
  // Build the browser ID.  This is composed of a unique identifier for the
  // parent browser process, an identifier for the child instance, and a random
  // component. We use a random component to help guarantee uniqueness.
  return StringPrintf(L"%d.%p.%d",
                      base::GetCurrentProcId(), instance,
                      base::RandInt(0, std::numeric_limits<int>::max()));
}

CefRefPtr<CefBrowser> CefProfileImpl::CreateBrowser(
    const std::wstring& id_string, CefWindowHandle parent, int xpos, int ypos,
    int width, int height, const std::wstring& command_line)
{
  CefRefPtr<CefBrowserImpl> browser;
  
  {
    AutoLock lock_scope(this);

    if(shutting_down_)
      return NULL;

    std::wstring browser_id;
    if(id_string.empty()) {
      browser_id = GenerateRandomBrowserID(this);
    } else {
      // Check if a browser with the specified id_string already exists.
      BrowserMap::const_iterator it = browsers_.find(id_string);
      if(it != browsers_.end())
        browser_id = GenerateRandomBrowserID(this);
      else
        browser_id = id_string;
    }

    // Create a new browser object.
    browser = new CefBrowserImpl(this, browser_id, parent,
        gfx::Rect(xpos, ypos, width, height), command_line);
    browsers_.insert(std::make_pair<std::wstring, CefRefPtr<CefBrowserImpl>>(
        id_string, browser));

    // Add the browser to the pending initialization list.
    PushPendingBrowser(browser);
  }
  
  // Continue browser initialization on the IO thread.
  CefThread::PostTask(CefThread::IO, FROM_HERE,
      NewRunnableMethod(this, &CefProfileImpl::InitializeBrowser));

  // Return the new browser object.
  return browser.get();
}

void CefProfileImpl::OnChannelCreated()
{
  {
    AutoLock lock_scope(this);

    if(shutting_down_)
      return;

    channel_created_ = true;
  }

  // Continue browser initialization on the IO thread.
  CefThread::PostTask(CefThread::IO, FROM_HERE,
      NewRunnableMethod(this, &CefProfileImpl::InitializeBrowser));
}

void CefProfileImpl::OnMessageReceived(const IPC::Message& msg)
{
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(CefProfileImpl, msg)
    IPC_MESSAGE_HANDLER(CefHostMsg_BrowserCreated, OnBrowserCreated)
    IPC_MESSAGE_HANDLER(CefHostMsg_TabAdded, OnTabAdded)
    IPC_MESSAGE_HANDLER(CefHostMsg_TabRemoved, OnTabRemoved)
    IPC_MESSAGE_HANDLER(CefHostMsg_TabSelected, OnTabSelected)
    IPC_MESSAGE_HANDLER(CefHostMsg_TabChanged, OnTabChanged)
    IPC_MESSAGE_HANDLER(CefHostMsg_TabNavStatus, OnTabNavStatus)
    IPC_MESSAGE_HANDLER(CefHostMsg_TabLoadStart, OnTabLoadStart)
    IPC_MESSAGE_HANDLER(CefHostMsg_TabLoadStop, OnTabLoadStop)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
}

void CefProfileImpl::OnChannelConnected(int32 peer_pid)
{
  MessageQueue pending_messages;

  {
    AutoLock lock_scope(this);

    if(shutting_down_)
      return;

    channel_connected_ = true;
    process_starting_ = false;

#if defined(IPC_MESSAGE_LOG_ENABLED)
    bool enabled = IPC::Logging::current()->Enabled();
    server_->Send(new CefMsg_SetIPCLoggingEnabled(enabled));
#endif

    pending_messages = pending_messages_;
    pending_messages_.empty();
  }

  // Send the pending queued messages, if any.
  while(!pending_messages.empty()) {
    Send(pending_messages.front());
    pending_messages.pop();
  }

  // Continue browser initialization.
  CefThread::PostTask(CefThread::IO, FROM_HERE,
      NewRunnableMethod(this, &CefProfileImpl::InitializeBrowser));
}

void CefProfileImpl::OnChannelError()
{
  // Reset the profile state.
  Cleanup();
}

bool CefProfileImpl::Send(IPC::Message* msg)
{
  if(!channel_connected_) {
    AutoLock lock_scope(this);

    // Queue messages if the channel is not yet connected.
    pending_messages_.push(msg);
    return true;
  }

  return server_->Send(msg);
}

void CefProfileImpl::InitializeBrowser()
{
  AutoLock lock_scope(this);

  if(shutting_down_)
    return;

  if(channel_created_) {
    if(channel_connected_) {
      // Retrieve all pending browsers.
      CefRefPtr<CefBrowserImpl> browser;
      while(PopPendingBrowser(browser)) {
        // Send a message to the existing chrome process to create each new
        // browser.
      }
    } else if(!process_starting_) {
      // Retrieve the first pending browser.
      CefRefPtr<CefBrowserImpl> browser;
      if(!PopPendingBrowser(browser))
        return;
      
      // Start a new chrome process that will create the first pending browser.
      // OnChannelConnected() will be called once the chrome process has
      // connected.
      process_starting_ = true;
      browser->AddRef();
      CefThread::PostTask(CefThread::PROCESS_LAUNCHER, FROM_HERE,
          NewRunnableMethod(this, &CefProfileImpl::CreateBrowserProcess,
          browser.get(), user_profile_));
    }
  } else if(!server_.get()) {
    // Create the server. OnChannelCreated() will be called once the server
    // channel is created.
    server_ = new CefServer(this);
    server_->CreateChannel();
  }
}

void CefProfileImpl::OnProcessLaunched(base::ProcessHandle handle)
{
  AutoLock lock_scope(this);

  if(shutting_down_) {
    base::CloseProcessHandle(handle);
    return;
  }
  process_handle_ = handle;
}

void CefProfileImpl::OnProcessLaunchError()
{
  // Reset the profile state.
  Cleanup();
}

bool CefProfileImpl::IsAllowedSwitch(const std::string& switch_string,
                                     const std::wstring& value_string)
{
  if(switch_string == "user-data-dir")
    return false;

  return true;
}

void CefProfileImpl::CreateBrowserProcess(CefBrowserImpl* browser,
                                          const std::wstring& user_profile)
{
  CefRefPtr<CefBrowserImpl> browserPtr(browser);
  browser->Release();

  // Chrome executable path.
  const FilePath& chrome_path = _Context->chrome_path();
  
  // Parse the browser command line.
  CommandLine browser_cmd = CommandLine::FromString(
      StringPrintf(L"placeholder %ls", browser->command_line().c_str()));

  // Build the launch command line.
  CommandLine command_line(chrome_path);

  // Add the app-cef switch.
  const gfx::Rect& rect = browser->rect();
  const std::string id_string = WideToUTF8(browser->id_string());
  command_line.AppendSwitchWithValue("app-cef",
      StringPrintf("%d,%d,%d,%d,%d,%s,%s", browser->parent(), rect.x(),
          rect.y(), rect.width(), rect.height(), server_->channel_id().c_str(),
          id_string.c_str()));

  // Add the user-data-dir switch.
  if(!user_profile.empty())
    command_line.AppendSwitchWithValue("user-data-dir", user_profile);

  // Only add allowed switches from the browser configuration.
  typedef std::map<std::string, std::wstring> SwitchMap;
  SwitchMap switches = browser_cmd.GetSwitches();
  SwitchMap::const_iterator it = switches.begin();
  for(; it != switches.end(); ++it) {
    if(!command_line.HasSwitch(it->first)
      && IsAllowedSwitch(it->first, it->second)) {
      if(it->second.empty())
        command_line.AppendSwitch(it->first);
      else
        command_line.AppendSwitchWithValue(it->first, it->second);
    }
  }

  if(browser_cmd.HasSwitch("debug-cef-chrome")) {
    // Output the app-cef switch value to the debug console instead of launching
    // the process.
    OutputDebugStringW(command_line.command_line_string().c_str());
    return;
  }

  // Launch the process.
  base::ProcessHandle handle = base::kNullProcessHandle;
  if(!base::LaunchApp(command_line, false, false, &handle)) {
    // The chrome process could not be launched.
    CefThread::PostTask(CefThread::IO, FROM_HERE,
        NewRunnableMethod(this, &CefProfileImpl::OnProcessLaunchError));
    return;
  }

  CefThread::PostTask(CefThread::IO, FROM_HERE,
      NewRunnableMethod(this, &CefProfileImpl::OnProcessLaunched, handle));
}

void CefProfileImpl::RemovePendingBrowser(CefRefPtr<CefBrowserImpl> browser)
{
  if(pending_browsers_.empty())
    return;

  std::wstring id_string = browser->GetIdString();
  BrowserList::iterator it = pending_browsers_.begin();
  for(; it != pending_browsers_.end(); ++it) {
    if(_wcsicmp((*it)->GetIdString().c_str(), id_string.c_str()) == 0) {
      pending_browsers_.erase(it);
      break;
    }
  }
}

void CefProfileImpl::RemoveAllPendingBrowsers()
{
  pending_browsers_.empty();
}

void CefProfileImpl::PushPendingBrowser(CefRefPtr<CefBrowserImpl> browser)
{
  pending_browsers_.push_back(browser);
}

bool CefProfileImpl::PopPendingBrowser(CefRefPtr<CefBrowserImpl>& browser)
{
  if(pending_browsers_.empty())
    return false;

  browser = pending_browsers_.front();
  pending_browsers_.pop_front();
  return true;
}

void CefProfileImpl::OnBrowserCreated(const std::string& browser_id,
                                      gfx::NativeViewId browser_handle)
{
  GetOrCreateBrowser(UTF8ToWide(browser_id))->OnCreated(
      gfx::NativeViewFromId(browser_handle));
}

void CefProfileImpl::OnTabAdded(const std::string& browser_id, int index,
                                bool foreground)
{
  GetOrCreateBrowser(UTF8ToWide(browser_id))->OnTabAdded(index, foreground);
}

void CefProfileImpl::OnTabRemoved(const std::string& browser_id, int index)
{
  GetOrCreateBrowser(UTF8ToWide(browser_id))->OnTabRemoved(index);
}

void CefProfileImpl::OnTabSelected(const std::string& browser_id, int index,
                                   bool user_gesture)
{
  GetOrCreateBrowser(UTF8ToWide(browser_id))->OnTabSelected(index,
      user_gesture);
}
  
void CefProfileImpl::OnTabChanged(const std::string& browser_id, int index,
                                  const std::string& url,
                                  const std::wstring& title)
{
  GetOrCreateBrowser(UTF8ToWide(browser_id))->OnTabChanged(index,
      UTF8ToWide(url), title);
}

void CefProfileImpl::OnTabNavStatus(const std::string& browser_id, int index,
                                    bool can_go_back, bool can_go_forward)
{
  GetOrCreateBrowser(UTF8ToWide(browser_id))->OnTabNavStatus(index,
      can_go_back, can_go_forward);
}

void CefProfileImpl::OnTabLoadStart(const std::string& browser_id, int index)
{
  GetOrCreateBrowser(UTF8ToWide(browser_id))->OnTabLoadStart(index);
}

void CefProfileImpl::OnTabLoadStop(const std::string& browser_id, int index)
{
  GetOrCreateBrowser(UTF8ToWide(browser_id))->OnTabLoadStop(index);
}
