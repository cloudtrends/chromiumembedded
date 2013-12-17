// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CEF_LIBCEF_COMMON_CONTENT_CLIENT_H_
#define CEF_LIBCEF_COMMON_CONTENT_CLIENT_H_
#pragma once

#include <list>
#include <string>
#include <vector>

#include "include/cef_app.h"

#include "base/compiler_specific.h"
#include "content/public/common/content_client.h"
#include "ui/base/resource/resource_bundle.h"

class CefContentClient : public content::ContentClient,
                         public ui::ResourceBundle::Delegate {
 public:
  explicit CefContentClient(CefRefPtr<CefApp> application);
  virtual ~CefContentClient();

  // Returns the singleton CefContentClient instance.
  static CefContentClient* Get();

  // content::ContentClient methods.
  virtual void AddAdditionalSchemes(
      std::vector<std::string>* standard_schemes,
      std::vector<std::string>* savable_schemes) OVERRIDE;
  virtual std::string GetUserAgent() const OVERRIDE;
  virtual base::string16 GetLocalizedString(int message_id) const OVERRIDE;
  virtual base::StringPiece GetDataResource(
      int resource_id,
      ui::ScaleFactor scale_factor) const OVERRIDE;
  virtual gfx::Image& GetNativeImageNamed(int resource_id) const OVERRIDE;

#if defined(OS_MACOSX) && !defined(OS_IOS)
  virtual std::string GetCarbonInterposePath() const OVERRIDE;
#endif

  struct SchemeInfo {
    std::string scheme_name;
    bool is_standard;
    bool is_local;
    bool is_display_isolated;
  };
  typedef std::list<SchemeInfo> SchemeInfoList;

  // Custom scheme registration.
  void AddCustomScheme(const SchemeInfo& scheme_info);
  const SchemeInfoList* GetCustomSchemes();
  bool HasCustomScheme(const std::string& scheme_name);

  CefRefPtr<CefApp> application() const { return application_; }

  void set_pack_loading_disabled(bool val) { pack_loading_disabled_ = val; }
  bool pack_loading_disabled() const { return pack_loading_disabled_; }
  void set_allow_pack_file_load(bool val) { allow_pack_file_load_ = val; }

 private:
  // ui::ResourceBundle::Delegate methods.
  virtual base::FilePath GetPathForResourcePack(
      const base::FilePath& pack_path,
      ui::ScaleFactor scale_factor) OVERRIDE;
  virtual base::FilePath GetPathForLocalePack(
      const base::FilePath& pack_path,
      const std::string& locale) OVERRIDE;
  virtual gfx::Image GetImageNamed(int resource_id) OVERRIDE;
  virtual gfx::Image GetNativeImageNamed(
      int resource_id,
      ui::ResourceBundle::ImageRTL rtl) OVERRIDE;
  virtual base::RefCountedStaticMemory* LoadDataResourceBytes(
      int resource_id,
      ui::ScaleFactor scale_factor) OVERRIDE;
  virtual bool GetRawDataResource(int resource_id,
                                  ui::ScaleFactor scale_factor,
                                  base::StringPiece* value) OVERRIDE;
  virtual bool GetLocalizedString(int message_id,
                                  base::string16* value) OVERRIDE;
  virtual scoped_ptr<gfx::Font> GetFont(
      ui::ResourceBundle::FontStyle style) OVERRIDE;

  CefRefPtr<CefApp> application_;
  bool pack_loading_disabled_;
  bool allow_pack_file_load_;

  // Custom schemes handled by the client.
  SchemeInfoList scheme_info_list_;
  bool scheme_info_list_locked_;
};

#endif  // CEF_LIBCEF_COMMON_CONTENT_CLIENT_H_
