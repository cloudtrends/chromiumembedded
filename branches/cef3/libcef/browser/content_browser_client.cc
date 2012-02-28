// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/browser/content_browser_client.h"
#include "libcef/browser/browser_impl.h"
#include "libcef/browser/browser_main.h"
#include "libcef/browser/browser_message_filter.h"
#include "libcef/browser/browser_settings.h"
#include "libcef/browser/resource_dispatcher_host_delegate.h"
#include "libcef/common/cef_switches.h"

#include "base/command_line.h"
#include "base/file_path.h"
#include "content/browser/renderer_host/resource_dispatcher_host.h"
#include "content/public/common/content_switches.h"
#include "googleurl/src/gurl.h"
#include "third_party/skia/include/core/SkBitmap.h"

#if defined(OS_WIN)
#include "content/browser/tab_contents/tab_contents.h"
#include "content/browser/tab_contents/tab_contents_view_win.h"
#include "content/common/view_messages.h"
#elif defined(OS_LINUX)
#include "content/browser/tab_contents/tab_contents_view_gtk.h"
#endif

CefContentBrowserClient::CefContentBrowserClient()
    : browser_main_parts_(NULL) {
}

CefContentBrowserClient::~CefContentBrowserClient() {
}

content::BrowserMainParts* CefContentBrowserClient::CreateBrowserMainParts(
    const content::MainFunctionParams& parameters) {
  return new CefBrowserMainParts(parameters);
}

content::WebContentsView* CefContentBrowserClient::CreateWebContentsView(
    content::WebContents* web_contents) {
#if defined(OS_WIN)
  return new TabContentsViewWin(web_contents);
#elif defined(OS_LINUX)
  return new content::TabContentsViewGtk(web_contents, NULL);
#else
  return NULL;
#endif
}

void CefContentBrowserClient::RenderViewHostCreated(
    RenderViewHost* render_view_host) {
}

void CefContentBrowserClient::RenderProcessHostCreated(
    content::RenderProcessHost* host) {
  host->GetChannel()->AddFilter(new CefBrowserMessageFilter(host));
}

content::WebUIControllerFactory*
CefContentBrowserClient::GetWebUIControllerFactory() {
  return NULL;
}

GURL CefContentBrowserClient::GetEffectiveURL(
    content::BrowserContext* browser_context, const GURL& url) {
  return GURL();
}

bool CefContentBrowserClient::ShouldUseProcessPerSite(
    content::BrowserContext* browser_context, const GURL& effective_url) {
  return false;
}

bool CefContentBrowserClient::IsHandledURL(const GURL& url) {
  return false;
}

bool CefContentBrowserClient::IsSuitableHost(
    content::RenderProcessHost* process_host,
    const GURL& site_url) {
  return true;
}

bool CefContentBrowserClient::ShouldTryToUseExistingProcessHost(
      content::BrowserContext* browser_context, const GURL& url) {
  return false;
}

void CefContentBrowserClient::SiteInstanceGotProcess(
    content::SiteInstance* site_instance) {
}

void CefContentBrowserClient::SiteInstanceDeleting(
    content::SiteInstance* site_instance) {
}

bool CefContentBrowserClient::ShouldSwapProcessesForNavigation(
    const GURL& current_url,
    const GURL& new_url) {
  return false;
}

std::string CefContentBrowserClient::GetCanonicalEncodingNameByAliasName(
    const std::string& alias_name) {
  return std::string();
}

void CefContentBrowserClient::AppendExtraCommandLineSwitches(
    CommandLine* command_line, int child_process_id) {
  std::string process_type =
      command_line->GetSwitchValueASCII(switches::kProcessType);
  if (process_type == switches::kRendererProcess) {
      // Propagate the following switches to the renderer command line (along
      // with any associated values) if present in the browser command line.
      static const char* const kSwitchNames[] = {
        switches::kProductVersion,
        switches::kLocale,
        switches::kPackFilePath,
        switches::kLocalesDirPath,
        switches::kPackLoadingDisabled,
      };
      const CommandLine& browser_cmd = *CommandLine::ForCurrentProcess();
      command_line->CopySwitchesFrom(browser_cmd, kSwitchNames,
                                     arraysize(kSwitchNames));
  }
}

std::string CefContentBrowserClient::GetApplicationLocale() {
  return std::string();
}

std::string CefContentBrowserClient::GetAcceptLangs(
    content::BrowserContext* context) {
  return std::string();
}

SkBitmap* CefContentBrowserClient::GetDefaultFavicon() {
  static SkBitmap empty;
  return &empty;
}

bool CefContentBrowserClient::AllowAppCache(
    const GURL& manifest_url,
    const GURL& first_party,
    content::ResourceContext* context) {
  return true;
}

bool CefContentBrowserClient::AllowGetCookie(
    const GURL& url,
    const GURL& first_party,
    const net::CookieList& cookie_list,
    content::ResourceContext* context,
    int render_process_id,
    int render_view_id) {
  return true;
}

bool CefContentBrowserClient::AllowSetCookie(
    const GURL& url,
    const GURL& first_party,
    const std::string& cookie_line,
    content::ResourceContext* context,
    int render_process_id,
    int render_view_id,
    net::CookieOptions* options) {
  return true;
}

bool CefContentBrowserClient::AllowSaveLocalState(
    content::ResourceContext* context) {
  return true;
}

bool CefContentBrowserClient::AllowWorkerDatabase(
    const GURL& url,
    const string16& name,
    const string16& display_name,
    unsigned long estimated_size,  // NOLINT(runtime/int)
    content::ResourceContext* context,
    const std::vector<std::pair<int, int> >& render_views) {
  return true;
}

