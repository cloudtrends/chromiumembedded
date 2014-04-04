// Copyright (c) 2012 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CEF_LIBCEF_RENDERER_WEBKIT_GLUE_H_
#define CEF_LIBCEF_RENDERER_WEBKIT_GLUE_H_

#include <string>
#include "base/basictypes.h"

namespace v8 {
class Context;
template <class T> class Handle;
class Isolate;
}

namespace blink {
class WebFrame;
class WebNode;
class WebString;
class WebView;
}

namespace webkit_glue {

extern const int64 kInvalidFrameId;

bool CanGoBack(blink::WebView* view);
bool CanGoForward(blink::WebView* view);
void GoBack(blink::WebView* view);
void GoForward(blink::WebView* view);

// Returns the text of the document element.
std::string DumpDocumentText(blink::WebFrame* frame);

bool SetNodeValue(blink::WebNode& node, const blink::WebString& value);

int64 GetIdentifier(blink::WebFrame* frame);

}  // webkit_glue

#endif  // CEF_LIBCEF_RENDERER_WEBKIT_GLUE_H_
