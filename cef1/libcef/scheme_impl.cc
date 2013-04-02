// Copyright (c) 2011 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2006-2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>

#include "include/cef_scheme.h"
#include "libcef/browser_devtools_scheme_handler.h"
#include "libcef/browser_impl.h"
#include "libcef/browser_resource_loader_bridge.h"
#include "libcef/cef_context.h"
#include "libcef/cef_thread.h"
#include "libcef/request_impl.h"
#include "libcef/response_impl.h"

#include "base/bind.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/message_loop.h"
#include "base/string_util.h"
#include "base/synchronization/lock.h"
#include "googleurl/src/url_util.h"
#include "net/base/completion_callback.h"
#include "net/base/load_flags.h"
#include "net/cookies/cookie_monster.h"
#include "net/base/io_buffer.h"
#include "net/base/upload_data.h"
#include "net/http/http_response_headers.h"
#include "net/http/http_util.h"
#include "net/url_request/http_user_agent_settings.h"
#include "net/url_request/url_request.h"
#include "net/url_request/url_request_about_job.h"
#include "net/url_request/url_request_data_job.h"
#include "net/url_request/url_request_error_job.h"
#include "net/url_request/url_request_file_job.h"
#include "net/url_request/url_request_filter.h"
#include "net/url_request/url_request_ftp_job.h"
#include "net/url_request/url_request_http_job.h"
#include "net/url_request/url_request_job.h"
#include "net/url_request/url_request_job_factory_impl.h"
#include "third_party/WebKit/Source/Platform/chromium/public/WebString.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebSecurityPolicy.h"

using net::URLRequestStatus;
using WebKit::WebSecurityPolicy;
using WebKit::WebString;

namespace {

bool IsStandardScheme(const std::string& scheme) {
  url_parse::Component scheme_comp(0, scheme.length());
  return url_util::IsStandard(scheme.c_str(), scheme_comp);
}

// Copied from net/url_request/url_request_job_manager.cc.
struct SchemeToFactory {
  const char* scheme;
  net::URLRequest::ProtocolFactory* factory;
};
static const SchemeToFactory kBuiltinFactories[] = {
  { "http", net::URLRequestHttpJob::Factory },
  { "https", net::URLRequestHttpJob::Factory },
  { "file", net::URLRequestFileJob::Factory },
  { "ftp", net::URLRequestFtpJob::Factory },
  { "about", net::URLRequestAboutJob::Factory },
  { "data", net::URLRequestDataJob::Factory },
};

bool IsBuiltinScheme(const std::string& scheme) {
  for (size_t i = 0; i < arraysize(kBuiltinFactories); ++i)
    if (LowerCaseEqualsASCII(scheme, kBuiltinFactories[i].scheme))
      return true;
  return false;
}

net::URLRequestJob* GetBuiltinSchemeRequestJob(
    net::URLRequest* request,
    net::NetworkDelegate* network_delegate,
    const std::string& scheme) {
  // See if the request should be handled by a built-in protocol factory.
  for (size_t i = 0; i < arraysize(kBuiltinFactories); ++i) {
    if (scheme == kBuiltinFactories[i].scheme) {
      net::URLRequestJob* job =
          (kBuiltinFactories[i].factory)(request, network_delegate, scheme);
      DCHECK(job);  // The built-in factories are not expected to fail!
      return job;
    }
  }

  return NULL;
}

bool SetHeaderIfMissing(CefRequest::HeaderMap& headerMap,
                        const std::string& name,
                        const std::string& value) {
  if (value.empty())
    return false;
                              
  CefRequest::HeaderMap::const_iterator it = headerMap.find(name);
  if (it == headerMap.end()) {
    headerMap.insert(std::make_pair(name, value));
    return true;
  }

  return false;
}


// net::URLRequestJob implementation.
class CefUrlRequestJob : public net::URLRequestJob {
 public:
  CefUrlRequestJob(net::URLRequest* request,
                   net::NetworkDelegate* network_delegate,
                   CefRefPtr<CefSchemeHandler> handler)
    : net::URLRequestJob(request, network_delegate),
      handler_(handler),
      remaining_bytes_(0),
      response_cookies_save_index_(0),
      ALLOW_THIS_IN_INITIALIZER_LIST(weak_factory_(this)) {
  }

  virtual ~CefUrlRequestJob() {
  }

