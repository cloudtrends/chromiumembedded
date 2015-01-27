// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_CLIENT_HANDLER_STD_H_
#define CEF_TESTS_CEFCLIENT_CLIENT_HANDLER_STD_H_

#include "cefclient/client_handler_single.h"

namespace client {

// Client handler implementation for windowed browsers. There will only ever be
// one browser per handler instance.
class ClientHandlerStd : public ClientHandlerSingle {
 public:
  ClientHandlerStd(Delegate* delegate,
                   const std::string& startup_url);

 private:
  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(ClientHandlerStd);
  DISALLOW_COPY_AND_ASSIGN(ClientHandlerStd);
};

}  // namespace client

#endif  // CEF_TESTS_CEFCLIENT_CLIENT_HANDLER_STD_H_
