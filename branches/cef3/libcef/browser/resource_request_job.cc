// Copyright (c) 2012 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2006-2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/browser/resource_request_job.h"

#include <map>

#include "include/cef_callback.h"
#include "libcef/browser/request_impl.h"
#include "libcef/browser/response_impl.h"
#include "libcef/browser/thread_util.h"

#include "base/logging.h"
#include "net/base/io_buffer.h"
#include "net/http/http_response_headers.h"
#include "net/url_request/url_request.h"

using net::URLRequestStatus;

// Client callback for asynchronous response continuation.
class CefResourceRequestJobCallback : public CefCallback {
 public:
  enum Type {
    HEADERS_AVAILABLE,
    BYTES_AVAILABLE,
  };

  explicit CefResourceRequestJobCallback(CefResourceRequestJob* job, Type type)
      : job_(job),
        type_(type),
        dest_(NULL),
        dest_size_(0) {}

  virtual void Continue() OVERRIDE {
    if (CEF_CURRENTLY_ON_IOT()) {
      // Currently on IO thread.
      // Return early if the callback has already been detached.
      if (!job_)
        return;
      
      if (type_ == HEADERS_AVAILABLE) {
        // Callback for headers available.
        if (!job_->has_response_started()) {
          // Send header information.
          job_->SendHeaders();
        }

        // This type of callback only ever needs to be called once.
        Detach();
      } else if (type_ == BYTES_AVAILABLE) {
        // Callback for bytes available.
        if (job_->has_response_started() &&
            job_->GetStatus().is_io_pending()) {
          // Read the bytes. They should be available but, if not, wait again.
          int bytes_read = 0;
          if (job_->ReadRawData(dest_, dest_size_, &bytes_read)) {
            if (bytes_read > 0) {
              // Clear the IO_PENDING status.
              job_->SetStatus(URLRequestStatus());

              // Notify about the available bytes.
              job_->NotifyReadComplete(bytes_read);

              dest_ = NULL;
              dest_size_ = 0;
            }
          } else {
            // All done.
            job_->NotifyDone(URLRequestStatus());
            Detach();
          }
        }
      }
    } else {
      // Execute this method on the IO thread.
      CEF_POST_TASK(CEF_IOT,
          base::Bind(&CefResourceRequestJobCallback::Continue, this));
    }
  }

  virtual void Cancel() OVERRIDE {
    if (CEF_CURRENTLY_ON_IOT()) {
      // Currently on IO thread.
      if (job_)
        job_->Kill();
    } else {
      // Execute this method on the IO thread.
      CEF_POST_TASK(CEF_IOT,
          base::Bind(&CefResourceRequestJobCallback::Cancel, this));
    }
  }

  void Detach() {
    CEF_REQUIRE_IOT();
    job_ = NULL;
  }

  void SetDestination(net::IOBuffer* dest, int dest_size) {
    CEF_REQUIRE_IOT();
    dest_ = dest;
    dest_size_ = dest_size;
  }

 private:
  CefResourceRequestJob* job_;
  Type type_;

  net::IOBuffer* dest_;
  int dest_size_;

  IMPLEMENT_REFCOUNTING(Callback);
};

CefResourceRequestJob::CefResourceRequestJob(
    net::URLRequest* request,
    CefRefPtr<CefResourceHandler> handler)
    : net::URLRequestJob(request),
      handler_(handler),
      remaining_bytes_(0) {
}

CefResourceRequestJob::~CefResourceRequestJob() {
}

void CefResourceRequestJob::Start() {
  CEF_REQUIRE_IOT();

  // Create the callback that will be used to notify when header information is
  // available.
  callback_ = new CefResourceRequestJobCallback(this,
      CefResourceRequestJobCallback::HEADERS_AVAILABLE);

  CefRefPtr<CefRequest> req(CefRequest::CreateRequest());

  // Populate the request data.
  static_cast<CefRequestImpl*>(req.get())->Set(request());

  // Handler can decide whether to process the request.
  bool rv = handler_->ProcessRequest(req, callback_.get());
  if (!rv) {
    // Cancel the request.
    NotifyStartError(URLRequestStatus(URLRequestStatus::FAILED, ERR_ABORTED));
  }

  return;
}