  virtual void Start() OVERRIDE {
    REQUIRE_IOT();

    cef_request_ = CefRequest::CreateRequest();

    // Populate the request data.
    static_cast<CefRequestImpl*>(cef_request_.get())->Set(request_);

    // Add default headers if not already specified.
    const net::URLRequestContext* context = request_->context();
    if (context) {
      CefRequest::HeaderMap headerMap;
      cef_request_->GetHeaderMap(headerMap);
      bool changed = false;

      const net::HttpUserAgentSettings* ua_settings =
          context->http_user_agent_settings();
      if (ua_settings) {
        if (SetHeaderIfMissing(headerMap,
                               net::HttpRequestHeaders::kAcceptLanguage,
                               ua_settings->GetAcceptLanguage())) {
          changed = true;
        }

        if (SetHeaderIfMissing(headerMap,
                               net::HttpRequestHeaders::kUserAgent,
                               ua_settings->GetUserAgent(request_->url()))) {
          changed = true;
        }
      }

      if (changed)
        cef_request_->SetHeaderMap(headerMap);
    }

    AddCookieHeaderAndStart();
  }

  void AddCookieHeaderAndStart() {
    // No matter what, we want to report our status as IO pending since we will
    // be notifying our consumer asynchronously via OnStartCompleted.
    SetStatus(URLRequestStatus(URLRequestStatus::IO_PENDING, 0));

    // If the request was destroyed, then there is no more work to do.
    if (!request_)
      return;

    net::CookieStore* cookie_store =
        request_->context()->cookie_store();
    if (cookie_store &&
        !(request_->load_flags() & net::LOAD_DO_NOT_SEND_COOKIES)) {
      net::CookieMonster* cookie_monster = cookie_store->GetCookieMonster();
      if (cookie_monster) {
        cookie_monster->GetAllCookiesForURLAsync(
            request_->url(),
            base::Bind(&CefUrlRequestJob::CheckCookiePolicyAndLoad,
                       weak_factory_.GetWeakPtr()));
      } else {
        DoLoadCookies();
      }
    } else {
      DoStartTransaction();
    }
  }

  void DoLoadCookies() {
    net::CookieOptions options;
    options.set_include_httponly();
    request_->context()->cookie_store()->GetCookiesWithOptionsAsync(
        request_->url(), options,
        base::Bind(&CefUrlRequestJob::OnCookiesLoaded,
                   weak_factory_.GetWeakPtr()));
  }

  void CheckCookiePolicyAndLoad(
      const net::CookieList& cookie_list) {
    if (CanGetCookies(cookie_list))
      DoLoadCookies();
    else
      DoStartTransaction();
  }

  void OnCookiesLoaded(
      const std::string& cookie_line) {
    if (!cookie_line.empty()) {
      CefRequest::HeaderMap headerMap;
      cef_request_->GetHeaderMap(headerMap);
      headerMap.insert(
          std::make_pair(net::HttpRequestHeaders::kCookie, cookie_line));
      cef_request_->SetHeaderMap(headerMap);
    }
    DoStartTransaction();
  }

  void DoStartTransaction() {
    // We may have been canceled while retrieving cookies.
    if (GetStatus().is_success()) {
      StartTransaction();
    } else {
      NotifyCanceled();
    }
  }

  void StartTransaction() {
    if (!callback_)
      callback_ = new Callback(this);

    // Protect against deletion of this object.
    base::WeakPtr<CefUrlRequestJob> weak_ptr(weak_factory_.GetWeakPtr());

    // Handler can decide whether to process the request.
    bool rv = handler_->ProcessRequest(cef_request_, callback_.get());
    if (weak_ptr.get() && !rv) {
      // Cancel the request.
      NotifyCanceled();
    }
  }

  virtual void Kill() OVERRIDE {
    REQUIRE_IOT();

    // Notify the handler that the request has been canceled.
    handler_->Cancel();

    if (callback_) {
      callback_->Detach();
      callback_ = NULL;
    }

    net::URLRequestJob::Kill();
  }

