// Copyright (c) 2008-2009 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/compiler_specific.h"

#include "third_party/WebKit/Source/WebCore/config.h"
MSVC_PUSH_WARNING_LEVEL(0);
#include "ApplicationCacheStorage.h"
#include "CrossOriginPreflightResultCache.h"
#include "DocumentLoader.h"
#include "MemoryCache.h"
#include "TextEncoding.h"
#include "third_party/WebKit/Source/WebKit/chromium/src/WebFrameImpl.h"
MSVC_POP_WARNING();
#undef LOG

#include "browser_webkit_glue.h"
#include "cef_context.h"

#include "base/file_util.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/path_service.h"
#include "base/string_util.h"
#include "base/string16.h"
#include "net/base/mime_util.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebString.h"
#include "ui/base/resource/resource_bundle.h"
#include "webkit/glue/user_agent.h"
#include "webkit/glue/webkit_glue.h"
#include "webkit/plugins/npapi/plugin_list.h"

// Generated by GRIT
#include "grit/webkit_resources.h"

using WebKit::WebFrameImpl;

namespace webkit_glue {

void InitializeResourceBundle(const std::string& locale) {
  // Load chrome.pak (on Mac) and the appropiate locale pack.
  const std::string loaded_locale =
      ResourceBundle::InitSharedInstance(locale);
  CHECK(!loaded_locale.empty()) << "Locale could not be found for " << locale;

#if defined(OS_WIN)
  // Explicitly load chrome.pak on Windows. Use the module (libcef.dll)
  // directory to match the location of the locale folder.
  FilePath chrome_pack_path;
  PathService::Get(base::DIR_MODULE, &chrome_pack_path);
  chrome_pack_path = chrome_pack_path.AppendASCII("chrome.pak");
  if (file_util::PathExists(chrome_pack_path))
    ResourceBundle::AddDataPackToSharedInstance(chrome_pack_path);
  else
    NOTREACHED() << "Could not load chrome.pak";
#endif
}

void CleanupResourceBundle() {
  ResourceBundle::CleanupSharedInstance();
}

string16 GetLocalizedString(int message_id) {
  return ResourceBundle::GetSharedInstance().GetLocalizedString(message_id);
}

bool IsMediaPlayerAvailable() {
  return true;
}

void GetPlugins(bool refresh,
                std::vector<webkit::WebPluginInfo>* plugins) {
  if (refresh)
    webkit::npapi::PluginList::Singleton()->RefreshPlugins();
  webkit::npapi::PluginList::Singleton()->GetPlugins(plugins);
}

bool IsProtocolSupportedForMedia(const GURL& url) {
  return true;
}

void InitializeTextEncoding() {
  WebCore::UTF8Encoding();
}

v8::Handle<v8::Context> GetV8Context(WebKit::WebFrame* frame)
{
  WebFrameImpl* webFrameImpl = static_cast<WebFrameImpl*>(frame);
  WebCore::Frame* core_frame = webFrameImpl->frame();
  return WebCore::V8Proxy::context(core_frame);
}

void ClearCache()
{
  if (WebCore::memoryCache()->disabled())
    return;

  // Clear the memory cache by disabling and then re-enabling it.
  WebCore::memoryCache()->setDisabled(true);
  WebCore::memoryCache()->setDisabled(false);

  // Empty the Cross-Origin Preflight cache
  WebCore::CrossOriginPreflightResultCache::shared().empty();
}

#if defined(OS_LINUX)
int MatchFontWithFallback(const std::string& face, bool bold,
                          bool italic, int charset) {
  return -1;
}

bool GetFontTable(int fd, uint32_t table, uint8_t* output,
                  size_t* output_length) {
  return false;
}
#endif

// Adapted from Chromium's BufferedResourceHandler::ShouldDownload
bool ShouldDownload(const std::string& content_disposition,
                    const std::string& mime_type)
{
  std::string type = StringToLowerASCII(mime_type);
  std::string disposition = StringToLowerASCII(content_disposition);

  // First, examine content-disposition.
  if (!disposition.empty()) {
    bool should_download = true;

    // Some broken sites just send ...
    //    Content-Disposition: ; filename="file"
    // ... screen those out here.
    if (disposition[0] == ';')
      should_download = false;

    if (disposition.compare(0, 6, "inline") == 0)
      should_download = false;

    // Some broken sites just send ...
    //    Content-Disposition: filename="file"
    // ... without a disposition token... Screen those out.
    if (disposition.compare(0, 8, "filename") == 0)
      should_download = false;

    // Also in use is Content-Disposition: name="file"
    if (disposition.compare(0, 4, "name") == 0)
      should_download = false;

    // We have a content-disposition of "attachment" or unknown.
    // RFC 2183, section 2.8 says that an unknown disposition
    // value should be treated as "attachment".
    if (should_download)
      return true;
  }

  // Mirrors WebViewImpl::CanShowMIMEType()
  if (type.empty() || net::IsSupportedMimeType(type))
    return false;

  // Finally, check the plugin list.
  bool allow_wildcard = false;
  std::vector<webkit::WebPluginInfo> plugins;
  webkit::npapi::PluginList::Singleton()->GetPluginInfoArray(
      GURL(), type, allow_wildcard, NULL, &plugins, NULL);
  
  // If any associated plugins exist and are enabled don't allow the download.
  if (!plugins.empty()) {
    std::vector<webkit::WebPluginInfo>::const_iterator it = plugins.begin();
    for (; it != plugins.end(); ++it) {
      if (webkit_glue::IsPluginEnabled(*it))
        return false;
    }
  }

  return true;
}

bool IsPluginEnabled(const webkit::WebPluginInfo& plugin) {
  return true;
}

}  // namespace webkit_glue
