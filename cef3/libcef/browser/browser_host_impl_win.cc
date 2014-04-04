// Copyright (c) 2012 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/browser/browser_host_impl.h"

#include <commdlg.h>
#include <dwmapi.h>
#include <shellapi.h>
#include <wininet.h>
#include <winspool.h>

#include "libcef/browser/content_browser_client.h"
#include "libcef/browser/context.h"
#include "libcef/browser/thread_util.h"

#include "base/file_util.h"
#include "base/i18n/case_conversion.h"
#include "base/memory/ref_counted_memory.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/registry.h"
#include "base/win/windows_version.h"
#include "content/common/cursors/webcursor.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/common/file_chooser_params.h"
#include "grit/cef_strings.h"
#include "grit/ui_strings.h"
#include "grit/ui_unscaled_resources.h"
#include "net/base/mime_util.h"
#include "third_party/WebKit/public/web/WebInputEvent.h"
#include "third_party/WebKit/public/web/win/WebInputEventFactory.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/win/shell.h"
#include "ui/gfx/win/hwnd_util.h"
#include "ui/views/background.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"

#pragma comment(lib, "dwmapi.lib")

namespace {

bool IsAeroGlassEnabled() {
  if (base::win::GetVersion() < base::win::VERSION_VISTA)
    return false;

  BOOL enabled = FALSE;
  return SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && enabled;
}

void SetAeroGlass(HWND hWnd) {
  if (!IsAeroGlassEnabled())
    return;

  // Make the whole window transparent.
  MARGINS mgMarInset = { -1, -1, -1, -1 };
  DwmExtendFrameIntoClientArea(hWnd, &mgMarInset);
}

void WriteTempFileAndView(scoped_refptr<base::RefCountedString> str) {
  CEF_REQUIRE_FILET();

  base::FilePath tmp_file;
  if (!base::CreateTemporaryFile(&tmp_file))
    return;

  // The shell command will look at the file extension to identify the correct
  // program to open.
  tmp_file = tmp_file.AddExtension(L"txt");

  const std::string& data = str->data();
  int write_ct = base::WriteFile(tmp_file, data.c_str(), data.size());
  DCHECK_EQ(static_cast<int>(data.size()), write_ct);

  ui::win::OpenItemViaShell(tmp_file);
}

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
      l10n_util::GetStringUTF16(IDS_APP_SAVEAS_ALL_FILES) +
      L" (" + all_ext + L")";

  DCHECK(file_ext.size() >= ext_desc.size());

  if (file_ext.empty())
    include_all_files = true;

  std::wstring result;

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
        desc = l10n_util::GetStringFUTF16(
            IDS_APP_SAVEAS_EXTENSION_FORMAT,
            base::i18n::ToUpper(base::WideToUTF16(ext_name)),
            ext_name);
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
    int string_id;
  } kWildCardMimeTypes[] = {
    { "audio", IDS_APP_AUDIO_FILES },
    { "image", IDS_APP_IMAGE_FILES },
    { "text", IDS_APP_TEXT_FILES },
    { "video", IDS_APP_VIDEO_FILES },
  };

  for (size_t i = 0; i < arraysize(kWildCardMimeTypes); ++i) {
    if (mime_type == std::string(kWildCardMimeTypes[i].mime_type) + "/*")
      return l10n_util::GetStringUTF16(kWildCardMimeTypes[i].string_id);
  }

  return std::wstring();
}

