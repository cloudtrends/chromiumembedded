// Copyright (c) 2008 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CEF_LIBCEF_BROWSER_REQUEST_CONTEXT_H_
#define CEF_LIBCEF_BROWSER_REQUEST_CONTEXT_H_
#pragma once

#include <string>

#include "net/http/http_cache.h"
#include "net/http/url_security_manager.h"
#include "net/url_request/url_request.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_storage.h"

namespace base {
class FilePath;
}

namespace net {
class URLRequestJobFactoryImpl;
}

namespace webkit_blob {
class BlobStorageController;
}

// A basic URLRequestContext that only provides an in-memory cookie store.
class BrowserRequestContext : public net::URLRequestContext {
 public:
  // Use an in-memory cache
  BrowserRequestContext();
  virtual ~BrowserRequestContext();

  // Use an on-disk cache at the specified location.  Optionally, use the cache
  // in playback or record mode.
  BrowserRequestContext(const base::FilePath& cache_path,
                        net::HttpCache::Mode cache_mode,
                        bool no_proxy);

  // Set the path used for cookie storage. If |path| is empty memory only
  // storage will be used. If the old cookie data is being stored on disk it
  // will be flushed and closed.
  void SetCookieStoragePath(const base::FilePath& path);

  webkit_blob::BlobStorageController* blob_storage_controller() const {
    return blob_storage_controller_.get();
  }
  net::URLRequestJobFactoryImpl* job_factory_impl() const {
    return job_factory_impl_;
  };

 private:
  void Init(const base::FilePath& cache_path, net::HttpCache::Mode cache_mode,
            bool no_proxy);

  net::URLRequestContextStorage storage_;
  scoped_ptr<webkit_blob::BlobStorageController> blob_storage_controller_;
  scoped_ptr<net::URLSecurityManager> url_security_manager_;
  scoped_ptr<net::URLRequest::Interceptor> url_request_interceptor_;
  base::FilePath cookie_store_path_;
  net::URLRequestJobFactoryImpl* job_factory_impl_;
};

#endif  // CEF_LIBCEF_BROWSER_REQUEST_CONTEXT_H_