void CefResourceRequestJob::Kill() {
  CEF_REQUIRE_IOT();

  // Notify the handler that the request has been canceled.
  handler_->Cancel();

  if (callback_) {
    callback_->Detach();
    callback_ = NULL;
  }

  net::URLRequestJob::Kill();
}

bool CefResourceRequestJob::ReadRawData(net::IOBuffer* dest, int dest_size,
                                        int* bytes_read) {
  CEF_REQUIRE_IOT();

  DCHECK_NE(dest_size, 0);
  DCHECK(bytes_read);

  if (remaining_bytes_ == 0) {
    // No more data to read.
    *bytes_read = 0;
    return true;
  } else if (remaining_bytes_ > 0 && remaining_bytes_ < dest_size) {
    // The handler knows the content size beforehand.
    dest_size = static_cast<int>(remaining_bytes_);
  }

  if (!callback_.get()) {
    // Create the bytes available callback that will be used until the request
    // is completed.
    callback_ = new CefResourceRequestJobCallback(this,
        CefResourceRequestJobCallback::BYTES_AVAILABLE);
  }

  // Read response data from the handler.
  bool rv = handler_->ReadResponse(dest->data(), dest_size, *bytes_read,
                                   callback_.get());
  if (!rv) {
    // The handler has indicated completion of the request.
    *bytes_read = 0;
    return true;
  } else if (*bytes_read == 0) {
    if (!GetStatus().is_io_pending()) {
      // Report our status as IO pending.
      SetStatus(URLRequestStatus(URLRequestStatus::IO_PENDING, 0));
      callback_->SetDestination(dest, dest_size);
    }
    return false;
  } else if (*bytes_read > dest_size) {
    // Normalize the return value.
    *bytes_read = dest_size;
  }

  if (remaining_bytes_ > 0)
    remaining_bytes_ -= *bytes_read;

  // Continue calling this method.
  return true;
}

void CefResourceRequestJob::GetResponseInfo(net::HttpResponseInfo* info) {
  CEF_REQUIRE_IOT();

  if (response_.get()) {
    CefResponseImpl* responseImpl =
        static_cast<CefResponseImpl*>(response_.get());
    info->headers = responseImpl->GetResponseHeaders();
  }
}

bool CefResourceRequestJob::IsRedirectResponse(GURL* location,
                                               int* http_status_code) {
  CEF_REQUIRE_IOT();

  if (redirect_url_.is_valid()) {
    // Redirect to the new URL.
    *http_status_code = 303;
    location->Swap(&redirect_url_);
    return true;
  }

  if (response_.get()) {
    // Check for HTTP 302 or HTTP 303 redirect.
    int status = response_->GetStatus();
    if (status == 302 || status == 303) {
      CefResponse::HeaderMap headerMap;
      response_->GetHeaderMap(headerMap);
      CefRequest::HeaderMap::iterator iter = headerMap.find("Location");
      if (iter != headerMap.end()) {
          GURL new_url = GURL(std::string(iter->second));
          *http_status_code = status;
          location->Swap(&new_url);
          return true;
      }
    }
  }

  return false;
}

bool CefResourceRequestJob::GetMimeType(std::string* mime_type) const {
  CEF_REQUIRE_IOT();

  if (response_.get())
    *mime_type = response_->GetMimeType();
  return true;
}

void CefResourceRequestJob::SendHeaders() {
  CEF_REQUIRE_IOT();

  // Clear the headers available callback.
  callback_ = NULL;

  // We may have been orphaned...
  if (!request())
    return;

  response_ = new CefResponseImpl();
  remaining_bytes_ = 0;

  CefString redirectUrl;

  // Get header information from the handler.
  handler_->GetResponseHeaders(response_, remaining_bytes_, redirectUrl);
  if (!redirectUrl.empty()) {
    std::string redirectUrlStr = redirectUrl;
    redirect_url_ = GURL(redirectUrlStr);
  }

  if (remaining_bytes_ > 0)
    set_expected_content_size(remaining_bytes_);

  // Continue processing the request.
  NotifyHeadersComplete();
}
