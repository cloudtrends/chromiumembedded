// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "transfer_util.h"


void transfer_string_contents(const std::wstring& fromString,
                              cef_string_t* toString)
{
  if(*toString == NULL || *toString != fromString) {
    if(*toString) {
      cef_string_free(*toString);
      *toString = NULL;
    }
    if(!fromString.empty())
      *toString = cef_string_alloc(fromString.c_str());
  }
}

void transfer_string_contents(cef_string_t fromString, std::wstring& toString,
                              bool freeFromString)
{
  if(fromString == NULL || fromString != toString) {
    if(fromString) {
      toString = fromString;
      if(freeFromString)
        cef_string_free(fromString);
    } else if(!toString.empty())
      toString.clear();
  }
}
