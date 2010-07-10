// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "include/cef2.h"
#include "cefclient.h"
#include "resource_util.h"
#include "string_util.h"
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

#define MAX_LOADSTRING 100
#define MAX_URL_LENGTH  255
#define BUTTON_WIDTH 72
#define URLBAR_HEIGHT  24

// Define this value to run CEF with messages processed using the current
// application's message loop.
#define TEST_SINGLE_THREADED_MESSAGE_LOOP

// Define this value to load Chromium in app mode.
//#define TEST_CHROMIUM_APP_MODE

// Define this value to output the Chromium command line to the console instead
// of launching the Chromium process.  This allows for easier debugging of the
// Chromium process.
//#define TEST_CHROMIUM_DEBUG_MODE

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);


// Program entry point function.
int APIENTRY wWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef TEST_SINGLE_THREADED_MESSAGE_LOOP
  // Initialize the CEF with messages processed using the current application's
  // message loop.
  CefInitialize(std::wstring(), false);
#else
  // Initialize the CEF with messages processed using a separate UI thread.
  CefInitialize(std::wstring(), true);
#endif

  MSG msg;
  HACCEL hAccelTable;

  // Initialize global strings
  LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
  LoadString(hInstance, IDC_CEFCLIENT, szWindowClass, MAX_LOADSTRING);
  MyRegisterClass(hInstance);

  // Perform application initialization
  if (!InitInstance (hInstance, nCmdShow))
  {
    return FALSE;
  }

  hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CEFCLIENT));

  // Main message loop
  while (GetMessage(&msg, NULL, 0, 0))
  {
#ifdef TEST_SINGLE_THREADED_MESSAGE_LOOP
    // Allow the CEF to do its message loop processing.
    CefDoMessageLoopWork();
#endif

    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  // Shut down the CEF
  CefShutdown();

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this
//    function so that the application will get 'well formed' small icons
//    associated with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CEFCLIENT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_CEFCLIENT);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle,
      WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, 0, CW_USEDEFAULT,
      0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

class Client : public CefThreadSafeBase<CefBase>
{
public:
  // Client BrowserListener implementation.
  class BrowserListener : public CefThreadSafeBase<CefBrowserListener>
  {
  public:
    BrowserListener(Client *pClient) : m_pClient(pClient) {}

    // Called when a browser is created.
    virtual void OnCreate(CefRefPtr<CefBrowser> browser)
    {
      Client::AutoLock lock_scope(m_pClient);
      if(m_pClient->m_BrowserHwnd == NULL)
      {
        // The main browser window has been created.
        m_pClient->m_Browser = browser;
        m_pClient->m_BrowserHwnd = browser->GetWindowHandle();
      }
    }

    // Called when a browser is destroyed.
    virtual void OnDestroy(CefRefPtr<CefBrowser> browser)
    {
      Client::AutoLock lock_scope(m_pClient);
      if(m_pClient->m_BrowserHwnd != NULL &&
         m_pClient->m_BrowserHwnd == browser->GetWindowHandle())
      {
        // The main browser window has been destroyed.
        m_pClient->m_Browser = NULL;
        m_pClient->m_BrowserHwnd = NULL;

        // Exit the application.
        DestroyWindow(m_pClient->m_MainHwnd);
      }
    }

  private:
    Client *m_pClient;
  };

  // Client TabListener implementation.
  class TabListener : public CefThreadSafeBase<CefTabListener>
  {
  public:
    TabListener(Client *pClient) : m_pClient(pClient) {}

    // Called when a tab is created.
    virtual void OnCreate(CefRefPtr<CefTab> tab)
    {
    }

    // Called when a tab is destroyed.
    virtual void OnDestroy(CefRefPtr<CefTab> tab)
    {
    }

    // Called when the address changes.
    virtual void OnAddressChange(CefRefPtr<CefTab> tab,
                                 const std::wstring& url)
    {
      CefRefPtr<CefBrowser> browser = tab->GetBrowser();
      if(m_pClient->m_BrowserHwnd == browser->GetWindowHandle()
        && tab->IsSelected())
      {
        // Update the address.
        m_pClient->SetAddress(url);
      }
    }

    // Called when the page title changes.
    virtual void OnTitleChange(CefRefPtr<CefTab> tab,
                               const std::wstring& title)
    {
      CefRefPtr<CefBrowser> browser = tab->GetBrowser();
      if(m_pClient->m_BrowserHwnd == browser->GetWindowHandle()
        && tab->IsSelected())
      {
        // Update the title.
        m_pClient->SetTitle(title);
      }
    }

    // Called when the navigation state changes.
    virtual void OnNavStateChange(CefRefPtr<CefTab> tab, bool can_go_back,
                                  bool can_go_forward)
    {
      CefRefPtr<CefBrowser> browser = tab->GetBrowser();
      if(m_pClient->m_BrowserHwnd == browser->GetWindowHandle()
        && tab->IsSelected())
      {
        // Update the nav state.
        m_pClient->SetNavState(can_go_back, can_go_forward);
      }
    }

    // Called when the selection state changes.
    virtual void OnSelectChange(CefRefPtr<CefTab> tab, bool selected)
    {
      CefRefPtr<CefBrowser> browser = tab->GetBrowser();
      if(m_pClient->m_BrowserHwnd == browser->GetWindowHandle() &&
         selected)
      {
        // Update everything.
        m_pClient->SetAddress(tab->GetAddress());
        m_pClient->SetTitle(tab->GetTitle());
        m_pClient->SetNavState(tab->CanGoBack(), tab->CanGoForward());
        m_pClient->SetLoading(tab->IsLoading());

        // Send a state change notification.
        m_pClient->NotifyStateChange();
      }
    }

    // Called when the page load has started.
    /*--cef()--*/
    virtual void OnLoadStart(CefRefPtr<CefTab> tab)
    {
      CefRefPtr<CefBrowser> browser = tab->GetBrowser();
      if(m_pClient->m_BrowserHwnd == browser->GetWindowHandle() &&
         tab->IsSelected())
      {
        // Update load state.
        m_pClient->SetLoading(true);
        
        // Send a state change notification.
        m_pClient->NotifyStateChange();
      }
    }

    // Called when the page load has stopped.
    /*--cef()--*/
    virtual void OnLoadStop(CefRefPtr<CefTab> tab)
    {
      CefRefPtr<CefBrowser> browser = tab->GetBrowser();
      if(m_pClient->m_BrowserHwnd == browser->GetWindowHandle() &&
         tab->IsSelected())
      {
        // Update load state.
        m_pClient->SetLoading(false);
        
        // Send a state change notification.
        m_pClient->NotifyStateChange();
      }
    }
  
  private:
    Client *m_pClient;
  };

  Client()
  {
    m_BrowserListener = new BrowserListener(this);
    m_TabListener = new TabListener(this);
    m_MainHwnd = NULL;
    m_BrowserHwnd = NULL;
    m_EditHwnd = NULL;
    m_bLoading = true;
    m_bCanGoBack = false;
    m_bCanGoForward = false;
  }

  CefRefPtr<CefBrowserListener> GetBrowserListener()
    { return m_BrowserListener; }
  CefRefPtr<CefTabListener> GetTabListener()
    { return m_TabListener; }

  void SetMainHwnd(HWND hwnd)
  {
    AutoLock lock_scope(this);
    m_MainHwnd = hwnd;
  }
  HWND GetMainHwnd() { return m_MainHwnd; }

  void SetEditHwnd(HWND hwnd)
  {
    AutoLock lock_scope(this);
    m_EditHwnd = hwnd;
  }

  // Set the edit window text.
  void SetAddress(const std::wstring& url)
  {
    SetWindowText(m_EditHwnd, url.c_str());
  }

  // Set the frame window title bar.
  void SetTitle(const std::wstring& title)
  {
    SetWindowText(m_MainHwnd, title.c_str());
  }

  // Set the navigation state.
  void SetNavState(bool can_go_back, bool can_go_forward)
  {
    AutoLock lock_scope(this);
    m_bCanGoBack = can_go_back;
    m_bCanGoForward = can_go_forward;
  }

  // Set the load state.
  void SetLoading(bool loading)
  {
    AutoLock lock_scope(this);
    m_bLoading = loading;
  }
  
  // Retrieve the state information.
  void GetState(bool& can_go_back, bool& can_go_forward, bool& loading)
  {
    AutoLock lock_scope(this);
    can_go_back = m_bCanGoBack;
    can_go_forward = m_bCanGoForward;
    loading = m_bLoading;
  }

  // Notify the application to update state information.
  void NotifyStateChange()
  {
    PostMessage(m_MainHwnd, WM_COMMAND, ID_STATE_CHANGE, 0);
  }

  CefRefPtr<CefBrowser> GetBrowser() { return m_Browser; }
  HWND GetBrowserHwnd() { return m_BrowserHwnd; }

protected:
  // Listeners for handling events
  CefRefPtr<CefBrowserListener> m_BrowserListener;
  CefRefPtr<CefTabListener> m_TabListener;

  // The child browser window
  CefRefPtr<CefBrowser> m_Browser;

  // The main frame window handle
  HWND m_MainHwnd;

  // The child browser window handle
  HWND m_BrowserHwnd;

  // The edit window handle
  HWND m_EditHwnd;

  // True if the page is currently loading
  bool m_bLoading;
  // True if the user can navigate backwards
  bool m_bCanGoBack;
  // True if the user can navigate forwards
  bool m_bCanGoForward;
};

// global handler instance
CefRefPtr<Client> g_client;

CefRefPtr<CefBrowser> AppGetBrowser()
{
  if(!g_client.get())
    return NULL;
  return g_client->GetBrowser();
}

HWND AppGetMainHwnd()
{
  if(!g_client.get())
    return NULL;
  return g_client->GetMainHwnd();
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  static HWND backWnd = NULL, forwardWnd = NULL, reloadWnd = NULL,
      stopWnd = NULL, editWnd = NULL;
  static WNDPROC editWndOldProc = NULL;

  int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

  if(hWnd == editWnd)
  {
    // Callback for the edit window
    switch (message)
    {
    case WM_CHAR:
		  if (wParam == VK_RETURN && g_client.get())
      {
        CefRefPtr<CefBrowser> browser = g_client->GetBrowser();
        // When the user hits the enter key load the URL
        wchar_t strPtr[MAX_URL_LENGTH] = {0};
        *((LPWORD)strPtr) = MAX_URL_LENGTH; 
        LRESULT strLen = SendMessage(hWnd, EM_GETLINE, 0, (LPARAM)strPtr);
        if (strLen > 0) {
          strPtr[strLen] = 0;
          browser->GetSelectedTab()->LoadAddress(strPtr, L"");
        }

        return 0;
      }
	  }
    return (LRESULT)CallWindowProc(editWndOldProc, hWnd, message, wParam,
        lParam);
  }
  else
  {
    // Callback for the main window
	  switch (message)
	  {
    case WM_CREATE:
      {
        // Create the single static handler class instance
        g_client = new Client();
        g_client->SetMainHwnd(hWnd);

        // Create the child windows used for navigation
        RECT rect;
        int x = 0;
        
        GetClientRect(hWnd, &rect);
        
        backWnd = CreateWindow(L"BUTTON", L"Back",
                               WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON
                               | WS_DISABLED, x, 0, BUTTON_WIDTH, URLBAR_HEIGHT,
                               hWnd, (HMENU) IDC_NAV_BACK, hInst, 0);
        x += BUTTON_WIDTH;

        forwardWnd = CreateWindow(L"BUTTON", L"Forward",
                                  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON
                                  | WS_DISABLED, x, 0, BUTTON_WIDTH,
                                  URLBAR_HEIGHT, hWnd, (HMENU) IDC_NAV_FORWARD,
                                  hInst, 0);
        x += BUTTON_WIDTH;

        reloadWnd = CreateWindow(L"BUTTON", L"Reload",
                                 WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON
                                 | WS_DISABLED, x, 0, BUTTON_WIDTH,
                                 URLBAR_HEIGHT, hWnd, (HMENU) IDC_NAV_RELOAD,
                                 hInst, 0);
        x += BUTTON_WIDTH;

        stopWnd = CreateWindow(L"BUTTON", L"Stop",
                               WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON
                               | WS_DISABLED, x, 0, BUTTON_WIDTH, URLBAR_HEIGHT,
                               hWnd, (HMENU) IDC_NAV_STOP, hInst, 0);
        x += BUTTON_WIDTH;

        editWnd = CreateWindow(L"EDIT", 0,
                               WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT |
                               ES_AUTOVSCROLL | ES_AUTOHSCROLL| WS_DISABLED, 
                               x, 0, rect.right - BUTTON_WIDTH * 4,
                               URLBAR_HEIGHT, hWnd, 0, hInst, 0);
        
        // Assign the edit window's WNDPROC to this function so that we can
        // capture the enter key
        editWndOldProc =
            reinterpret_cast<WNDPROC>(GetWindowLongPtr(editWnd, GWLP_WNDPROC));
        SetWindowLongPtr(editWnd, GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(WndProc)); 
        g_client->SetEditHwnd(editWnd);
        
        rect.top += URLBAR_HEIGHT;

        // Create the profile object.
        CefRefPtr<CefProfile> profile = CefCreateProfile(L"");

        // Register the client listeners.
        profile->AddBrowserListener(L"", g_client->GetBrowserListener());
        profile->AddTabListener(L"", g_client->GetTabListener());

        std::wstringstream command_line;
#ifdef TEST_CHROMIUM_APP_MODE
        command_line << L"--app=http://www.google.com ";
#endif
#ifdef TEST_CHROMIUM_DEBUG_MODE
        command_line << L"--debug-cef-chrome ";
#endif

        // Create the browser object.
        CefRefPtr<CefBrowser> browser = profile->CreateBrowser(L"",
            hWnd, rect.left, rect.top, rect.right - rect.left,
            rect.bottom - rect.top, command_line.str());
      }
      return 0;

    case WM_COMMAND:
      {
        CefRefPtr<CefBrowser> browser;
        if(g_client.get())
          browser = g_client->GetBrowser();

		    wmId    = LOWORD(wParam);
		    wmEvent = HIWORD(wParam);
		    // Parse the menu selections:
		    switch (wmId)
		    {
		    case IDM_ABOUT:
          DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			    return 0;
		    case IDM_EXIT:
			    DestroyWindow(hWnd);
			    return 0;
        case ID_STATE_CHANGE: // Update state information
          {
            // Retrieve the current state
            bool can_go_back, can_go_forward, loading;
            g_client->GetState(can_go_back, can_go_forward, loading);

            // Update the status of child windows
            EnableWindow(editWnd, TRUE);
            EnableWindow(backWnd, can_go_back);
            EnableWindow(forwardWnd, can_go_forward);
            EnableWindow(reloadWnd, !loading);
            EnableWindow(stopWnd, loading);
          }
          return 0;
        case IDC_NAV_BACK:  // Back button
          if(browser.get())
            browser->GetSelectedTab()->GoBack();
          return 0;
        case IDC_NAV_FORWARD: // Forward button
          if(browser.get())
            browser->GetSelectedTab()->GoForward();
          return 0;
        case IDC_NAV_RELOAD:  // Reload button
          if(browser.get())
            browser->GetSelectedTab()->Reload();
          return 0;
        case IDC_NAV_STOP:  // Stop button
		     if(browser.get())
            browser->GetSelectedTab()->StopLoad();
          return 0;
        case ID_TESTS_JAVASCRIPT_EXECUTE: // Test javascript execution
          if(browser.get())
          {
            browser->GetSelectedTab()->ExecuteJavaScript(L"",
                L"alert('JavaScript execute works!');");
          }
          return 0;
        case ID_TESTS_JAVASCRIPT_WINDOW: // Test javascript window creation
          if(browser.get())
          {
            browser->GetSelectedTab()->ExecuteJavaScript(L"",
                L"window.open('http://www.google.com');");
          }
          return 0;
        case ID_TESTS_CREATE_TAB: // Test new tab creation
          if(browser.get())
          {
            browser->CreateTab(browser->GetTabCount(), L"http://www.google.com",
                L"", true);
          }
          return 0;
        case ID_TESTS_SELECT_TAB: // Test tab selection
          if(browser.get())
            browser->GetTab(0)->Select();
          return 0;
        case ID_TESTS_DESTROY_TAB: // Test tab destruction
          if(browser.get())
            browser->GetTab(0)->Destroy();
          return 0;
        case ID_TESTS_PRINT: // Test printing
          if(browser.get())
            browser->GetSelectedTab()->Print();
          return 0;
        case ID_TESTS_VIEW_SOURCE: // Test view source
          if(browser.get())
            browser->GetSelectedTab()->ViewSource();
          return 0;
        }
      }
		  break;

	  case WM_PAINT:
		  hdc = BeginPaint(hWnd, &ps);
		  EndPaint(hWnd, &ps);
		  return 0;

    case WM_SETFOCUS:
      if(g_client.get() && g_client->GetBrowserHwnd())
      {
        // Pass focus to the browser window
        PostMessage(g_client->GetBrowserHwnd(), WM_SETFOCUS, wParam, NULL);
      }
      return 0;

    case WM_SIZE:
      if(g_client.get() && g_client->GetBrowserHwnd())
      {
        // Resize the browser window and address bar to match the new frame
        // window size
        RECT rect;
        GetClientRect(hWnd, &rect);
        rect.top += URLBAR_HEIGHT;

        int urloffset = rect.left + BUTTON_WIDTH * 4;

        HDWP hdwp = BeginDeferWindowPos(1);
        hdwp = DeferWindowPos(hdwp, editWnd, NULL, urloffset,
            0, rect.right - urloffset, URLBAR_HEIGHT, SWP_NOZORDER);
        hdwp = DeferWindowPos(hdwp, g_client->GetBrowserHwnd(), NULL,
            rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
            SWP_NOZORDER);
        EndDeferWindowPos(hdwp);
      }
      break;

    case WM_ERASEBKGND:
      if(g_client.get() && g_client->GetBrowserHwnd())
      {
        // Dont erase the background if the browser window has been loaded
        // (this avoids flashing)
		    return 0;
      }
      break;
    
	  case WM_DESTROY:
      // The frame window has exited
      PostQuitMessage(0);
		  return 0;
    }
  	
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
