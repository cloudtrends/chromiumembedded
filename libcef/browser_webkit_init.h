// Copyright (c) 2011 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _BROWSER_WEBKIT_INIT_H
#define _BROWSER_WEBKIT_INIT_H

#include "browser_appcache_system.h"
#include "browser_database_system.h"
#include "browser_file_system.h"
#include "browser_webblobregistry_impl.h"
#include "browser_webcookiejar_impl.h"

#include "base/scoped_temp_dir.h"
#include "webkit/glue/simple_webmimeregistry_impl.h"
#include "webkit/glue/webclipboard_impl.h"
#include "webkit/glue/webfileutilities_impl.h"
#include "webkit/glue/webkitplatformsupport_impl.h"


class BrowserWebKitInit : public webkit_glue::WebKitPlatformSupportImpl {
 public:
  BrowserWebKitInit();
  virtual ~BrowserWebKitInit();

  virtual WebKit::WebMimeRegistry* mimeRegistry() OVERRIDE;
  virtual WebKit::WebClipboard* clipboard() OVERRIDE;
  virtual WebKit::WebFileUtilities* fileUtilities() OVERRIDE;
  virtual WebKit::WebSandboxSupport* sandboxSupport() OVERRIDE;
  virtual WebKit::WebBlobRegistry* blobRegistry() OVERRIDE;
  virtual WebKit::WebCookieJar* cookieJar() OVERRIDE;
  virtual WebKit::WebFileSystem* fileSystem() OVERRIDE ;
  virtual bool sandboxEnabled() OVERRIDE;
  virtual WebKit::WebKitPlatformSupport::FileHandle databaseOpenFile(
      const WebKit::WebString& vfs_file_name, int desired_flags) OVERRIDE;
  virtual int databaseDeleteFile(const WebKit::WebString& vfs_file_name,
                                 bool sync_dir) OVERRIDE;
  virtual long databaseGetFileAttributes(
      const WebKit::WebString& vfs_file_name) OVERRIDE;
  virtual long long databaseGetFileSize(
      const WebKit::WebString& vfs_file_name) OVERRIDE;
  virtual long long databaseGetSpaceAvailableForOrigin(
      const WebKit::WebString& origin_identifier) OVERRIDE;
  virtual unsigned long long visitedLinkHash(const char* canonicalURL,
                                             size_t length) OVERRIDE;
  virtual bool isLinkVisited(unsigned long long linkHash) OVERRIDE;
  virtual WebKit::WebMessagePortChannel* createMessagePortChannel() OVERRIDE;
  virtual void prefetchHostName(const WebKit::WebString&) OVERRIDE;
  virtual void decrementStatsCounter(const char* name) OVERRIDE;
  virtual void incrementStatsCounter(const char* name) OVERRIDE;
  virtual void histogramCustomCounts(const char* name, int sample, int min, 
                                     int max, int bucket_count) OVERRIDE;
  virtual void histogramEnumeration(const char* name, int sample,
                                    int boundary_value) OVERRIDE;
  virtual bool isTraceEventEnabled() const OVERRIDE;
  virtual void traceEventBegin(const char* name, void* id,
                               const char* extra) OVERRIDE;
  virtual void traceEventEnd(const char* name, void* id, 
                             const char* extra) OVERRIDE;
  virtual WebKit::WebData loadResource(const char* name) OVERRIDE;
  virtual WebKit::WebString defaultLocale() OVERRIDE;
  virtual WebKit::WebStorageNamespace* createLocalStorageNamespace(
      const WebKit::WebString& path, unsigned quota) OVERRIDE;
  void dispatchStorageEvent(const WebKit::WebString& key,
      const WebKit::WebString& old_value, const WebKit::WebString& new_value,
      const WebKit::WebString& origin, const WebKit::WebURL& url,
      bool is_local_storage) OVERRIDE;
  virtual WebKit::WebIDBFactory* idbFactory() OVERRIDE;
  virtual void createIDBKeysFromSerializedValuesAndKeyPath(
      const WebKit::WebVector<WebKit::WebSerializedScriptValue>& values,
      const WebKit::WebString& keyPath,
      WebKit::WebVector<WebKit::WebIDBKey>& keys_out) OVERRIDE;
  virtual WebKit::WebSerializedScriptValue injectIDBKeyIntoSerializedValue(
      const WebKit::WebIDBKey& key,
      const WebKit::WebSerializedScriptValue& value,
      const WebKit::WebString& keyPath) OVERRIDE;
  virtual WebKit::WebGraphicsContext3D* createGraphicsContext3D() OVERRIDE;
  virtual WebKit::WebString queryLocalizedString(
      WebKit::WebLocalizedString::Name name) OVERRIDE;
  virtual WebKit::WebString queryLocalizedString(
      WebKit::WebLocalizedString::Name name, const WebKit::WebString& value)
      OVERRIDE;
  virtual WebKit::WebString queryLocalizedString(
      WebKit::WebLocalizedString::Name name,
      const WebKit::WebString& value1,
      const WebKit::WebString& value2) OVERRIDE;

 private:
  webkit_glue::SimpleWebMimeRegistryImpl mime_registry_;
  webkit_glue::WebClipboardImpl clipboard_;
  webkit_glue::WebFileUtilitiesImpl file_utilities_;
  ScopedTempDir appcache_dir_;
  BrowserAppCacheSystem appcache_system_;
  BrowserDatabaseSystem database_system_;
  BrowserWebCookieJarImpl cookie_jar_;
  scoped_refptr<BrowserWebBlobRegistryImpl> blob_registry_;
  BrowserFileSystem file_system_;
};

#endif  // _BROWSER_WEBKIT_INIT_H
