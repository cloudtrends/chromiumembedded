// Copyright (c) 2010 Marshall A. Greenblatt. All rights reserved.
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


#ifndef _CEF2_WIN_H
#define _CEF2_WIN_H

#ifdef _WIN32
#include <windows.h>
#include "cef2_types_win.h"

// Atomic increment and decrement.
#define CefAtomicIncrement(p) InterlockedIncrement(p)
#define CefAtomicDecrement(p) InterlockedDecrement(p)

// Critical section wrapper.
class CefCriticalSection
{
public:
  CefCriticalSection()
  {
    memset(&m_sec, 0, sizeof(CRITICAL_SECTION));
    InitializeCriticalSection(&m_sec);
  }
  ~CefCriticalSection()
  {
    DeleteCriticalSection(&m_sec);
  }
  void Lock()
  {
    EnterCriticalSection(&m_sec);
  }
  void Unlock()
  {
    LeaveCriticalSection(&m_sec);
  }

  CRITICAL_SECTION m_sec;
};

// Window handle.
#define CefWindowHandle cef_window_handle_t
#endif // _WIN32

#endif // _CEF2_WIN_H
