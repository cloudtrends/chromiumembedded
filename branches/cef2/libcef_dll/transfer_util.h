// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef _TRANSFER_UTIL_H
#define _TRANSFER_UTIL_H

#include "include/cef2_string.h"
#include <string>

// Copy the contents from one string type to another.
void transfer_string_contents(const std::wstring& fromString,
                              cef_string_t* toString);
void transfer_string_contents(cef_string_t fromString, std::wstring& toString,
                              bool freeFromString);

#endif // _TRANSFER_UTIL_H