  virtual bool ReadRawData(net::IOBuffer* dest, int dest_size, int *bytes_read)
      OVERRIDE {
    REQUIRE_IOT();

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

  virtual void GetResponseInfo(net::HttpResponseInfo* info) OVERRIDE {
    REQUIRE_IOT();

    info->headers = GetResponseHeaders();
  }

  virtual bool IsRedirectResponse(GURL* location, int* http_status_code)
      OVERRIDE {
    REQUIRE_IOT();

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

  virtual bool GetMimeType(std::string* mime_type) const OVERRIDE {
    REQUIRE_IOT();

    if (response_.get())
      *mime_type = response_->GetMimeType();
    return true;
  }

  CefRefPtr<CefSchemeHandler> handler_;
  CefRefPtr<CefResponse> response_;

 private:
  void SendHeaders() {
    REQUIRE_IOT();

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
    SaveCookiesAndNotifyHeadersComplete();
  }

  net::HttpResponseHeaders* GetResponseHeaders() {
    DCHECK(response_);
    if (!response_headers_.get()) {
      CefResponseImpl* responseImpl =
          static_cast<CefResponseImpl*>(response_.get());
      response_headers_ = responseImpl->GetResponseHeaders();
    }
    return response_headers_;
  }

  void SaveCookiesAndNotifyHeadersComplete() {
    if (request_->load_flags() & net::LOAD_DO_NOT_SAVE_COOKIES) {
      SetStatus(URLRequestStatus());  // Clear the IO_PENDING status
      NotifyHeadersComplete();
      return;
    }

    response_cookies_.clear();
    response_cookies_save_index_ = 0;

    FetchResponseCookies(&response_cookies_);

    // Now, loop over the response cookies, and attempt to persist each.
    SaveNextCookie();
  }

  void SaveNextCookie() {
    if (response_cookies_save_index_ == response_cookies_.size()) {
      response_cookies_.clear();
      response_cookies_save_index_ = 0;
      SetStatus(URLRequestStatus());  // Clear the IO_PENDING status
      NotifyHeadersComplete();
      return;
    }

    // No matter what, we want to report our status as IO pending since we will
    // be notifying our consumer asynchronously via OnStartCompleted.
    SetStatus(URLRequestStatus(URLRequestStatus::IO_PENDING, 0));

    net::CookieOptions options;
    options.set_include_httponly();
    if (CanSetCookie(
        response_cookies_[response_cookies_save_index_], &options)) {
      request_->context()->cookie_store()->SetCookieWithOptionsAsync(
          request_->url(), response_cookies_[response_cookies_save_index_],
          options, base::Bind(&CefUrlRequestJob::OnCookieSaved,
                              weak_factory_.GetWeakPtr()));
      return;
    }

    CookieHandled();
  }

  void OnCookieSaved(bool cookie_status) {
    CookieHandled();
  }

  void CookieHandled() {
    response_cookies_save_index_++;
    // We may have been canceled within OnSetCookie.
    if (GetStatus().is_success()) {
      SaveNextCookie();
    } else {
      NotifyCanceled();
    }
  }

  void FetchResponseCookies(
      std::vector<std::string>* cookies) {
    const std::string name = "Set-Cookie";
    std::string value;

    void* iter = NULL;
    net::HttpResponseHeaders* headers = GetResponseHeaders();
    while (headers->EnumerateHeader(&iter, name, &value)) {
      if (!value.empty())
        cookies->push_back(value);
    }
  }

  // Client callback for asynchronous response continuation.
  class Callback : public CefSchemeHandlerCallback {
   public:
    explicit Callback(CefUrlRequestJob* job)
      : job_(job),
        dest_(NULL),
        dest_size_() {}

    virtual void HeadersAvailable() OVERRIDE {
      if (CefThread::CurrentlyOn(CefThread::IO)) {
        // Currently on IO thread.
        if (job_ && !job_->has_response_started()) {
          // Send header information.
          job_->SendHeaders();
        }
      } else {
        // Execute this method on the IO thread.
        CefThread::PostTask(CefThread::IO, FROM_HERE,
            base::Bind(&Callback::HeadersAvailable, this));
      }
    }

    virtual void BytesAvailable() OVERRIDE {
      if (CefThread::CurrentlyOn(CefThread::IO)) {
        // Currently on IO thread.
        if (job_ && job_->has_response_started() &&
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
            } else {
              // All done.
              job_->NotifyDone(URLRequestStatus());
            }
          } else if (!job_->GetStatus().is_io_pending()) {
            // Failed due to an error.
            NOTREACHED() <<
                "ReadRawData returned false without setting IO as pending";
            job_->NotifyDone(URLRequestStatus());
          }
        }
      } else {
        // Execute this method on the IO thread.
        CefThread::PostTask(CefThread::IO, FROM_HERE,
            base::Bind(&Callback::BytesAvailable, this));
      }
    }

    virtual void Cancel() OVERRIDE {
      if (CefThread::CurrentlyOn(CefThread::IO)) {
        // Currently on IO thread.
        if (job_)
          job_->Kill();
      } else {
        // Execute this method on the IO thread.
        CefThread::PostTask(CefThread::IO, FROM_HERE,
            base::Bind(&Callback::Cancel, this));
      }
    }

    void Detach() {
      REQUIRE_IOT();
      job_ = NULL;
    }

    void SetDestination(net::IOBuffer* dest, int dest_size) {
      dest_ = dest;
      dest_size_ = dest_size;
    }

    static bool ImplementsThreadSafeReferenceCounting() { return true; }

   private:
    CefUrlRequestJob* job_;

    net::IOBuffer* dest_;
    int dest_size_;

    IMPLEMENT_REFCOUNTING(Callback);
  };