std::wstring GetFilterStringFromAcceptTypes(
    const std::vector<base::string16>& accept_types) {
  std::vector<std::wstring> extensions;
  std::vector<std::wstring> descriptions;

  for (size_t i = 0; i < accept_types.size(); ++i) {
    std::string ascii_type = base::UTF16ToASCII(accept_types[i]);
    if (ascii_type.length()) {
      // Just treat as extension if contains '.' as the first character.
      if (ascii_type[0] == '.') {
        extensions.push_back(L"*" + base::ASCIIToWide(ascii_type));
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

// from chrome/browser/views/shell_dialogs_win.cc

bool RunOpenFileDialog(const content::FileChooserParams& params,
                       HWND owner,
                       base::FilePath* path) {
  OPENFILENAME ofn;

  // We must do this otherwise the ofn's FlagsEx may be initialized to random
  // junk in release builds which can cause the Places Bar not to show up!
  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = owner;

  // Consider default file name if any.
  base::FilePath default_file_name(params.default_file_name);

  wchar_t filename[MAX_PATH] = {0};

  ofn.lpstrFile = filename;
  ofn.nMaxFile = MAX_PATH;

  std::wstring directory;
  if (!default_file_name.empty()) {
    base::wcslcpy(filename, default_file_name.value().c_str(),
        arraysize(filename));

    directory = default_file_name.DirName().value();
    ofn.lpstrInitialDir = directory.c_str();
  }

  std::wstring title;
  if (!params.title.empty())
    title = params.title;
  else
    title = l10n_util::GetStringUTF16(IDS_OPEN_FILE_DIALOG_TITLE);
  if (!title.empty())
    ofn.lpstrTitle = title.c_str();

  // We use OFN_NOCHANGEDIR so that the user can rename or delete the directory
  // without having to close Chrome first.
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER |
              OFN_ENABLESIZING;

  std::wstring filter = GetFilterStringFromAcceptTypes(params.accept_types);
  if (!filter.empty())
    ofn.lpstrFilter = filter.c_str();

  bool success = !!GetOpenFileName(&ofn);
  if (success)
    *path = base::FilePath(filename);
  return success;
}

bool RunOpenMultiFileDialog(const content::FileChooserParams& params,
                            HWND owner,
                            std::vector<base::FilePath>* paths) {
  OPENFILENAME ofn;

  // We must do this otherwise the ofn's FlagsEx may be initialized to random
  // junk in release builds which can cause the Places Bar not to show up!
  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = owner;

  scoped_ptr<wchar_t[]> filename(new wchar_t[UNICODE_STRING_MAX_CHARS]);
  filename[0] = 0;

  ofn.lpstrFile = filename.get();
  ofn.nMaxFile = UNICODE_STRING_MAX_CHARS;

  std::wstring title;
  if (!params.title.empty())
    title = params.title;
  else
    title = l10n_util::GetStringUTF16(IDS_OPEN_FILES_DIALOG_TITLE);
  if (!title.empty())
    ofn.lpstrTitle = title.c_str();

  // We use OFN_NOCHANGEDIR so that the user can rename or delete the directory
  // without having to close Chrome first.
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER |
              OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT | OFN_ENABLESIZING;

  std::wstring filter = GetFilterStringFromAcceptTypes(params.accept_types);
  if (!filter.empty())
    ofn.lpstrFilter = filter.c_str();

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

bool RunSaveFileDialog(const content::FileChooserParams& params,
                       HWND owner,
                       base::FilePath* path) {
  OPENFILENAME ofn;

  // We must do this otherwise the ofn's FlagsEx may be initialized to random
  // junk in release builds which can cause the Places Bar not to show up!
  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = owner;

  // Consider default file name if any.
  base::FilePath default_file_name(params.default_file_name);

  wchar_t filename[MAX_PATH] = {0};

  ofn.lpstrFile = filename;
  ofn.nMaxFile = MAX_PATH;

  std::wstring directory;
  if (!default_file_name.empty()) {
    base::wcslcpy(filename, default_file_name.value().c_str(),
        arraysize(filename));

    directory = default_file_name.DirName().value();
    ofn.lpstrInitialDir = directory.c_str();
  }

  std::wstring title;
  if (!params.title.empty())
    title = params.title;
  else
    title = l10n_util::GetStringUTF16(IDS_SAVE_AS_DIALOG_TITLE);
  if (!title.empty())
    ofn.lpstrTitle = title.c_str();

  // We use OFN_NOCHANGEDIR so that the user can rename or delete the directory
  // without having to close Chrome first.
  ofn.Flags = OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_ENABLESIZING |
              OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;

  std::wstring filter = GetFilterStringFromAcceptTypes(params.accept_types);
  if (!filter.empty())
    ofn.lpstrFilter = filter.c_str();

  bool success = !!GetSaveFileName(&ofn);
  if (success)
    *path = base::FilePath(filename);
  return success;
}


// According to Mozilla in uriloader/exthandler/win/nsOSHelperAppService.cpp:
// "Some versions of windows (Win2k before SP3, Win XP before SP1) crash in
// ShellExecute on long URLs (bug 161357 on bugzilla.mozilla.org). IE 5 and 6
// support URLS of 2083 chars in length, 2K is safe."
const int kMaxAddressLengthChars = 2048;

bool HasExternalHandler(const std::string& scheme) {
  base::win::RegKey key;
  const std::wstring registry_path =
      base::ASCIIToWide(scheme + "\\shell\\open\\command");
  key.Open(HKEY_CLASSES_ROOT, registry_path.c_str(), KEY_READ);
  if (key.Valid()) {
    DWORD size = 0;
    key.ReadValue(NULL, NULL, &size, NULL);
    if (size > 2) {
       // ShellExecute crashes the process when the command is empty.
       // We check for "2" because it always returns the trailing NULL.
       return true;
    }
  }

  return false;
}

WORD KeyStatesToWord() {
  static const USHORT kHighBitMaskShort = 0x8000;
  WORD result = 0;

  if (GetKeyState(VK_CONTROL) & kHighBitMaskShort)
    result |= MK_CONTROL;
  if (GetKeyState(VK_SHIFT) & kHighBitMaskShort)
    result |= MK_SHIFT;
  if (GetKeyState(VK_LBUTTON) & kHighBitMaskShort)
    result |= MK_LBUTTON;
  if (GetKeyState(VK_MBUTTON) & kHighBitMaskShort)
    result |= MK_MBUTTON;
  if (GetKeyState(VK_RBUTTON) & kHighBitMaskShort)
    result |= MK_RBUTTON;
  return result;
}

// From content/common/cursors/webcursor_win.cc.

using blink::WebCursorInfo;

LPCWSTR ToCursorID(WebCursorInfo::Type type) {
  switch (type) {
    case WebCursorInfo::TypePointer:
      return IDC_ARROW;
    case WebCursorInfo::TypeCross:
      return IDC_CROSS;
    case WebCursorInfo::TypeHand:
      return IDC_HAND;
    case WebCursorInfo::TypeIBeam:
      return IDC_IBEAM;
    case WebCursorInfo::TypeWait:
      return IDC_WAIT;
    case WebCursorInfo::TypeHelp:
      return IDC_HELP;
    case WebCursorInfo::TypeEastResize:
      return IDC_SIZEWE;
    case WebCursorInfo::TypeNorthResize:
      return IDC_SIZENS;
    case WebCursorInfo::TypeNorthEastResize:
      return IDC_SIZENESW;
    case WebCursorInfo::TypeNorthWestResize:
      return IDC_SIZENWSE;
    case WebCursorInfo::TypeSouthResize:
      return IDC_SIZENS;
    case WebCursorInfo::TypeSouthEastResize:
      return IDC_SIZENWSE;
    case WebCursorInfo::TypeSouthWestResize:
      return IDC_SIZENESW;
    case WebCursorInfo::TypeWestResize:
      return IDC_SIZEWE;
    case WebCursorInfo::TypeNorthSouthResize:
      return IDC_SIZENS;
    case WebCursorInfo::TypeEastWestResize:
      return IDC_SIZEWE;
    case WebCursorInfo::TypeNorthEastSouthWestResize:
      return IDC_SIZENESW;
    case WebCursorInfo::TypeNorthWestSouthEastResize:
      return IDC_SIZENWSE;
    case WebCursorInfo::TypeColumnResize:
      return MAKEINTRESOURCE(IDC_COLRESIZE);
    case WebCursorInfo::TypeRowResize:
      return MAKEINTRESOURCE(IDC_ROWRESIZE);
    case WebCursorInfo::TypeMiddlePanning:
      return MAKEINTRESOURCE(IDC_PAN_MIDDLE);
    case WebCursorInfo::TypeEastPanning:
      return MAKEINTRESOURCE(IDC_PAN_EAST);
    case WebCursorInfo::TypeNorthPanning:
      return MAKEINTRESOURCE(IDC_PAN_NORTH);
    case WebCursorInfo::TypeNorthEastPanning:
      return MAKEINTRESOURCE(IDC_PAN_NORTH_EAST);
    case WebCursorInfo::TypeNorthWestPanning:
      return MAKEINTRESOURCE(IDC_PAN_NORTH_WEST);
    case WebCursorInfo::TypeSouthPanning:
      return MAKEINTRESOURCE(IDC_PAN_SOUTH);
    case WebCursorInfo::TypeSouthEastPanning:
      return MAKEINTRESOURCE(IDC_PAN_SOUTH_EAST);
    case WebCursorInfo::TypeSouthWestPanning:
      return MAKEINTRESOURCE(IDC_PAN_SOUTH_WEST);
    case WebCursorInfo::TypeWestPanning:
      return MAKEINTRESOURCE(IDC_PAN_WEST);
    case WebCursorInfo::TypeMove:
      return IDC_SIZEALL;
    case WebCursorInfo::TypeVerticalText:
      return MAKEINTRESOURCE(IDC_VERTICALTEXT);
    case WebCursorInfo::TypeCell:
      return MAKEINTRESOURCE(IDC_CELL);
    case WebCursorInfo::TypeContextMenu:
      return MAKEINTRESOURCE(IDC_ARROW);
    case WebCursorInfo::TypeAlias:
      return MAKEINTRESOURCE(IDC_ALIAS);
    case WebCursorInfo::TypeProgress:
      return IDC_APPSTARTING;
    case WebCursorInfo::TypeNoDrop:
      return IDC_NO;
    case WebCursorInfo::TypeCopy:
      return MAKEINTRESOURCE(IDC_COPYCUR);
    case WebCursorInfo::TypeNone:
      return MAKEINTRESOURCE(IDC_CURSOR_NONE);
    case WebCursorInfo::TypeNotAllowed:
      return IDC_NO;
    case WebCursorInfo::TypeZoomIn:
      return MAKEINTRESOURCE(IDC_ZOOMIN);
    case WebCursorInfo::TypeZoomOut:
      return MAKEINTRESOURCE(IDC_ZOOMOUT);
    case WebCursorInfo::TypeGrab:
      return MAKEINTRESOURCE(IDC_HAND_GRAB);
    case WebCursorInfo::TypeGrabbing:
      return MAKEINTRESOURCE(IDC_HAND_GRABBING);
  }
  NOTREACHED();
  return NULL;
}

bool IsSystemCursorID(LPCWSTR cursor_id) {
  return cursor_id >= IDC_ARROW;  // See WinUser.h
}

// Manages the views-based root window that hosts the web contents. This object
// will be deleted automatically when the associated root window is destroyed.
class CefWindowDelegateView : public views::WidgetDelegateView {
 public:
  explicit CefWindowDelegateView(SkColor background_color)
    : background_color_(background_color),
      web_view_(NULL) {
    
  }

  // Create the Widget and associated root window.
  void Init(gfx::AcceleratedWidget parent_widget,
            content::WebContents* web_contents,
            const gfx::Rect& bounds) {
    DCHECK(!web_view_);
    web_view_ = new views::WebView(web_contents->GetBrowserContext());
    web_view_->SetWebContents(web_contents);
    web_view_->SetPreferredSize(bounds.size());

    views::Widget* widget = new views::Widget;

    // See CalculateWindowStylesFromInitParams in
    // ui/views/widget/widget_hwnd_utils.cc for the conversion of |params| to
    // Windows style flags.
    views::Widget::InitParams params;
    params.parent_widget = parent_widget;
    params.bounds = bounds;
    params.delegate = this;
    params.top_level = true;
    // Set the WS_CHILD flag.
    params.child = true;
    // Set the WS_VISIBLE flag.
    params.type = views::Widget::InitParams::TYPE_CONTROL;
    // Don't set the WS_EX_COMPOSITED flag.
    params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;

    // Results in a call to InitContent().
    widget->Init(params);

    // |widget| should now be associated with |this|.
    DCHECK_EQ(widget, GetWidget());
  }

 private:
  // Initialize the Widget's content.
  void InitContent() {
    set_background(views::Background::CreateSolidBackground(background_color_));
    SetLayoutManager(new views::FillLayout());
    AddChildView(web_view_);
  }

  // WidgetDelegateView methods:
  virtual bool CanResize() const OVERRIDE { return true; }
  virtual bool CanMaximize() const OVERRIDE { return true; }
  virtual View* GetContentsView() OVERRIDE { return this; }

  // View methods:
  virtual void ViewHierarchyChanged(
      const ViewHierarchyChangedDetails& details) OVERRIDE {
    if (details.is_add && details.child == this)
      InitContent();
  }

 private:
  SkColor background_color_;
  views::WebView* web_view_;

  DISALLOW_COPY_AND_ASSIGN(CefWindowDelegateView);
};

}  // namespace

// static
void CefBrowserHostImpl::RegisterWindowClass() {
  // Register the window class
  WNDCLASSEX wcex = {
    /* cbSize = */ sizeof(WNDCLASSEX),
    /* style = */ CS_HREDRAW | CS_VREDRAW,
    /* lpfnWndProc = */ CefBrowserHostImpl::WndProc,
    /* cbClsExtra = */ 0,
    /* cbWndExtra = */ 0,
    /* hInstance = */ ::GetModuleHandle(NULL),
    /* hIcon = */ NULL,
    /* hCursor = */ LoadCursor(NULL, IDC_ARROW),
    /* hbrBackground = */ 0,
    /* lpszMenuName = */ NULL,
    /* lpszClassName = */ CefBrowserHostImpl::GetWndClass(),
    /* hIconSm = */ NULL,
  };
  RegisterClassEx(&wcex);
}

// static
LPCTSTR CefBrowserHostImpl::GetWndClass() {
  return L"CefBrowserWindow";
}

// static
LRESULT CALLBACK CefBrowserHostImpl::WndProc(HWND hwnd, UINT message,
                                             WPARAM wParam, LPARAM lParam) {
  CefBrowserHostImpl* browser =
      static_cast<CefBrowserHostImpl*>(gfx::GetWindowUserData(hwnd));

  switch (message) {
  case WM_CLOSE:
    // Protect against multiple requests to close while the close is pending.
    if (browser && browser->destruction_state() <= DESTRUCTION_STATE_PENDING) {
      if (browser->destruction_state() == DESTRUCTION_STATE_NONE) {
        // Request that the browser close.
        browser->CloseBrowser(false);
      }

      // Cancel the close.
      return 0;
    }

    // Allow the close.
    break;

  case WM_DESTROY:
    if (browser) {
      // Clear the user data pointer.
      gfx::SetWindowUserData(hwnd, NULL);

      // Force the browser to be destroyed and release the reference added in
      // PlatformCreateWindow().
      browser->WindowDestroyed();
    }
    return 0;

  case WM_SIZE:
    // Minimizing resizes the window to 0x0 which causes our layout to go all
    // screwy, so we just ignore it.
    if (wParam != SIZE_MINIMIZED && browser && browser->window_widget_) {
      // Resize the Widget window to the full size of the browser window.
      RECT rc;
      GetClientRect(hwnd, &rc);
      browser->window_widget_->SetSize(gfx::Size(rc.right, rc.bottom));
    }
    return 0;

  case WM_SETFOCUS:
    if (browser)
      browser->OnSetFocus(FOCUS_SOURCE_SYSTEM);
    return 0;

  case WM_ERASEBKGND:
    return 0;

  case WM_DWMCOMPOSITIONCHANGED:
    // Message sent to top-level windows when composition has been enabled or
    // disabled.
    if (browser && browser->IsTransparent())
      SetAeroGlass(hwnd);
    break;
  }

  return DefWindowProc(hwnd, message, wParam, lParam);
}

ui::PlatformCursor CefBrowserHostImpl::GetPlatformCursor(
    blink::WebCursorInfo::Type type) {
  HMODULE module_handle = NULL;
  const wchar_t* cursor_id = ToCursorID(type);
  if (!IsSystemCursorID(cursor_id)) {
    module_handle = ::GetModuleHandle(
        CefContentBrowserClient::Get()->GetResourceDllName());
    if (!module_handle)
      module_handle = ::GetModuleHandle(NULL);
  }

  return LoadCursor(module_handle, cursor_id);
}

bool CefBrowserHostImpl::PlatformCreateWindow() {
  std::wstring windowName(CefString(&window_info_.window_name));

  // Create the new browser window.
  window_info_.window = CreateWindowEx(window_info_.ex_style,
      GetWndClass(), windowName.c_str(), window_info_.style,
      window_info_.x, window_info_.y, window_info_.width,
      window_info_.height, window_info_.parent_window, window_info_.menu,
      ::GetModuleHandle(NULL), NULL);

  // It's possible for CreateWindowEx to fail if the parent window was
  // destroyed between the call to CreateBrowser and the above one.
  DCHECK(window_info_.window != NULL);
  if (!window_info_.window)
    return false;

  if (window_info_.transparent_painting_enabled &&
      !(window_info_.style & WS_CHILD)) {
    // Transparent top-level windows will be given "sheet of glass" effect.
    SetAeroGlass(window_info_.window);
  }

  // Set window user data to this object for future reference from the window
  // procedure.
  gfx::SetWindowUserData(window_info_.window, this);

  // Add a reference that will be released in the WM_DESTROY handler.
  AddRef();

  RECT cr;
  GetClientRect(window_info_.window, &cr);

  DCHECK(!window_widget_);

  SkColor background_color = SK_ColorWHITE;
  const CefSettings& settings = CefContext::Get()->settings();
  if (CefColorGetA(settings.background_color) > 0) {
    background_color = SkColorSetRGB(
        CefColorGetR(settings.background_color),
        CefColorGetG(settings.background_color),
        CefColorGetB(settings.background_color));
  }

  CefWindowDelegateView* delegate_view =
      new CefWindowDelegateView(background_color);
  delegate_view->Init(window_info_.window,
                      web_contents(),
                      gfx::Rect(0, 0, cr.right, cr.bottom));

  window_widget_ = delegate_view->GetWidget();
  window_widget_->Show();

  return true;
}

void CefBrowserHostImpl::PlatformCloseWindow() {
  if (window_info_.window != NULL) {
    HWND frameWnd = GetAncestor(window_info_.window, GA_ROOT);
    PostMessage(frameWnd, WM_CLOSE, 0, 0);
  }
}

void CefBrowserHostImpl::PlatformSizeTo(int width, int height) {
  RECT rect = {0, 0, width, height};
  DWORD style = GetWindowLong(window_info_.window, GWL_STYLE);
  DWORD ex_style = GetWindowLong(window_info_.window, GWL_EXSTYLE);
  bool has_menu = !(style & WS_CHILD) && (GetMenu(window_info_.window) != NULL);

  // The size value is for the client area. Calculate the whole window size
  // based on the current style.
  AdjustWindowRectEx(&rect, style, has_menu, ex_style);

  // Size the window.
  SetWindowPos(window_info_.window, NULL, 0, 0, rect.right,
               rect.bottom, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
}

CefWindowHandle CefBrowserHostImpl::PlatformGetWindowHandle() {
  return IsWindowRenderingDisabled() ?
      window_info_.parent_window :
      window_info_.window;
}

bool CefBrowserHostImpl::PlatformViewText(const std::string& text) {
  CEF_REQUIRE_UIT();

  std::string str = text;
  scoped_refptr<base::RefCountedString> str_ref =
      base::RefCountedString::TakeString(&str);
  CEF_POST_TASK(CEF_FILET, base::Bind(WriteTempFileAndView, str_ref));
  return true;
}

void CefBrowserHostImpl::PlatformHandleKeyboardEvent(
    const content::NativeWebKeyboardEvent& event) {
  // Any unhandled keyboard/character messages are sent to DefWindowProc so that
  // shortcut keys work correctly.
  HWND hwnd = PlatformGetWindowHandle();
  if (!hwnd)
    return;

  UINT message = 0;
  switch (event.type) {
    case blink::WebInputEvent::RawKeyDown:
      message = WM_KEYDOWN;
      break;
    case blink::WebInputEvent::KeyUp:
      message = WM_KEYUP;
      break;
    case blink::WebInputEvent::Char:
      message = WM_CHAR;
      break;
    default:
      NOTREACHED();
      return;
  }

  DefWindowProc(hwnd, message, event.windowsKeyCode, event.nativeKeyCode);
}

void CefBrowserHostImpl::PlatformRunFileChooser(
    const content::FileChooserParams& params,
    RunFileChooserCallback callback) {
  std::vector<base::FilePath> files;

  if (params.mode == content::FileChooserParams::Open) {
    base::FilePath file;
    if (RunOpenFileDialog(params, PlatformGetWindowHandle(), &file))
      files.push_back(file);
  } else if (params.mode == content::FileChooserParams::OpenMultiple) {
    RunOpenMultiFileDialog(params, PlatformGetWindowHandle(), &files);
  } else if (params.mode == content::FileChooserParams::Save) {
    base::FilePath file;
    if (RunSaveFileDialog(params, PlatformGetWindowHandle(), &file))
      files.push_back(file);
  } else {
    NOTIMPLEMENTED();
  }

  callback.Run(files);
}

void CefBrowserHostImpl::PlatformHandleExternalProtocol(const GURL& url) {
  if (CEF_CURRENTLY_ON_FILET()) {
    if (!HasExternalHandler(url.scheme()))
      return;

    const std::string& address = url.spec();
    if (address.length() > kMaxAddressLengthChars)
      return;

    ShellExecuteA(NULL, "open", address.c_str(), NULL, NULL, SW_SHOWNORMAL);
  } else {
    // Execute on the FILE thread.
    CEF_POST_TASK(CEF_FILET,
        base::Bind(&CefBrowserHostImpl::PlatformHandleExternalProtocol, this,
                   url));
  }
}

// static
bool CefBrowserHostImpl::IsWindowRenderingDisabled(const CefWindowInfo& info) {
  return info.windowless_rendering_enabled ? true : false;
}

bool CefBrowserHostImpl::IsTransparent() {
  return window_info_.transparent_painting_enabled ? true : false;
}

void CefBrowserHostImpl::PlatformTranslateKeyEvent(
    content::NativeWebKeyboardEvent& result, const CefKeyEvent& key_event) {
  result.timeStampSeconds = GetMessageTime() / 1000.0;

  result.windowsKeyCode = key_event.windows_key_code;
  result.nativeKeyCode = key_event.native_key_code;
  result.isSystemKey = key_event.is_system_key ? 1 : 0;
  switch (key_event.type) {
  case KEYEVENT_RAWKEYDOWN:
  case KEYEVENT_KEYDOWN:
    result.type = blink::WebInputEvent::RawKeyDown;
    break;
  case KEYEVENT_KEYUP:
    result.type = blink::WebInputEvent::KeyUp;
    break;
  case KEYEVENT_CHAR:
    result.type = blink::WebInputEvent::Char;
    break;
  default:
    NOTREACHED();
  }

  if (result.type == blink::WebInputEvent::Char ||
      result.type == blink::WebInputEvent::RawKeyDown) {
    result.text[0] = result.windowsKeyCode;
    result.unmodifiedText[0] = result.windowsKeyCode;
  }
  if (result.type != blink::WebInputEvent::Char)
    result.setKeyIdentifierFromWindowsKeyCode();

  result.modifiers |= TranslateModifiers(key_event.modifiers);
}

void CefBrowserHostImpl::PlatformTranslateClickEvent(
    blink::WebMouseEvent& result,
    const CefMouseEvent& mouse_event,
    CefBrowserHost::MouseButtonType type,
    bool mouseUp, int clickCount) {
  PlatformTranslateMouseEvent(result, mouse_event);

  switch (type) {
  case MBT_LEFT:
    result.type = mouseUp ? blink::WebInputEvent::MouseUp :
                            blink::WebInputEvent::MouseDown;
    result.button = blink::WebMouseEvent::ButtonLeft;
    break;
  case MBT_MIDDLE:
    result.type = mouseUp ? blink::WebInputEvent::MouseUp :
                            blink::WebInputEvent::MouseDown;
    result.button = blink::WebMouseEvent::ButtonMiddle;
    break;
  case MBT_RIGHT:
    result.type = mouseUp ? blink::WebInputEvent::MouseUp :
                            blink::WebInputEvent::MouseDown;
    result.button = blink::WebMouseEvent::ButtonRight;
    break;
  default:
    NOTREACHED();
  }

  result.clickCount = clickCount;
}

void CefBrowserHostImpl::PlatformTranslateMoveEvent(
    blink::WebMouseEvent& result,
    const CefMouseEvent& mouse_event,
    bool mouseLeave) {
  PlatformTranslateMouseEvent(result, mouse_event);

  if (!mouseLeave) {
    result.type = blink::WebInputEvent::MouseMove;
    if (mouse_event.modifiers & EVENTFLAG_LEFT_MOUSE_BUTTON)
      result.button = blink::WebMouseEvent::ButtonLeft;
    else if (mouse_event.modifiers & EVENTFLAG_MIDDLE_MOUSE_BUTTON)
      result.button = blink::WebMouseEvent::ButtonMiddle;
    else if (mouse_event.modifiers & EVENTFLAG_RIGHT_MOUSE_BUTTON)
      result.button = blink::WebMouseEvent::ButtonRight;
    else
      result.button = blink::WebMouseEvent::ButtonNone;
  } else {
    result.type = blink::WebInputEvent::MouseLeave;
    result.button = blink::WebMouseEvent::ButtonNone;
  }

  result.clickCount = 0;
}

void CefBrowserHostImpl::PlatformTranslateWheelEvent(
    blink::WebMouseWheelEvent& result,
    const CefMouseEvent& mouse_event,
    int deltaX, int deltaY) {
  PlatformTranslateMouseEvent(result, mouse_event);

  result.type = blink::WebInputEvent::MouseWheel;
  result.button = blink::WebMouseEvent::ButtonNone;

  float wheelDelta;
  bool horizontalScroll = false;

  wheelDelta = static_cast<float>(deltaY ? deltaY : deltaX);

  horizontalScroll = (deltaY == 0);

  static const ULONG defaultScrollCharsPerWheelDelta = 1;
  static const FLOAT scrollbarPixelsPerLine = 100.0f / 3.0f;
  static const ULONG defaultScrollLinesPerWheelDelta = 3;
  wheelDelta /= WHEEL_DELTA;
  float scrollDelta = wheelDelta;
  if (horizontalScroll) {
    ULONG scrollChars = defaultScrollCharsPerWheelDelta;
    SystemParametersInfo(SPI_GETWHEELSCROLLCHARS, 0, &scrollChars, 0);
    scrollDelta *= static_cast<FLOAT>(scrollChars) * scrollbarPixelsPerLine;
  } else {
    ULONG scrollLines = defaultScrollLinesPerWheelDelta;
    SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &scrollLines, 0);
    if (scrollLines == WHEEL_PAGESCROLL)
      result.scrollByPage = true;
    if (!result.scrollByPage)
      scrollDelta *= static_cast<FLOAT>(scrollLines) * scrollbarPixelsPerLine;
  }

  // Set scroll amount based on above calculations.  WebKit expects positive
  // deltaY to mean "scroll up" and positive deltaX to mean "scroll left".
  if (horizontalScroll) {
    result.deltaX = scrollDelta;
    result.wheelTicksX = wheelDelta;
  } else {
    result.deltaY = scrollDelta;
    result.wheelTicksY = wheelDelta;
  }
}

void CefBrowserHostImpl::PlatformTranslateMouseEvent(
    blink::WebMouseEvent& result,
    const CefMouseEvent& mouse_event) {
  // position
  result.x = mouse_event.x;
  result.y = mouse_event.y;
  result.windowX = result.x;
  result.windowY = result.y;
  result.globalX = result.x;
  result.globalY = result.y;

  // global position
  if (IsWindowRenderingDisabled()) {
    GetClient()->GetRenderHandler()->GetScreenPoint(GetBrowser(),
        result.x, result.y,
        result.globalX, result.globalY);
  } else {
    POINT globalPoint = { result.x, result.y };
    ClientToScreen(GetWindowHandle(), &globalPoint);
    result.globalX = globalPoint.x;
    result.globalY = globalPoint.y;
  }

  // modifiers
  result.modifiers |= TranslateModifiers(mouse_event.modifiers);

  // timestamp
  result.timeStampSeconds = GetMessageTime() / 1000.0;
}
