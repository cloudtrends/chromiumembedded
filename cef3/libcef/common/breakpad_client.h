// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CEF_LIBCEF_COMMON_BREAKPAD_CLIENT_H_
#define CEF_LIBCEF_COMMON_BREAKPAD_CLIENT_H_

#include "base/compiler_specific.h"
#include "components/breakpad/app/breakpad_client.h"

class CefBreakpadClient : public breakpad::BreakpadClient {
 public:
  CefBreakpadClient();
  virtual ~CefBreakpadClient();

#if defined(OS_WIN)
  // Returns a textual description of the product type and version to include
  // in the crash report.
  virtual void GetProductNameAndVersion(const base::FilePath& exe_path,
                                        base::string16* product_name,
                                        base::string16* version,
                                        base::string16* special_build,
                                        base::string16* channel_name) OVERRIDE;
#endif

#if defined(OS_POSIX) && !defined(OS_MACOSX) && !defined(OS_IOS)
  // Returns a textual description of the product type and version to include
  // in the crash report.
  virtual void GetProductNameAndVersion(std::string* product_name,
                                        std::string* version) OVERRIDE;

  virtual base::FilePath GetReporterLogFilename() OVERRIDE;
#endif

  // The location where minidump files should be written. Returns true if
  // |crash_dir| was set.
  virtual bool GetCrashDumpLocation(base::FilePath* crash_dir) OVERRIDE;

private:
  DISALLOW_COPY_AND_ASSIGN(CefBreakpadClient);
};

#endif  // CEF_LIBCEF_COMMON_BREAKPAD_CLIENT_H_
