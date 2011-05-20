// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// ---------------------------------------------------------------------------
//
// A portion of this file was generated by the CEF translator tool.  When
// making changes by hand only do so within the body of existing function
// implementations. See the translator.README.txt file in the tools directory
// for more information.
//

#include "libcef_dll/cpptoc/download_handler_cpptoc.h"
#include "libcef_dll/cpptoc/request_handler_cpptoc.h"
#include "libcef_dll/ctocpp/browser_ctocpp.h"
#include "libcef_dll/ctocpp/frame_ctocpp.h"
#include "libcef_dll/ctocpp/request_ctocpp.h"
#include "libcef_dll/ctocpp/response_ctocpp.h"
#include "libcef_dll/ctocpp/stream_reader_ctocpp.h"


// MEMBER FUNCTIONS - Body may be edited by hand.

int CEF_CALLBACK request_handler_on_before_browse(
    struct _cef_request_handler_t* self, cef_browser_t* browser,
    cef_frame_t* frame, struct _cef_request_t* request,
    enum cef_handler_navtype_t navType, int isRedirect)
{
  DCHECK(self);
  DCHECK(browser);
  DCHECK(frame);
  DCHECK(request);
  if (!self || !browser || !frame || !request)
    return 0;

  return CefRequestHandlerCppToC::Get(self)->OnBeforeBrowse(
      CefBrowserCToCpp::Wrap(browser), CefFrameCToCpp::Wrap(frame),
      CefRequestCToCpp::Wrap(request), navType, (isRedirect ? true : false));
}

int CEF_CALLBACK request_handler_on_before_resource_load(
    struct _cef_request_handler_t* self, cef_browser_t* browser,
    struct _cef_request_t* request, cef_string_t* redirectUrl,
    struct _cef_stream_reader_t** resourceStream,
    struct _cef_response_t* response, int loadFlags)
{
  DCHECK(self);
  DCHECK(browser);
  DCHECK(request);
  DCHECK(redirectUrl);
  DCHECK(resourceStream);
  DCHECK(response);
  if (!self || !browser || !request || !redirectUrl || !resourceStream ||
      !response)
    return 0;

  CefRefPtr<CefStreamReader> streamPtr;

  CefString redirectUrlStr(redirectUrl);
  int rv = CefRequestHandlerCppToC::Get(self)->OnBeforeResourceLoad(
      CefBrowserCToCpp::Wrap(browser), CefRequestCToCpp::Wrap(request),
      redirectUrlStr, streamPtr, CefResponseCToCpp::Wrap(response), loadFlags);

  if(streamPtr.get())
    *resourceStream = CefStreamReaderCToCpp::Unwrap(streamPtr);

  return rv;
}

int CEF_CALLBACK request_handler_on_protocol_execution(
    struct _cef_request_handler_t* self, cef_browser_t* browser,
    const cef_string_t* url, int* allowOSExecution)
{
  DCHECK(self);
  DCHECK(browser);
  DCHECK(url);
  DCHECK(allowOSExecution);
  if (!self || !browser || !url || !allowOSExecution)
    return 0;

  bool allowExec = *allowOSExecution?true:false;
  int rv = CefRequestHandlerCppToC::Get(self)->OnProtocolExecution(
      CefBrowserCToCpp::Wrap(browser), CefString(url), allowExec);
  *allowOSExecution = allowExec;

  return rv;
}

int CEF_CALLBACK request_handler_get_download_handler(
    struct _cef_request_handler_t* self, cef_browser_t* browser,
    const cef_string_t* mimeType, const cef_string_t* fileName,
    int64 contentLength, struct _cef_download_handler_t** handler)
{
  DCHECK(self);
  DCHECK(browser);
  DCHECK(mimeType);
  DCHECK(fileName);
  DCHECK(handler);
  if (!self || !browser || !mimeType || !fileName || !handler)
    return 0;

  CefRefPtr<CefDownloadHandler> downloadPtr;

  int rv = CefRequestHandlerCppToC::Get(self)->GetDownloadHandler(
      CefBrowserCToCpp::Wrap(browser), CefString(mimeType), CefString(fileName),
      contentLength, downloadPtr);

  if(downloadPtr.get())
    *handler = CefDownloadHandlerCppToC::Wrap(downloadPtr);

  return rv;
}

int CEF_CALLBACK request_handler_get_auth_credentials(
    struct _cef_request_handler_t* self, cef_browser_t* browser, int isProxy,
    const cef_string_t* host, const cef_string_t* realm,
    const cef_string_t* scheme, cef_string_t* username,
    cef_string_t* password)
{
  DCHECK(self);
  DCHECK(browser);
  DCHECK(host);
  DCHECK(realm);
  DCHECK(scheme);
  DCHECK(username);
  DCHECK(password);
  if (!self || !browser || !host || !realm || !scheme || !username || !password)
    return 0;

  CefString usernameStr(username);
  CefString passwordStr(password);
  return CefRequestHandlerCppToC::Get(self)->GetAuthCredentials(
      CefBrowserCToCpp::Wrap(browser), (isProxy ? true : false),
      CefString(host), CefString(realm), CefString(scheme), usernameStr,
      passwordStr);
}


// CONSTRUCTOR - Do not edit by hand.

CefRequestHandlerCppToC::CefRequestHandlerCppToC(CefRequestHandler* cls)
    : CefCppToC<CefRequestHandlerCppToC, CefRequestHandler,
        cef_request_handler_t>(cls)
{
  struct_.struct_.on_before_browse = request_handler_on_before_browse;
  struct_.struct_.on_before_resource_load =
      request_handler_on_before_resource_load;
  struct_.struct_.on_protocol_execution = request_handler_on_protocol_execution;
  struct_.struct_.get_download_handler = request_handler_get_download_handler;
  struct_.struct_.get_auth_credentials = request_handler_get_auth_credentials;
}

#ifndef NDEBUG
template<> long CefCppToC<CefRequestHandlerCppToC, CefRequestHandler,
    cef_request_handler_t>::DebugObjCt = 0;
#endif