  GURL redirect_url_;
  int64 remaining_bytes_;
  CefRefPtr<CefRequest> cef_request_;
  CefRefPtr<Callback> callback_;
  scoped_refptr<net::HttpResponseHeaders> response_headers_;
  std::vector<std::string> response_cookies_;
  size_t response_cookies_save_index_;
  base::WeakPtrFactory<CefUrlRequestJob> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(CefUrlRequestJob);
};


// Class that manages the CefSchemeHandlerFactory instances.
class CefUrlRequestManager {
 protected:
  // Class used for creating URLRequestJob instances. The lifespan of this
  // object is managed by URLRequestJobFactory.
  class ProtocolHandler : public net::URLRequestJobFactory::ProtocolHandler {
   public:
    explicit ProtocolHandler(const std::string& scheme)
      : scheme_(scheme) {}

    // From net::URLRequestJobFactory::ProtocolHandler
    virtual net::URLRequestJob* MaybeCreateJob(
        net::URLRequest* request,
        net::NetworkDelegate* network_delegate) const OVERRIDE {
      REQUIRE_IOT();
      return CefUrlRequestManager::GetInstance()->GetRequestJob(
          request, network_delegate, scheme_);
    }

   private:
    std::string scheme_;
  };

 public:
  CefUrlRequestManager() {}

  // Retrieve the singleton instance.
  static CefUrlRequestManager* GetInstance();

  bool AddFactory(const std::string& scheme,
                  const std::string& domain,
                  CefRefPtr<CefSchemeHandlerFactory> factory) {
    if (!factory.get()) {
      RemoveFactory(scheme, domain);
      return true;
    }

    REQUIRE_IOT();

    std::string scheme_lower = StringToLowerASCII(scheme);
    std::string domain_lower = StringToLowerASCII(domain);

    // Hostname is only supported for standard schemes.
    if (!IsStandardScheme(scheme_lower))
      domain_lower.clear();

    handler_map_[make_pair(scheme_lower, domain_lower)] = factory;

    net::URLRequestJobFactoryImpl* job_factory =
        _Context->request_context()->job_factory_impl();

    job_factory->SetProtocolHandler(scheme_lower,
                                    new ProtocolHandler(scheme_lower));

    return true;
  }

  void RemoveFactory(const std::string& scheme,
                     const std::string& domain) {
    REQUIRE_IOT();

    std::string scheme_lower = StringToLowerASCII(scheme);
    std::string domain_lower = StringToLowerASCII(domain);

    // Hostname is only supported for standard schemes.
    if (!IsStandardScheme(scheme_lower))
      domain_lower.clear();

    HandlerMap::iterator iter =
        handler_map_.find(make_pair(scheme_lower, domain_lower));
    if (iter != handler_map_.end())
      handler_map_.erase(iter);
  }

  // Clear all the existing URL handlers and unregister the ProtocolFactory.
  void ClearFactories() {
    REQUIRE_IOT();

    net::URLRequestJobFactoryImpl* job_factory =
        _Context->request_context()->job_factory_impl();

    // Unregister with the ProtocolFactory.
    std::set<std::string> schemes;
    for (HandlerMap::const_iterator i = handler_map_.begin();
        i != handler_map_.end(); ++i) {
      schemes.insert(i->first.first);
    }
    for (std::set<std::string>::const_iterator scheme = schemes.begin();
        scheme != schemes.end(); ++scheme) {
      job_factory->SetProtocolHandler(*scheme, NULL);
    }

    handler_map_.clear();
  }

