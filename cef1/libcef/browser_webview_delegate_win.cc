// Copyright (c) 2011 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file contains the implementation of BrowserWebViewDelegate, which serves
// as the WebViewDelegate for the BrowserWebHost.  The host is expected to
// have initialized a MessageLoop before these methods are called.

#include "libcef/browser_webview_delegate.h"

#include <objidl.h>
#include <shlobj.h>
#include <shlwapi.h>

#include "libcef/browser_drag_delegate_win.h"
#include "libcef/browser_navigation_controller.h"
#include "libcef/browser_impl.h"
#include "libcef/cef_context.h"
#include "libcef/drag_data_impl.h"
#include "libcef/web_drop_target_win.h"
#include "libcef_dll/resource.h"

#include "base/i18n/case_conversion.h"
#include "base/message_loop.h"
#include "base/path_service.h"
#include "base/string_util.h"
#include "base/utf_string_conversions.h"
#include "base/win/registry.h"
#include "net/base/mime_util.h"
#include "net/base/net_errors.h"
#include "third_party/WebKit/Source/Platform/chromium/public/WebDragData.h"
#include "third_party/WebKit/Source/Platform/chromium/public/WebImage.h"
#include "third_party/WebKit/Source/Platform/chromium/public/WebPoint.h"
#include "third_party/WebKit/Source/Platform/chromium/public/WebRect.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebContextMenuData.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebCursorInfo.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebDocument.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebView.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/window_open_disposition.h"
#include "ui/gfx/gdi_util.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/point.h"
#include "webkit/glue/webdropdata.h"
#include "webkit/glue/webpreferences.h"
#include "webkit/glue/webkit_glue.h"
#include "webkit/plugins/npapi/plugin_list.h"
#include "webkit/plugins/npapi/webplugin.h"
#include "webkit/plugins/npapi/webplugin_delegate_impl.h"

using webkit::npapi::WebPluginDelegateImpl;
using WebKit::WebContextMenuData;
using WebKit::WebCursorInfo;
using WebKit::WebDragData;
using WebKit::WebDragOperationsMask;
using WebKit::WebExternalPopupMenu;
using WebKit::WebExternalPopupMenuClient;
using WebKit::WebFrame;
using WebKit::WebImage;
using WebKit::WebNavigationPolicy;
using WebKit::WebPoint;
using WebKit::WebPopupMenuInfo;
using WebKit::WebRect;
using WebKit::WebWidget;

