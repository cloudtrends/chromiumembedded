// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// ---------------------------------------------------------------------------
//
// This file was generated by the CEF translator tool. If making changes by
// hand only do so within the body of existing method and function
// implementations. See the translator.README.txt file in the tools directory
// for more information.
//

#ifndef _XMLREADER_CTOCPP_H
#define _XMLREADER_CTOCPP_H

#ifndef USING_CEF_SHARED
#pragma message("Warning: "__FILE__" may be accessed wrapper-side only")
#else // USING_CEF_SHARED

#include "include/cef_xml_reader.h"
#include "include/capi/cef_xml_reader_capi.h"
#include "include/cef_stream.h"
#include "include/capi/cef_stream_capi.h"
#include "libcef_dll/ctocpp/ctocpp.h"

// Wrap a C structure with a C++ class.
// This class may be instantiated and accessed wrapper-side only.
class CefXmlReaderCToCpp
    : public CefCToCpp<CefXmlReaderCToCpp, CefXmlReader, cef_xml_reader_t>
{
public:
  CefXmlReaderCToCpp(cef_xml_reader_t* str)
      : CefCToCpp<CefXmlReaderCToCpp, CefXmlReader, cef_xml_reader_t>(str) {}
  virtual ~CefXmlReaderCToCpp() {}

  // CefXmlReader methods
  virtual bool MoveToNextNode() OVERRIDE;
  virtual bool Close() OVERRIDE;
  virtual bool HasError() OVERRIDE;
  virtual CefString GetError() OVERRIDE;
  virtual NodeType GetType() OVERRIDE;
  virtual int GetDepth() OVERRIDE;
  virtual CefString GetLocalName() OVERRIDE;
  virtual CefString GetPrefix() OVERRIDE;
  virtual CefString GetQualifiedName() OVERRIDE;
  virtual CefString GetNamespaceURI() OVERRIDE;
  virtual CefString GetBaseURI() OVERRIDE;
  virtual CefString GetXmlLang() OVERRIDE;
  virtual bool IsEmptyElement() OVERRIDE;
  virtual bool HasValue() OVERRIDE;
  virtual CefString GetValue() OVERRIDE;
  virtual bool HasAttributes() OVERRIDE;
  virtual size_t GetAttributeCount() OVERRIDE;
  virtual CefString GetAttribute(int index) OVERRIDE;
  virtual CefString GetAttribute(const CefString& qualifiedName) OVERRIDE;
  virtual CefString GetAttribute(const CefString& localName,
      const CefString& namespaceURI) OVERRIDE;
  virtual CefString GetInnerXml() OVERRIDE;
  virtual CefString GetOuterXml() OVERRIDE;
  virtual int GetLineNumber() OVERRIDE;
  virtual bool MoveToAttribute(int index) OVERRIDE;
  virtual bool MoveToAttribute(const CefString& qualifiedName) OVERRIDE;
  virtual bool MoveToAttribute(const CefString& localName,
      const CefString& namespaceURI) OVERRIDE;
  virtual bool MoveToFirstAttribute() OVERRIDE;
  virtual bool MoveToNextAttribute() OVERRIDE;
  virtual bool MoveToCarryingElement() OVERRIDE;
};

#endif // USING_CEF_SHARED
#endif // _XMLREADER_CTOCPP_H

