// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// -------------------------------------------------------------------------
//
// This file was generated by the CEF translator tool and should not edited
// by hand. See the translator.README.txt file in the tools directory for
// more information.
//

#ifndef _COMMANDLINE_CTOCPP_H
#define _COMMANDLINE_CTOCPP_H

#ifndef USING_CEF_SHARED
#pragma message("Warning: "__FILE__" may be accessed wrapper-side only")
#else // USING_CEF_SHARED

#include "include/cef.h"
#include "include/cef_capi.h"
#include "libcef_dll/ctocpp/ctocpp.h"

// Wrap a C structure with a C++ class.
// This class may be instantiated and accessed wrapper-side only.
class CefCommandLineCToCpp
    : public CefCToCpp<CefCommandLineCToCpp, CefCommandLine, cef_command_line_t>
{
public:
  CefCommandLineCToCpp(cef_command_line_t* str)
      : CefCToCpp<CefCommandLineCToCpp, CefCommandLine, cef_command_line_t>(
          str) {}
  virtual ~CefCommandLineCToCpp() {}

  // CefCommandLine methods
  virtual void InitFromArgv(int argc, const char* const* argv) OVERRIDE;
  virtual void InitFromString(const CefString& command_line) OVERRIDE;
  virtual CefString GetCommandLineString() OVERRIDE;
  virtual CefString GetProgram() OVERRIDE;
  virtual void SetProgram(const CefString& program) OVERRIDE;
  virtual bool HasSwitches() OVERRIDE;
  virtual bool HasSwitch(const CefString& name) OVERRIDE;
  virtual CefString GetSwitchValue(const CefString& name) OVERRIDE;
  virtual void GetSwitches(SwitchMap& switches) OVERRIDE;
  virtual void AppendSwitch(const CefString& name) OVERRIDE;
  virtual void AppendSwitchWithValue(const CefString& name,
      const CefString& value) OVERRIDE;
  virtual bool HasArguments() OVERRIDE;
  virtual void GetArguments(ArgumentList& arguments) OVERRIDE;
  virtual void AppendArgument(const CefString& argument) OVERRIDE;
};

#endif // USING_CEF_SHARED
#endif // _COMMANDLINE_CTOCPP_H

