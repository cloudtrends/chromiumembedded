// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "libcef/browser/scheme_handler.h"

#include <string>

#include "libcef/browser/chrome_scheme_handler.h"
#include "libcef/browser/devtools_scheme_handler.h"
#include "libcef/common/scheme_registration.h"

#include "content/public/browser/browser_thread.h"
#include "content/public/common/url_constants.h"
#include "net/url_request/data_protocol_handler.h"
#include "net/url_request/file_protocol_handler.h"
#include "net/url_request/ftp_protocol_handler.h"
#include "net/url_request/url_request_job_factory_impl.h"

namespace scheme {

void InstallInternalProtectedHandlers(
    net::URLRequestJobFactoryImpl* job_factory,
    content::ProtocolHandlerMap* protocol_handlers,
    net::FtpTransactionFactory* ftp_transaction_factory) {
  protocol_handlers->insert(
      std::make_pair(chrome::kDataScheme, new net::DataProtocolHandler));
  protocol_handlers->insert(
      std::make_pair(chrome::kFileScheme, new net::FileProtocolHandler(
          content::BrowserThread::GetBlockingPool()->
              GetTaskRunnerWithShutdownBehavior(
                  base::SequencedWorkerPool::SKIP_ON_SHUTDOWN))));
#if !defined(DISABLE_FTP_SUPPORT)
  protocol_handlers->insert(
      std::make_pair(chrome::kFtpScheme,
                     new net::FtpProtocolHandler(ftp_transaction_factory)));
#endif

  for (content::ProtocolHandlerMap::iterator it =
           protocol_handlers->begin();
       it != protocol_handlers->end();
       ++it) {
    const std::string& scheme = it->first;
    scoped_ptr<net::URLRequestJobFactory::ProtocolHandler> protocol_handler;

    if (scheme == chrome::kChromeDevToolsScheme) {
      // Don't use the default "chrome-devtools" handler.
      continue;
    } else if (scheme == chrome::kChromeUIScheme) {
      // Filter the URLs that are passed to the default "chrome" handler so as
      // not to interfere with CEF's "chrome" handler.
      protocol_handler.reset(
          scheme::WrapChromeProtocolHandler(
              make_scoped_ptr(it->second.release())).release());
    } else {
      protocol_handler.reset(it->second.release());
    }

    // Make sure IsInternalProtectedScheme() stays synchronized with what
    // Chromium is actually giving us.
    DCHECK(IsInternalProtectedScheme(scheme));

    bool set_protocol = job_factory->SetProtocolHandler(
        scheme, protocol_handler.release());
    DCHECK(set_protocol);
  }
}

void RegisterInternalHandlers() {
  scheme::RegisterChromeHandler();
  scheme::RegisterChromeDevToolsHandler();
}

void DidFinishLoad(CefRefPtr<CefFrame> frame, const GURL& validated_url) {
  if (validated_url.scheme() == chrome::kChromeUIScheme)
    scheme::DidFinishChromeLoad(frame, validated_url);
}

}  // namespace scheme