 private:
  // Retrieve the matching handler factory, if any. |scheme| will already be in
  // lower case.
  CefRefPtr<CefSchemeHandlerFactory> GetHandlerFactory(
      net::URLRequest* request, const std::string& scheme) {
    CefRefPtr<CefSchemeHandlerFactory> factory;

    if (request->url().is_valid() && IsStandardScheme(scheme)) {
      // Check for a match with a domain first.
      const std::string& domain = request->url().host();

      HandlerMap::iterator i = handler_map_.find(make_pair(scheme, domain));
      if (i != handler_map_.end())
        factory = i->second;
    }

    if (!factory.get()) {
      // Check for a match with no specified domain.
      HandlerMap::iterator i =
          handler_map_.find(make_pair(scheme, std::string()));
      if (i != handler_map_.end())
        factory = i->second;
    }

    return factory;
  }

  // Create the job that will handle the request. |scheme| will already be in
  // lower case.
  net::URLRequestJob* GetRequestJob(net::URLRequest* request,
                                    net::NetworkDelegate* network_delegate,
                                    const std::string& scheme) {
    net::URLRequestJob* job = NULL;
    CefRefPtr<CefSchemeHandlerFactory> factory =
        GetHandlerFactory(request, scheme);
    if (factory) {
      // Call the handler factory to create the handler for the request.
      CefRefPtr<CefRequest> requestPtr(new CefRequestImpl());
      static_cast<CefRequestImpl*>(requestPtr.get())->Set(request);
      CefRefPtr<CefBrowserImpl> browser =
          BrowserResourceLoaderBridge::GetBrowserForRequest(request);
      CefRefPtr<CefSchemeHandler> handler =
          factory->Create(browser.get(), scheme, requestPtr);
      if (handler.get())
        job = new CefUrlRequestJob(request, network_delegate, handler);
    }

    if (!job && IsBuiltinScheme(scheme)) {
      // Give the built-in scheme handler a chance to handle the request.
      job = GetBuiltinSchemeRequestJob(request, network_delegate, scheme);
    }

#ifndef NDEBUG
    if (job)
      DLOG(INFO) << "CefUrlRequestManager hit for " << request->url().spec();
#endif

    return job;
  }

  // Map (scheme, domain) to factories. This map will only be accessed on the IO
  // thread.
  typedef std::map<std::pair<std::string, std::string>,
      CefRefPtr<CefSchemeHandlerFactory> > HandlerMap;
  HandlerMap handler_map_;

  // Set of registered schemes. This set may be accessed from multiple threads.
  typedef std::set<std::string> SchemeSet;
  SchemeSet scheme_set_;
  base::Lock scheme_set_lock_;

  DISALLOW_EVIL_CONSTRUCTORS(CefUrlRequestManager);
};

base::LazyInstance<CefUrlRequestManager> g_manager = LAZY_INSTANCE_INITIALIZER;

CefUrlRequestManager* CefUrlRequestManager::GetInstance() {
  return g_manager.Pointer();
}

}  // namespace


bool CefRegisterSchemeHandlerFactory(
    const CefString& scheme_name,
    const CefString& domain_name,
    CefRefPtr<CefSchemeHandlerFactory> factory) {
  // Verify that the context is in a valid state.
  if (!CONTEXT_STATE_VALID()) {
    NOTREACHED() << "context not valid";
    return false;
  }

  if (CefThread::CurrentlyOn(CefThread::IO)) {
    return CefUrlRequestManager::GetInstance()->AddFactory(scheme_name,
                                                           domain_name,
                                                           factory);
  } else {
    CefThread::PostTask(CefThread::IO, FROM_HERE,
        base::Bind(base::IgnoreResult(&CefRegisterSchemeHandlerFactory),
                   scheme_name, domain_name, factory));
    return true;
  }
}

bool CefClearSchemeHandlerFactories() {
  // Verify that the context is in a valid state.
  if (!CONTEXT_STATE_VALID()) {
    NOTREACHED() << "context not valid";
    return false;
  }

  if (CefThread::CurrentlyOn(CefThread::IO)) {
    CefUrlRequestManager::GetInstance()->ClearFactories();

    // Re-register the DevTools scheme handler.
    RegisterDevToolsSchemeHandler();
  } else {
    CefThread::PostTask(CefThread::IO, FROM_HERE,
        base::Bind(base::IgnoreResult(&CefClearSchemeHandlerFactories)));
  }

  return true;
}
