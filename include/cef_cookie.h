// Copyright (c) 2011 Marshall A. Greenblatt. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the name Chromium Embedded
// Framework nor the names of its contributors may be used to endorse
// or promote products derived from this software without specific prior
// written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// ---------------------------------------------------------------------------
//
// The contents of this file must follow a specific format in order to
// support the CEF translator tool. See the translator.README.txt file in the
// tools directory for more information.
//

#ifndef CEF_INCLUDE_CEF_COOKIE_H_
#define CEF_INCLUDE_CEF_COOKIE_H_
#pragma once

#include "include/cef_base.h"

class CefCookieVisitor;

///
// Visit all cookies. The returned cookies are ordered by longest path, then by
// earliest creation date. Returns false if cookies cannot be accessed.
///
/*--cef()--*/
bool CefVisitAllCookies(CefRefPtr<CefCookieVisitor> visitor);

///
// Visit a subset of cookies. The results are filtered by the given url scheme,
// host, domain and path. If |includeHttpOnly| is true HTTP-only cookies will
// also be included in the results. The returned cookies are ordered by longest
// path, then by earliest creation date. Returns false if cookies cannot be
// accessed.
///
/*--cef()--*/
bool CefVisitUrlCookies(const CefString& url, bool includeHttpOnly,
                        CefRefPtr<CefCookieVisitor> visitor);

///
// Sets a cookie given a valid URL and explicit user-provided cookie attributes.
// This function expects each attribute to be well-formed. It will check for
// disallowed characters (e.g. the ';' character is disallowed within the cookie
// value attribute) and will return false without setting the cookie if such
// characters are found. This method must be called on the IO thread.
///
/*--cef()--*/
bool CefSetCookie(const CefString& url, const CefCookie& cookie);

///
// Delete all cookies that match the specified parameters. If both |url| and
// |cookie_name| are specified all host and domain cookies matching both values
// will be deleted. If only |url| is specified all host cookies (but not domain
// cookies) irrespective of path will be deleted. If |url| is empty all cookies
// for all hosts and domains will be deleted. Returns false if a non-empty
// invalid URL is specified or if cookies cannot be accessed. This method must
// be called on the IO thread.
///
/*--cef(optional_param=url,optional_param=cookie_name)--*/
bool CefDeleteCookies(const CefString& url, const CefString& cookie_name);

///
// Sets the directory path that will be used for storing cookie data. If |path|
// is empty data will be stored in memory only. By default the cookie path is
// the same as the cache path. Returns false if cookies cannot be accessed.
///
/*--cef(optional_param=path)--*/
bool CefSetCookiePath(const CefString& path);


///
// Interface to implement for visiting cookie values. The methods of this class
// will always be called on the IO thread.
///
/*--cef(source=client)--*/
class CefCookieVisitor : public virtual CefBase {
 public:
  ///
  // Method that will be called once for each cookie. |count| is the 0-based
  // index for the current cookie. |total| is the total number of cookies.
  // Set |deleteCookie| to true to delete the cookie currently being visited.
  // Return false to stop visiting cookies. This method may never be called if
  // no cookies are found.
  ///
  /*--cef()--*/
  virtual bool Visit(const CefCookie& cookie, int count, int total,
                     bool& deleteCookie) =0;
};

#endif  // CEF_INCLUDE_CEF_COOKIE_H_
