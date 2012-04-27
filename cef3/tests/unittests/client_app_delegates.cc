// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "tests/cefclient/client_app.h"

// static
void ClientApp::CreateRenderDelegates(RenderDelegateSet& delegates) {
  // Bring in the process message tests.
  extern void CreateProcessMessageRendererTests(
      ClientApp::RenderDelegateSet& delegates);
  CreateProcessMessageRendererTests(delegates);

  // Bring in the V8 tests.
  extern void CreateV8RendererTests(RenderDelegateSet& delegates);
  CreateV8RendererTests(delegates);

  // Bring in the DOM tests.
  extern void CreateDOMRendererTests(RenderDelegateSet& delegates);
  CreateDOMRendererTests(delegates);
}

// static
void ClientApp::RegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar) {
  // Bring in the scheme handler tests.
  extern void RegisterSchemeHandlerCustomSchemes(
      CefRefPtr<CefSchemeRegistrar> registrar);
  RegisterSchemeHandlerCustomSchemes(registrar);

  // Bring in the cookie tests.
  extern void RegisterCookieCustomSchemes(
      CefRefPtr<CefSchemeRegistrar> registrar);
  RegisterCookieCustomSchemes(registrar);
}
