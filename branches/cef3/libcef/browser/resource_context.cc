// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/browser/resource_context.h"
#include "libcef/browser/thread_util.h"
#include "libcef/browser/url_request_context_getter.h"

#include "content/browser/browser_main_loop.h"
#include "content/public/browser/media_observer.h"

class MockMediaObserver : public content::MediaObserver {
 public:
  MockMediaObserver() {}
  virtual ~MockMediaObserver() {}

  virtual void OnDeleteAudioStream(void* host, int stream_id) OVERRIDE {}

  virtual void OnSetAudioStreamPlaying(void* host, int stream_id,
                                       bool playing) OVERRIDE {}
  virtual void OnSetAudioStreamStatus(void* host, int stream_id,
                                      const std::string& status) OVERRIDE {}
  virtual void OnSetAudioStreamVolume(void* host, int stream_id,
                                      double volume) OVERRIDE {}
  virtual void OnMediaEvent(int render_process_id,
                            const media::MediaLogEvent& event) OVERRIDE {}
};

CefResourceContext::CefResourceContext(
    CefURLRequestContextGetter* getter)
    : getter_(getter) {
}

CefResourceContext::~CefResourceContext() {
}

net::HostResolver* CefResourceContext::GetHostResolver() {
  CEF_REQUIRE_IOT();
  return getter_->host_resolver();
}

net::URLRequestContext* CefResourceContext::GetRequestContext() {
  CEF_REQUIRE_IOT();
  return getter_->GetURLRequestContext();
}

content::MediaObserver* CefResourceContext::GetMediaObserver() {
  CEF_REQUIRE_IOT();
  // TODO(cef): Return NULL once it's supported. See crbug.com/116113.
  if (!media_observer_.get())
     media_observer_.reset(new MockMediaObserver());
  return media_observer_.get();
}