bool CefContentBrowserClient::AllowWorkerFileSystem(
    const GURL& url,
    content::ResourceContext* context,
    const std::vector<std::pair<int, int> >& render_views) {
  return true;
}

QuotaPermissionContext*
    CefContentBrowserClient::CreateQuotaPermissionContext() {
  return NULL;
}

net::URLRequestContext* CefContentBrowserClient::OverrideRequestContextForURL(
    const GURL& url, content::ResourceContext* context) {
  return NULL;
}

void CefContentBrowserClient::OpenItem(const FilePath& path) {
}

void CefContentBrowserClient::ShowItemInFolder(const FilePath& path) {
}

void CefContentBrowserClient::AllowCertificateError(
    int render_process_id,
    int render_view_id,
    int cert_error,
    const net::SSLInfo& ssl_info,
    const GURL& request_url,
    bool overridable,
    const base::Callback<void(bool)>& callback,
    bool* cancel_request) {
}

void CefContentBrowserClient::SelectClientCertificate(
    int render_process_id,
    int render_view_id,
    const net::HttpNetworkSession* network_session,
    net::SSLCertRequestInfo* cert_request_info,
    const base::Callback<void(net::X509Certificate*)>& callback) {
}

void CefContentBrowserClient::AddNewCertificate(
    net::URLRequest* request,
    net::X509Certificate* cert,
    int render_process_id,
    int render_view_id) {
}

bool CefContentBrowserClient::AllowSocketAPI(const GURL& url) {
  return false;
}

void CefContentBrowserClient::RequestMediaAccessPermission(
    const content::MediaStreamRequest* request,
    const content::MediaResponseCallback& callback) {
}

void CefContentBrowserClient::RequestDesktopNotificationPermission(
    const GURL& source_origin,
    int callback_context,
    int render_process_id,
    int render_view_id) {
}

WebKit::WebNotificationPresenter::Permission
    CefContentBrowserClient::CheckDesktopNotificationPermission(
        const GURL& source_origin,
        content::ResourceContext* context,
        int render_process_id) {
  return WebKit::WebNotificationPresenter::PermissionAllowed;
}

void CefContentBrowserClient::ShowDesktopNotification(
    const content::ShowDesktopNotificationHostMsgParams& params,
    int render_process_id,
    int render_view_id,
    bool worker) {
}

void CefContentBrowserClient::CancelDesktopNotification(
    int render_process_id,
    int render_view_id,
    int notification_id) {
}

bool CefContentBrowserClient::CanCreateWindow(
    const GURL& origin,
    WindowContainerType container_type,
    content::ResourceContext* context,
    int render_process_id) {
  return true;
}

std::string CefContentBrowserClient::GetWorkerProcessTitle(
    const GURL& url, content::ResourceContext* context) {
  return std::string();
}

void CefContentBrowserClient::ResourceDispatcherHostCreated() {
  ResourceDispatcherHost* rdh = ResourceDispatcherHost::Get();
  resource_dispatcher_host_delegate_.reset(
      new CefResourceDispatcherHostDelegate(rdh));
  rdh->set_delegate(resource_dispatcher_host_delegate_.get());
}

content::SpeechInputManagerDelegate*
CefContentBrowserClient::GetSpeechInputManagerDelegate() {
  return NULL;
}

ui::Clipboard* CefContentBrowserClient::GetClipboard() {
  return browser_main_parts_->GetClipboard();
}

MHTMLGenerationManager* CefContentBrowserClient::GetMHTMLGenerationManager() {
  return NULL;
}

net::NetLog* CefContentBrowserClient::GetNetLog() {
  return NULL;
}

content::AccessTokenStore* CefContentBrowserClient::CreateAccessTokenStore() {
  return NULL;
}

bool CefContentBrowserClient::IsFastShutdownPossible() {
  return true;
}

void CefContentBrowserClient::OverrideWebkitPrefs(RenderViewHost* rvh,
                                                  WebPreferences* prefs) {
  CefRefPtr<CefBrowserImpl> browser = CefBrowserImpl::GetBrowserForHost(rvh);
  DCHECK(browser.get());

  // Populate WebPreferences based on CefBrowserSettings.
  BrowserToWebSettings(browser->settings(), *prefs);
}

void CefContentBrowserClient::UpdateInspectorSetting(
    RenderViewHost* rvh, const std::string& key, const std::string& value) {
}

void CefContentBrowserClient::ClearInspectorSettings(RenderViewHost* rvh) {
}

void CefContentBrowserClient::BrowserURLHandlerCreated(
    BrowserURLHandler* handler) {
}

void CefContentBrowserClient::ClearCache(RenderViewHost* rvh) {
}

void CefContentBrowserClient::ClearCookies(RenderViewHost* rvh) {
}

FilePath CefContentBrowserClient::GetDefaultDownloadDirectory() {
  return FilePath();
}

std::string CefContentBrowserClient::GetDefaultDownloadName() {
  return "download";
}

#if defined(OS_POSIX) && !defined(OS_MACOSX)
int CefContentBrowserClient::GetCrashSignalFD(
    const CommandLine& command_line) {
  return -1;
}
#endif

#if defined(OS_WIN)
const wchar_t* CefContentBrowserClient::GetResourceDllName() {
  return NULL;
}
#endif

#if defined(USE_NSS)
crypto::CryptoModuleBlockingPasswordDelegate*
    CefContentBrowserClient::GetCryptoPasswordDelegate(const GURL& url) {
  return NULL;
}
#endif
