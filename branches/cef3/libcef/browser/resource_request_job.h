// Copyright (c) 2012 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2006-2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CEF_LIBCEF_BROWSER_RESOURCE_REQUEST_JOB_H_
#define CEF_LIBCEF_BROWSER_RESOURCE_REQUEST_JOB_H_

#include <string>

#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_request_handler.h"

#include "net/url_request/url_request_job.h"

namespace net {
class URLRequest;
}

class CefResourceRequestJobCallback;

class CefResourceRequestJob : public net::URLRequestJob {
 public:
  CefResourceRequestJob(net::URLRequest* request,
                        CefRefPtr<CefResourceHandler> handler);
  virtual ~CefResourceRequestJob();

  // net::URLRequestJob methods.
  virtual void Start() OVERRIDE;
  virtual void Kill() OVERRIDE;
  virtual bool ReadRawData(net::IOBuffer* dest, int dest_size, int* bytes_read)
      OVERRIDE;
  virtual void GetResponseInfo(net::HttpResponseInfo* info) OVERRIDE;
  virtual bool IsRedirectResponse(GURL* location, int* http_status_code)
      OVERRIDE;
  virtual bool GetMimeType(std::string* mime_type) const OVERRIDE;

 private:
  void SendHeaders();

  CefRefPtr<CefResourceHandler> handler_;
  CefRefPtr<CefResponse> response_;
  GURL redirect_url_;
  int64 remaining_bytes_;
  CefRefPtr<CefResourceRequestJobCallback> callback_;

  friend class CefResourceRequestJobCallback;

  DISALLOW_COPY_AND_ASSIGN(CefResourceRequestJob);
};

#endif  // CEF_LIBCEF_BROWSER_RESOURCE_REQUEST_JOB_H_