namespace {

static const wchar_t kPluginWindowClassName[] = L"WebPluginHost";

// From ui/base/dialogs/select_file_dialog_win.cc.

// Get the file type description from the registry. This will be "Text Document"
// for .txt files, "JPEG Image" for .jpg files, etc. If the registry doesn't
// have an entry for the file type, we return false, true if the description was
// found. 'file_ext' must be in form ".txt".
static bool GetRegistryDescriptionFromExtension(const std::wstring& file_ext,
                                                std::wstring* reg_description) {
  DCHECK(reg_description);
  base::win::RegKey reg_ext(HKEY_CLASSES_ROOT, file_ext.c_str(), KEY_READ);
  std::wstring reg_app;
  if (reg_ext.ReadValue(NULL, &reg_app) == ERROR_SUCCESS && !reg_app.empty()) {
    base::win::RegKey reg_link(HKEY_CLASSES_ROOT, reg_app.c_str(), KEY_READ);
    if (reg_link.ReadValue(NULL, reg_description) == ERROR_SUCCESS)
      return true;
  }
  return false;
}

// Set up a filter for a Save/Open dialog, which will consist of |file_ext| file
// extensions (internally separated by semicolons), |ext_desc| as the text
// descriptions of the |file_ext| types (optional), and (optionally) the default
// 'All Files' view. The purpose of the filter is to show only files of a
// particular type in a Windows Save/Open dialog box. The resulting filter is
// returned. The filters created here are:
//   1. only files that have 'file_ext' as their extension
//   2. all files (only added if 'include_all_files' is true)
// Example:
//   file_ext: { "*.txt", "*.htm;*.html" }
//   ext_desc: { "Text Document" }
//   returned: "Text Document\0*.txt\0HTML Document\0*.htm;*.html\0"
//             "All Files\0*.*\0\0" (in one big string)
// If a description is not provided for a file extension, it will be retrieved
// from the registry. If the file extension does not exist in the registry, it
// will be omitted from the filter, as it is likely a bogus extension.
std::wstring FormatFilterForExtensions(
    const std::vector<std::wstring>& file_ext,
    const std::vector<std::wstring>& ext_desc,
    bool include_all_files) {
  const std::wstring all_ext = L"*.*";
  const std::wstring all_desc =
      std::wstring(L"All Files") +
      L" (" + all_ext + L")";

  DCHECK(file_ext.size() >= ext_desc.size());

  if (file_ext.empty())
    include_all_files = true;

  std::wstring result;

  if (file_ext.size() > 1) {
     std::wstring extensions = JoinString(file_ext, L';');
     std::wstring all_supported_types = L"All Supported Types (" +
         extensions + L")";

     result.append(all_supported_types.c_str(), all_supported_types.size() + 1);
     result.append(extensions.c_str(), extensions.size() + 1);
  }

  for (size_t i = 0; i < file_ext.size(); ++i) {
    std::wstring ext = file_ext[i];
    std::wstring desc;
    if (i < ext_desc.size())
      desc = ext_desc[i];

    if (ext.empty()) {
      // Force something reasonable to appear in the dialog box if there is no
      // extension provided.
      include_all_files = true;
      continue;
    }

    if (desc.empty()) {
      DCHECK(ext.find(L'.') != std::wstring::npos);
      std::wstring first_extension = ext.substr(ext.find(L'.'));
      size_t first_separator_index = first_extension.find(L';');
      if (first_separator_index != std::wstring::npos)
        first_extension = first_extension.substr(0, first_separator_index);

      // Find the extension name without the preceeding '.' character.
      std::wstring ext_name = first_extension;
      size_t ext_index = ext_name.find_first_not_of(L'.');
      if (ext_index != std::wstring::npos)
        ext_name = ext_name.substr(ext_index);

      if (!GetRegistryDescriptionFromExtension(first_extension, &desc)) {
        // The extension doesn't exist in the registry. Create a description
        // based on the unknown extension type (i.e. if the extension is .qqq,
        // the we create a description "QQQ File (.qqq)").
        include_all_files = true;
        std::vector<string16> replacements;
        replacements.push_back(base::i18n::ToUpper(WideToUTF16(ext_name)));
        replacements.push_back(ext_name);
        desc = ReplaceStringPlaceholders(
            L"$1 File (.$2)",
            replacements,
            NULL);
      }
    }

    if (!desc.empty())
      desc += L" (" + ext + L")";
    else
      desc = ext;

    result.append(desc.c_str(), desc.size() + 1);  // Append NULL too.
    result.append(ext.c_str(), ext.size() + 1);
  }

  if (include_all_files) {
    result.append(all_desc.c_str(), all_desc.size() + 1);
    result.append(all_ext.c_str(), all_ext.size() + 1);
  }

  result.append(1, '\0');  // Double NULL required.
  return result;
}

std::wstring GetDescriptionFromMimeType(const std::string& mime_type) {
  // Check for wild card mime types and return an appropriate description.
  static const struct {
    const char* mime_type;
    std::wstring full_string;
  } kWildCardMimeTypes[] = {
    { "audio", L"Audio Files" },
    { "image", L"Image Files" },
    { "text", L"Text Files" },
    { "video", L"Video Files" },
  };

  for (size_t i = 0; i < arraysize(kWildCardMimeTypes); ++i) {
    if (mime_type == std::string(kWildCardMimeTypes[i].mime_type) + "/*")
      return std::wstring(kWildCardMimeTypes[i].full_string);
  }

  return std::wstring();
}

std::wstring GetFilterStringFromAcceptTypes(
    const std::vector<std::string>& accept_types) {
    std::vector<std::wstring> extensions;
    std::vector<std::wstring> descriptions;

  for (size_t i = 0; i < accept_types.size(); ++i) {
    std::string ascii_type = accept_types[i];
    if (ascii_type.length()) {
      // Just treat as extension if contains '.' as the first character.
      if (ascii_type[0] == '.') {
        extensions.push_back(L"*" + ASCIIToWide(ascii_type));
        descriptions.push_back(std::wstring());
      } else {
        // Otherwise convert mime type to one or more extensions.
        std::vector<base::FilePath::StringType> ext;
        std::wstring ext_str;
        net::GetExtensionsForMimeType(ascii_type, &ext);
        if (ext.size() > 0) {
          for (size_t x = 0; x < ext.size(); ++x) {
            if (x != 0)
              ext_str += L";";
            ext_str += L"*." + ext[x];
          }
          extensions.push_back(ext_str);
          descriptions.push_back(GetDescriptionFromMimeType(ascii_type));
        }
      }
    }
  }

  return FormatFilterForExtensions(extensions, descriptions, true);
}

void AddMenuItem(CefRefPtr<CefBrowser> browser,
                 CefRefPtr<CefMenuHandler> handler,
                 HMENU menu,
                 cef_menu_id_t menuId,
                 const wchar_t* label,
                 bool enabled,
                 std::list<std::wstring>& label_list) {
  CefString actual_label(label);
  if (handler.get()) {
    // Let the handler change the label if desired,
    handler->GetMenuLabel(browser, menuId, actual_label);
  }

  // Store the label in a list to simplify memory management.
  label_list.push_back(actual_label);

  MENUITEMINFO mii;
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STRING;
  mii.fType = MFT_STRING;
  if (!enabled) {
    mii.fMask |= MIIM_STATE;
    mii.fState = MFS_GRAYED;
  }
  mii.wID = menuId;
  mii.dwTypeData = const_cast<wchar_t*>(label_list.back().c_str());

  InsertMenuItem(menu, -1, TRUE, &mii);
}

void AddMenuSeparator(HMENU menu) {
  MENUITEMINFO mii;
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_FTYPE;
  mii.fType = MFT_SEPARATOR;

  InsertMenuItem(menu, -1, TRUE, &mii);
}

std::wstring GetDialogLabel(WebKit::WebFrame* webframe,
                            const std::string& label) {
  const GURL& url = webframe->document().url();
  std::string urlStr;
  if (!url.is_empty())
    urlStr = url.host();
  std::string labelStr = label;
  if (!urlStr.empty())
    labelStr += " - " + urlStr;
  return UTF8ToWide(labelStr);
}

struct DialogConfig {
  std::wstring label;
  std::wstring message;
  std::wstring default_value;
  std::wstring result;
};

INT_PTR CALLBACK DialogProc(HWND dialog,
                            UINT message,
                            WPARAM wparam,
                            LPARAM lparam) {
  switch (message) {
    case WM_INITDIALOG: {
      SetWindowLongPtr(dialog, DWL_USER, static_cast<LONG_PTR>(lparam));
      DialogConfig* config = reinterpret_cast<DialogConfig*>(lparam);
      SetWindowText(dialog, config->label.c_str());
      SetDlgItemText(dialog, IDC_DIALOGTEXT, config->message.c_str());
      SetDlgItemText(dialog, IDC_PROMPTEDIT, config->default_value.c_str());
      break;
    }
    case WM_CLOSE: {
      DialogConfig* config = reinterpret_cast<DialogConfig*>(
          GetWindowLongPtr(dialog, DWL_USER));
      if (config)
        EndDialog(dialog, IDCANCEL);
      break;
    }
    case WM_COMMAND: {
      DialogConfig* config = reinterpret_cast<DialogConfig*>(
          GetWindowLongPtr(dialog, DWL_USER));
      bool finish = false;
      switch (LOWORD(wparam)) {
        case IDOK: {
          finish = true;
          size_t length =
              GetWindowTextLength(GetDlgItem(dialog, IDC_PROMPTEDIT)) + 1;
          if (length > 1) {
            GetDlgItemText(dialog, IDC_PROMPTEDIT,
                           WriteInto(&config->result, length), length);
          }
          break;
        }
        case IDCANCEL:
          finish = true;
          break;
      }
      if (finish)
        EndDialog(dialog, LOWORD(wparam));
      break;
    }
  }

  return DefWindowProc(dialog, message, wparam, lparam);
}

}  // namespace

