// Copyright (c) 2014 Marshall A. Greenblatt. All rights reserved.
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
//
// ---------------------------------------------------------------------------
//
// This file was generated by the CEF translator tool and should not edited
// by hand. See the translator.README.txt file in the tools directory for
// more information.
//

#ifndef CEF_INCLUDE_CAPI_CEF_DOM_CAPI_H_
#define CEF_INCLUDE_CAPI_CEF_DOM_CAPI_H_
#pragma once

#include "include/capi/cef_base_capi.h"

#ifdef __cplusplus
extern "C" {
#endif

struct _cef_domdocument_t;
struct _cef_domnode_t;

///
// Structure to implement for visiting the DOM. The functions of this structure
// will be called on the render process main thread.
///
typedef struct _cef_domvisitor_t {
  ///
  // Base structure.
  ///
  cef_base_t base;

  ///
  // Method executed for visiting the DOM. The document object passed to this
  // function represents a snapshot of the DOM at the time this function is
  // executed. DOM objects are only valid for the scope of this function. Do not
  // keep references to or attempt to access any DOM objects outside the scope
  // of this function.
  ///
  void (CEF_CALLBACK *visit)(struct _cef_domvisitor_t* self,
      struct _cef_domdocument_t* document);
} cef_domvisitor_t;


///
// Structure used to represent a DOM document. The functions of this structure
// should only be called on the render process main thread thread.
///
typedef struct _cef_domdocument_t {
  ///
  // Base structure.
  ///
  cef_base_t base;

  ///
  // Returns the document type.
  ///
  cef_dom_document_type_t (CEF_CALLBACK *get_type)(
      struct _cef_domdocument_t* self);

  ///
  // Returns the root document node.
  ///
  struct _cef_domnode_t* (CEF_CALLBACK *get_document)(
      struct _cef_domdocument_t* self);

  ///
  // Returns the BODY node of an HTML document.
  ///
  struct _cef_domnode_t* (CEF_CALLBACK *get_body)(
      struct _cef_domdocument_t* self);

  ///
  // Returns the HEAD node of an HTML document.
  ///
  struct _cef_domnode_t* (CEF_CALLBACK *get_head)(
      struct _cef_domdocument_t* self);

  ///
  // Returns the title of an HTML document.
  ///
  // The resulting string must be freed by calling cef_string_userfree_free().
  cef_string_userfree_t (CEF_CALLBACK *get_title)(
      struct _cef_domdocument_t* self);

  ///
  // Returns the document element with the specified ID value.
  ///
  struct _cef_domnode_t* (CEF_CALLBACK *get_element_by_id)(
      struct _cef_domdocument_t* self, const cef_string_t* id);

  ///
  // Returns the node that currently has keyboard focus.
  ///
  struct _cef_domnode_t* (CEF_CALLBACK *get_focused_node)(
      struct _cef_domdocument_t* self);

  ///
  // Returns true (1) if a portion of the document is selected.
  ///
  int (CEF_CALLBACK *has_selection)(struct _cef_domdocument_t* self);

  ///
  // Returns the selection offset within the start node.
  ///
  int (CEF_CALLBACK *get_selection_start_offset)(
      struct _cef_domdocument_t* self);

  ///
  // Returns the selection offset within the end node.
  ///
  int (CEF_CALLBACK *get_selection_end_offset)(struct _cef_domdocument_t* self);

  ///
  // Returns the contents of this selection as markup.
  ///
  // The resulting string must be freed by calling cef_string_userfree_free().
  cef_string_userfree_t (CEF_CALLBACK *get_selection_as_markup)(
      struct _cef_domdocument_t* self);

  ///
  // Returns the contents of this selection as text.
  ///
  // The resulting string must be freed by calling cef_string_userfree_free().
  cef_string_userfree_t (CEF_CALLBACK *get_selection_as_text)(
      struct _cef_domdocument_t* self);

  ///
  // Returns the base URL for the document.
  ///
  // The resulting string must be freed by calling cef_string_userfree_free().
  cef_string_userfree_t (CEF_CALLBACK *get_base_url)(
      struct _cef_domdocument_t* self);

  ///
  // Returns a complete URL based on the document base URL and the specified
  // partial URL.
  ///
  // The resulting string must be freed by calling cef_string_userfree_free().
  cef_string_userfree_t (CEF_CALLBACK *get_complete_url)(
      struct _cef_domdocument_t* self, const cef_string_t* partialURL);
} cef_domdocument_t;


///
// Structure used to represent a DOM node. The functions of this structure
// should only be called on the render process main thread.
///
typedef struct _cef_domnode_t {
  ///
  // Base structure.
  ///
  cef_base_t base;

  ///
  // Returns the type for this node.
  ///
  cef_dom_node_type_t (CEF_CALLBACK *get_type)(struct _cef_domnode_t* self);

  ///
  // Returns true (1) if this is a text node.
  ///
  int (CEF_CALLBACK *is_text)(struct _cef_domnode_t* self);

  ///
  // Returns true (1) if this is an element node.
  ///
  int (CEF_CALLBACK *is_element)(struct _cef_domnode_t* self);

  ///
  // Returns true (1) if this is an editable node.
  ///
  int (CEF_CALLBACK *is_editable)(struct _cef_domnode_t* self);

  ///
  // Returns true (1) if this is a form control element node.
  ///
  int (CEF_CALLBACK *is_form_control_element)(struct _cef_domnode_t* self);

  ///
  // Returns the type of this form control element node.
  ///
  // The resulting string must be freed by calling cef_string_userfree_free().
  cef_string_userfree_t (CEF_CALLBACK *get_form_control_element_type)(
      struct _cef_domnode_t* self);

  ///
  // Returns true (1) if this object is pointing to the same handle as |that|
  // object.
  ///
  int (CEF_CALLBACK *is_same)(struct _cef_domnode_t* self,
      struct _cef_domnode_t* that);

  ///
  // Returns the name of this node.
  ///
  // The resulting string must be freed by calling cef_string_userfree_free().
  cef_string_userfree_t (CEF_CALLBACK *get_name)(struct _cef_domnode_t* self);

  ///
  // Returns the value of this node.
  ///
  // The resulting string must be freed by calling cef_string_userfree_free().
  cef_string_userfree_t (CEF_CALLBACK *get_value)(struct _cef_domnode_t* self);

  ///
  // Set the value of this node. Returns true (1) on success.
  ///
  int (CEF_CALLBACK *set_value)(struct _cef_domnode_t* self,
      const cef_string_t* value);

  ///
  // Returns the contents of this node as markup.
  ///
  // The resulting string must be freed by calling cef_string_userfree_free().
  cef_string_userfree_t (CEF_CALLBACK *get_as_markup)(
      struct _cef_domnode_t* self);

  ///
  // Returns the document associated with this node.
  ///
  struct _cef_domdocument_t* (CEF_CALLBACK *get_document)(
      struct _cef_domnode_t* self);

  ///
  // Returns the parent node.
  ///
  struct _cef_domnode_t* (CEF_CALLBACK *get_parent)(
      struct _cef_domnode_t* self);

  ///
  // Returns the previous sibling node.
  ///
  struct _cef_domnode_t* (CEF_CALLBACK *get_previous_sibling)(
      struct _cef_domnode_t* self);

  ///
  // Returns the next sibling node.
  ///
  struct _cef_domnode_t* (CEF_CALLBACK *get_next_sibling)(
      struct _cef_domnode_t* self);

  ///
  // Returns true (1) if this node has child nodes.
  ///
  int (CEF_CALLBACK *has_children)(struct _cef_domnode_t* self);

  ///
  // Return the first child node.
  ///
  struct _cef_domnode_t* (CEF_CALLBACK *get_first_child)(
      struct _cef_domnode_t* self);

  ///
  // Returns the last child node.
  ///
  struct _cef_domnode_t* (CEF_CALLBACK *get_last_child)(
      struct _cef_domnode_t* self);


  // The following functions are valid only for element nodes.

  ///
  // Returns the tag name of this element.
  ///
  // The resulting string must be freed by calling cef_string_userfree_free().
  cef_string_userfree_t (CEF_CALLBACK *get_element_tag_name)(
      struct _cef_domnode_t* self);

  ///
  // Returns true (1) if this element has attributes.
  ///
  int (CEF_CALLBACK *has_element_attributes)(struct _cef_domnode_t* self);

  ///
  // Returns true (1) if this element has an attribute named |attrName|.
  ///
  int (CEF_CALLBACK *has_element_attribute)(struct _cef_domnode_t* self,
      const cef_string_t* attrName);

  ///
  // Returns the element attribute named |attrName|.
  ///
  // The resulting string must be freed by calling cef_string_userfree_free().
  cef_string_userfree_t (CEF_CALLBACK *get_element_attribute)(
      struct _cef_domnode_t* self, const cef_string_t* attrName);

  ///
  // Returns a map of all element attributes.
  ///
  void (CEF_CALLBACK *get_element_attributes)(struct _cef_domnode_t* self,
      cef_string_map_t attrMap);

  ///
  // Set the value for the element attribute named |attrName|. Returns true (1)
  // on success.
  ///
  int (CEF_CALLBACK *set_element_attribute)(struct _cef_domnode_t* self,
      const cef_string_t* attrName, const cef_string_t* value);

  ///
  // Returns the inner text of the element.
  ///
  // The resulting string must be freed by calling cef_string_userfree_free().
  cef_string_userfree_t (CEF_CALLBACK *get_element_inner_text)(
      struct _cef_domnode_t* self);
} cef_domnode_t;


#ifdef __cplusplus
}
#endif

#endif  // CEF_INCLUDE_CAPI_CEF_DOM_CAPI_H_