// WebViewClient --------------------------------------------------------------

WebExternalPopupMenu* BrowserWebViewDelegate::createExternalPopupMenu(
    const WebPopupMenuInfo& info,
    WebExternalPopupMenuClient* client)  {
  NOTREACHED();
  return NULL;
}

// WebWidgetClient ------------------------------------------------------------

void BrowserWebViewDelegate::show(WebNavigationPolicy) {
  if (this == browser_->UIT_GetWebViewDelegate()) {
    if (!browser_->IsWindowRenderingDisabled()) {
      // Restore the window and bring it to the top if the window is currently
      // visible.
      HWND root = GetAncestor(browser_->UIT_GetMainWndHandle(), GA_ROOT);
      if (IsWindowVisible(root)) {
        ShowWindow(root, SW_SHOWNORMAL);
        SetWindowPos(root, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
      }
    }
  } else if (this == browser_->UIT_GetPopupDelegate()) {
    if (!browser_->IsWindowRenderingDisabled()) {
      // Show popup widgets without activation.
      ShowWindow(browser_->UIT_GetPopupWndHandle(), SW_SHOWNA);
    } else {
      // Notify the handler of popup visibility change.
      CefRefPtr<CefClient> client = browser_->GetClient();
      if (client.get()) {
        CefRefPtr<CefRenderHandler> handler = client->GetRenderHandler();
        if (handler.get())
          handler->OnPopupShow(browser_, true);
      }
    }
  }
}

void BrowserWebViewDelegate::didChangeCursor(const WebCursorInfo& cursor_info) {
  if (WebWidgetHost* host = GetWidgetHost()) {
    current_cursor_.InitFromCursorInfo(cursor_info);
    HMODULE hModule = ::GetModuleHandle(L"libcef.dll");
    if (!hModule)
      hModule = ::GetModuleHandle(NULL);
    HCURSOR hCursor = current_cursor_.GetCursor(hModule);

    if (!browser_->IsWindowRenderingDisabled()) {
      host->SetCursor(hCursor);
    } else {
      // Notify the handler of cursor change.
      CefRefPtr<CefClient> client = browser_->GetClient();
      if (client.get()) {
        CefRefPtr<CefRenderHandler> handler = client->GetRenderHandler();
        if (handler.get())
          handler->OnCursorChange(browser_, hCursor);
      }
    }
  }
}

WebRect BrowserWebViewDelegate::windowRect() {
  if (WebWidgetHost* host = GetWidgetHost()) {
    if (!browser_->IsWindowRenderingDisabled()) {
      RECT rect;
      ::GetWindowRect(host->view_handle(), &rect);
      return gfx::Rect(rect);
    } else {
      // Retrieve the view rectangle from the handler.
      CefRefPtr<CefClient> client = browser_->GetClient();
      if (client.get()) {
        CefRefPtr<CefRenderHandler> handler = client->GetRenderHandler();
        if (handler.get()) {
          CefRect rect(0, 0, 0, 0);
          if (handler->GetViewRect(browser_, rect))
            return WebRect(rect.x, rect.y, rect.width, rect.height);
        }
      }
    }
  }
  return WebRect();
}

void BrowserWebViewDelegate::setWindowRect(const WebRect& rect) {
  if (this == browser_->UIT_GetWebViewDelegate()) {
    // ignored
  } else if (this == browser_->UIT_GetPopupDelegate()) {
    if (!browser_->IsWindowRenderingDisabled()) {
      MoveWindow(browser_->UIT_GetPopupWndHandle(), rect.x, rect.y, rect.width,
          rect.height, FALSE);
    } else {
      browser_->set_popup_rect(rect);
      browser_->UIT_GetPopupHost()->SetSize(rect.width, rect.height);

      // Notify the handler of popup size change.
      CefRefPtr<CefClient> client = browser_->GetClient();
      if (client.get()) {
        CefRefPtr<CefRenderHandler> handler = client->GetRenderHandler();
        if (handler.get()) {
          handler->OnPopupSize(browser_,
              CefRect(rect.x, rect.y, rect.width, rect.height));
        }
      }
    }
  }
}

WebRect BrowserWebViewDelegate::rootWindowRect() {
  if (WebWidgetHost* host = GetWidgetHost()) {
    RECT rect;
    HWND root_window = ::GetAncestor(host->view_handle(), GA_ROOT);
    ::GetWindowRect(root_window, &rect);
    return gfx::Rect(rect);
  }
  return WebRect();
}

WebRect BrowserWebViewDelegate::windowResizerRect() {
  // Not necessary on Windows.
  return WebRect();
}

void BrowserWebViewDelegate::startDragging(
    WebFrame* frame,
    const WebDragData& data,
    WebDragOperationsMask mask,
    const WebImage& image,
    const WebPoint& image_offset) {
  // Dragging is not supported when window rendering is disabled.
  if (browser_->settings().drag_drop_disabled ||
      browser_->IsWindowRenderingDisabled()) {
    EndDragging();
    return;
  }

  WebDropData drop_data(data);

  CefRefPtr<CefClient> client = browser_->GetClient();
  if (client.get()) {
    CefRefPtr<CefDragHandler> handler = client->GetDragHandler();
    if (handler.get()) {
      CefRefPtr<CefDragData> data(new CefDragDataImpl(drop_data));
      if (handler->OnDragStart(browser_, data,
              static_cast<CefDragHandler::DragOperationsMask>(mask))) {
        EndDragging();
        return;
      }
    }
  }

  drag_delegate_ = new BrowserDragDelegate(this);
  gfx::ImageSkia image_skia =
      gfx::ImageSkia(gfx::ImageSkiaRep(image.getSkBitmap(),
                                       ui::SCALE_FACTOR_100P));
  drag_delegate_->StartDragging(drop_data, mask, image_skia,
                                gfx::Point(image_offset).OffsetFromOrigin());
}

void BrowserWebViewDelegate::runModal() {
  WebWidgetHost* host = GetWidgetHost();
  if (!host)
    return;

  show(WebKit::WebNavigationPolicyNewWindow);

  browser_->UIT_SetIsModal(true);

  CefRefPtr<CefClient> client = browser_->GetClient();
  CefRefPtr<CefLifeSpanHandler> handler;

  if (client.get())
    handler = client->GetLifeSpanHandler();

  bool handled(false);

  if (handler.get()) {
    // Let the client override the modal message loop.
    handled = handler->RunModal(browser_);
  }

  if (!handled) {
    HWND child = ::GetAncestor(browser_->UIT_GetMainWndHandle(), GA_ROOT);
    HWND owner = ::GetAncestor(browser_->opener_window(), GA_ROOT);

    if (child && owner) {
      // Set the owner so that Windows keeps this window above the owner.
      ::SetWindowLong(child, GWL_HWNDPARENT, (LONG)owner);
      // Disable the owner if it is enabled so that you can't interact with it.
      // while this child window is open.
      if (::IsWindowEnabled(owner)) {
        ::EnableWindow(owner, FALSE);
        browser_->set_opener_was_disabled_by_modal_loop(true);
      }
      DWORD dwStyle = ::GetWindowLong(child, GWL_STYLE);
      DWORD dwNewStyle = dwStyle | WS_POPUP;
      if (dwStyle != dwNewStyle)
        ::SetWindowLong(child, GWL_STYLE, dwNewStyle);
    }

    // Tell the browser to exit this message loop when this window closes.
    browser_->set_internal_modal_message_loop_is_active(true);

    // Start a new message loop here and return when this window closes.
    MessageLoop* message_loop = MessageLoop::current();
    bool old_state = message_loop->NestableTasksAllowed();
    message_loop->SetNestableTasksAllowed(true);
    message_loop->Run();
    message_loop->SetNestableTasksAllowed(old_state);
  }
}

// WebPluginPageDelegate ------------------------------------------------------

webkit::npapi::WebPluginDelegate* BrowserWebViewDelegate::CreatePluginDelegate(
      const base::FilePath& file_path,
      const std::string& mime_type) {
  WebViewHost* host = browser_->UIT_GetWebViewHost();
  if (!host)
    return NULL;

  return WebPluginDelegateImpl::Create(file_path, mime_type);
}

void BrowserWebViewDelegate::CreatedPluginWindow(
    gfx::PluginWindowHandle handle) {
  if (!browser_->IsWindowRenderingDisabled()) {
    // Parent the plugin container to the existing browser window.
    HWND parent = browser_->UIT_GetWebViewHost()->view_handle();
    DCHECK(parent != NULL);
    SetParent(handle, parent);
  } else {
    static bool registered_class = false;
    if (!registered_class) {
      WNDCLASSEX wcex = {0};
      wcex.cbSize        = sizeof(wcex);
      wcex.style         = CS_DBLCLKS;
      wcex.lpfnWndProc   = DefWindowProc;
      wcex.hInstance     = GetModuleHandle(NULL);
      wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
      wcex.lpszClassName = kPluginWindowClassName;
      RegisterClassEx(&wcex);
      registered_class = true;
    }

    // Parent windowed plugin containers to a hidden window.
    HWND parent = CreateWindow(kPluginWindowClassName, NULL,
                               WS_OVERLAPPED|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
                               0, 0, 0, 0, NULL, NULL,
                               GetModuleHandle(NULL), NULL);
    DCHECK(parent != NULL);
    SetParent(handle, parent);

    WebViewHost* host = browser_->UIT_GetWebViewHost();
    if (host)
      host->AddWindowedPlugin(handle);
  }
}

void BrowserWebViewDelegate::WillDestroyPluginWindow(
    gfx::PluginWindowHandle handle) {
  if (browser_->IsWindowRenderingDisabled()) {
    WebViewHost* host = browser_->UIT_GetWebViewHost();
    if (host)
      host->RemoveWindowedPlugin(handle);

    // Destroy the hidden parent window.
    DestroyWindow(GetParent(handle));
  }
}

void BrowserWebViewDelegate::DidMovePlugin(
    const webkit::npapi::WebPluginGeometry& move) {
  UINT flags = 0;

  if (move.rects_valid) {
    HRGN hrgn = ::CreateRectRgn(move.clip_rect.x(),
                                move.clip_rect.y(),
                                move.clip_rect.right(),
                                move.clip_rect.bottom());
    gfx::SubtractRectanglesFromRegion(hrgn, move.cutout_rects);

    // Note: System will own the hrgn after we call SetWindowRgn,
    // so we don't need to call DeleteObject(hrgn)
    ::SetWindowRgn(move.window, hrgn, FALSE);
  } else {
    flags |= (SWP_NOSIZE | SWP_NOMOVE);
  }

  if (move.visible)
    flags |= SWP_SHOWWINDOW;
  else
    flags |= SWP_HIDEWINDOW;

  ::SetWindowPos(move.window,
                 NULL,
                 move.window_rect.x(),
                 move.window_rect.y(),
                 move.window_rect.width(),
                 move.window_rect.height(),
                 flags);

  if (browser_->IsWindowRenderingDisabled()) {
    WebViewHost* host = browser_->UIT_GetWebViewHost();
    if (host) {
      host->MoveWindowedPlugin(move);
    }
  }
}

void BrowserWebViewDelegate::showContextMenu(
    WebFrame* frame, const WebContextMenuData& data) {
  int screenX = -1, screenY = -1;

  POINT mouse_pt = {data.mousePosition.x, data.mousePosition.y};
  if (!browser_->IsWindowRenderingDisabled()) {
    // Perform the conversion to screen coordinates only if window rendering is
    // enabled.
    MapWindowPoints(browser_->UIT_GetWebViewWndHandle(), HWND_DESKTOP,
        &mouse_pt, 1);
    screenX = mouse_pt.x;
    screenY = mouse_pt.y;
  }

  int edit_flags = 0;
  int type_flags = 0;
  HMENU menu = NULL;
  std::list<std::wstring> label_list;

  // Make sure events can be pumped while the menu is up.
  MessageLoop::ScopedNestableTaskAllower allow(MessageLoop::current());

  // Give the client a chance to handle the menu.
  if (OnBeforeMenu(data, mouse_pt.x, mouse_pt.y, edit_flags, type_flags))
    return;

  CefRefPtr<CefClient> client = browser_->GetClient();
  CefRefPtr<CefMenuHandler> handler;
  if (client.get())
    handler = client->GetMenuHandler();

  if (client.get() && browser_->IsWindowRenderingDisabled()) {
    // Retrieve the screen coordinates.
    CefRefPtr<CefRenderHandler> render_handler = client->GetRenderHandler();
    if (!render_handler.get() ||
        !render_handler->GetScreenPoint(browser_, mouse_pt.x, mouse_pt.y,
                                        screenX, screenY)) {
      return;
    }
  }

  // Build the correct default context menu
  if (type_flags &  MENUTYPE_EDITABLE) {
    menu = CreatePopupMenu();
    AddMenuItem(browser_, handler, menu, MENU_ID_UNDO, L"Undo",
      !!(edit_flags & MENU_CAN_UNDO), label_list);
    AddMenuItem(browser_, handler, menu, MENU_ID_REDO, L"Redo",
      !!(edit_flags & MENU_CAN_REDO), label_list);
    AddMenuSeparator(menu);
    AddMenuItem(browser_, handler, menu, MENU_ID_CUT, L"Cut",
      !!(edit_flags & MENU_CAN_CUT), label_list);
    AddMenuItem(browser_, handler, menu, MENU_ID_COPY, L"Copy",
      !!(edit_flags & MENU_CAN_COPY), label_list);
    AddMenuItem(browser_, handler, menu, MENU_ID_PASTE, L"Paste",
      !!(edit_flags & MENU_CAN_PASTE), label_list);
    AddMenuItem(browser_, handler, menu, MENU_ID_DELETE, L"Delete",
      !!(edit_flags & MENU_CAN_DELETE), label_list);
    AddMenuSeparator(menu);
    AddMenuItem(browser_, handler, menu, MENU_ID_SELECTALL, L"Select All",
      !!(edit_flags & MENU_CAN_SELECT_ALL), label_list);
  } else if (type_flags & MENUTYPE_SELECTION) {
    menu = CreatePopupMenu();
    AddMenuItem(browser_, handler, menu, MENU_ID_COPY, L"Copy",
      !!(edit_flags & MENU_CAN_COPY), label_list);
  } else if (type_flags & (MENUTYPE_PAGE | MENUTYPE_FRAME)) {
    menu = CreatePopupMenu();
    AddMenuItem(browser_, handler, menu, MENU_ID_NAV_BACK, L"Back",
      !!(edit_flags & MENU_CAN_GO_BACK), label_list);
    AddMenuItem(browser_, handler, menu, MENU_ID_NAV_FORWARD, L"Forward",
      !!(edit_flags & MENU_CAN_GO_FORWARD), label_list);
    AddMenuSeparator(menu);
    AddMenuItem(browser_, handler, menu, MENU_ID_PRINT, L"Print",
      true, label_list);
    AddMenuItem(browser_, handler, menu, MENU_ID_VIEWSOURCE, L"View Source",
      true, label_list);
  }

  if (!menu)
    return;

  MessageLoop::current()->set_os_modal_loop(true);

  // Show the context menu
  int selected_id = TrackPopupMenu(menu,
      TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_RECURSE,
      screenX, screenY, 0, browser_->UIT_GetMainWndHandle(), NULL);

  MessageLoop::current()->set_os_modal_loop(false);

  if (selected_id != 0) {
    // An action was chosen
    cef_menu_id_t menuId = static_cast<cef_menu_id_t>(selected_id);
    bool handled = false;
    if (handler.get()) {
      // Ask the handler if it wants to handle the action
      handled = handler->OnMenuAction(browser_, menuId);
    }

    if (!handled) {
      // Execute the action
      browser_->UIT_HandleAction(menuId, browser_->GetFocusedFrame());
    }
  }

  DestroyMenu(menu);
}

// Private methods ------------------------------------------------------------

void BrowserWebViewDelegate::RegisterDragDrop() {
  DCHECK(!drop_target_);
  drop_target_ = new WebDropTarget(browser_);
}

void BrowserWebViewDelegate::RevokeDragDrop() {
  if (drop_target_.get())
    ::RevokeDragDrop(browser_->UIT_GetWebViewWndHandle());
}

void BrowserWebViewDelegate::EndDragging() {
  if (browser_->UIT_GetWebView())
    browser_->UIT_GetWebView()->dragSourceSystemDragEnded();
  drag_delegate_ = NULL;

  if (destroy_on_drag_end_)
    browser_->UIT_DestroyBrowser();
}

void BrowserWebViewDelegate::ShowJavaScriptAlert(WebFrame* webframe,
                                                 const CefString& message) {
  std::wstring messageStr = message;
  std::wstring titleStr = GetDialogLabel(webframe, "JavaScript Alert");
  MessageBox(browser_->UIT_GetMainWndHandle(), messageStr.c_str(),
      titleStr.c_str(), MB_OK | MB_ICONWARNING);
}

bool BrowserWebViewDelegate::ShowJavaScriptConfirm(WebFrame* webframe,
                                                   const CefString& message) {
  std::wstring messageStr = message;
  std::wstring titleStr = GetDialogLabel(webframe, "JavaScript Confirm");
  int rv = MessageBox(browser_->UIT_GetMainWndHandle(), messageStr.c_str(),
      titleStr.c_str(), MB_YESNO | MB_ICONQUESTION);
  return (rv == IDYES);
}

bool BrowserWebViewDelegate::ShowJavaScriptPrompt(WebFrame* webframe,
                                                  const CefString& message,
                                                  const CefString& default_value,
                                                  CefString* result) {
  base::FilePath file_path;
  HMODULE hModule = NULL;

  // Try to load the dialog from the DLL.
  if (PathService::Get(base::FILE_MODULE, &file_path))
    hModule = ::GetModuleHandle(file_path.value().c_str());
  if (!hModule)
    hModule = ::GetModuleHandle(NULL);
  DCHECK(hModule);

  DialogConfig config;
  config.label = GetDialogLabel(webframe, "JavaScript Prompt");
  config.message = message;
  config.default_value = default_value;

  HWND parent = GetAncestor(browser_->GetWindowHandle(), GA_ROOT);

  // Show the modal dialog.
  int rv = DialogBoxParam(hModule,
                          MAKEINTRESOURCE(IDD_PROMPT),
                          parent,
                          DialogProc,
                          reinterpret_cast<LPARAM>(&config));
  if (rv == IDOK)
    *result = config.result;

  // Return focus to the parent window.
  if (IsWindow(parent))
    SetFocus(parent);

  return (rv == IDOK);
}

namespace {

// from chrome/browser/views/shell_dialogs_win.cc

bool RunOpenFileDialog(const std::wstring& filter, HWND owner,
                       base::FilePath* path) {
  OPENFILENAME ofn;

  // We must do this otherwise the ofn's FlagsEx may be initialized to random
  // junk in release builds which can cause the Places Bar not to show up!
  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = owner;

  wchar_t filename[MAX_PATH];
  base::wcslcpy(filename, path->value().c_str(), arraysize(filename));

  ofn.lpstrFile = filename;
  ofn.nMaxFile = MAX_PATH;

  // We use OFN_NOCHANGEDIR so that the user can rename or delete the directory
  // without having to close Chrome first.
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

  if (!filter.empty()) {
    ofn.lpstrFilter = filter.c_str();
  }
  bool success = !!GetOpenFileName(&ofn);
  if (success)
    *path = base::FilePath(filename);
  return success;
}

bool RunOpenMultiFileDialog(const std::wstring& filter, HWND owner,
                            std::vector<base::FilePath>* paths) {
  OPENFILENAME ofn;

  // We must do this otherwise the ofn's FlagsEx may be initialized to random
  // junk in release builds which can cause the Places Bar not to show up!
  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = owner;

  scoped_array<wchar_t> filename(new wchar_t[UNICODE_STRING_MAX_CHARS]);
  filename[0] = 0;

  ofn.lpstrFile = filename.get();
  ofn.nMaxFile = UNICODE_STRING_MAX_CHARS;

  // We use OFN_NOCHANGEDIR so that the user can rename or delete the directory
  // without having to close Chrome first.
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER
               | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT;

  if (!filter.empty()) {
    ofn.lpstrFilter = filter.c_str();
  }
  bool success = !!GetOpenFileName(&ofn);

  if (success) {
    std::vector<base::FilePath> files;
    const wchar_t* selection = ofn.lpstrFile;
    while (*selection) {  // Empty string indicates end of list.
      files.push_back(base::FilePath(selection));
      // Skip over filename and null-terminator.
      selection += files.back().value().length() + 1;
    }
    if (files.empty()) {
      success = false;
    } else if (files.size() == 1) {
      // When there is one file, it contains the path and filename.
      paths->swap(files);
    } else {
      // Otherwise, the first string is the path, and the remainder are
      // filenames.
      std::vector<base::FilePath>::iterator path = files.begin();
      for (std::vector<base::FilePath>::iterator file = path + 1;
           file != files.end(); ++file) {
        paths->push_back(path->Append(*file));
      }
    }
  }
  return success;
}

}  // namespace

bool BrowserWebViewDelegate::ShowFileChooser(
    std::vector<base::FilePath>& file_names,
    bool multi_select,
    const WebKit::WebString& title,
    const base::FilePath& default_file,
    const std::vector<std::string>& accept_mime_types) {
  bool result = false;
  const std::wstring& filter =
      GetFilterStringFromAcceptTypes(accept_mime_types);

  if (multi_select) {
    result = RunOpenMultiFileDialog(filter, browser_->UIT_GetMainWndHandle(),
                                    &file_names);
  } else {
    base::FilePath file_name;
    result = RunOpenFileDialog(filter, browser_->UIT_GetMainWndHandle(),
                               &file_name);
    if (result)
      file_names.push_back(file_name);
  }

  return result;
}
