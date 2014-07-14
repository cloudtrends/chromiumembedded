// Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
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

#include "libcef_dll/cpptoc/menu_model_cpptoc.h"


// MEMBER FUNCTIONS - Body may be edited by hand.

int CEF_CALLBACK menu_model_clear(struct _cef_menu_model_t* self) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->Clear();

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_get_count(struct _cef_menu_model_t* self) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  int _retval = CefMenuModelCppToC::Get(self)->GetCount();

  // Return type: simple
  return _retval;
}

int CEF_CALLBACK menu_model_add_separator(struct _cef_menu_model_t* self) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->AddSeparator();

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_add_item(struct _cef_menu_model_t* self,
    int command_id, const cef_string_t* label) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: label; type: string_byref_const
  DCHECK(label);
  if (!label)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->AddItem(
      command_id,
      CefString(label));

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_add_check_item(struct _cef_menu_model_t* self,
    int command_id, const cef_string_t* label) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: label; type: string_byref_const
  DCHECK(label);
  if (!label)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->AddCheckItem(
      command_id,
      CefString(label));

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_add_radio_item(struct _cef_menu_model_t* self,
    int command_id, const cef_string_t* label, int group_id) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: label; type: string_byref_const
  DCHECK(label);
  if (!label)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->AddRadioItem(
      command_id,
      CefString(label),
      group_id);

  // Return type: bool
  return _retval;
}

struct _cef_menu_model_t* CEF_CALLBACK menu_model_add_sub_menu(
    struct _cef_menu_model_t* self, int command_id,
    const cef_string_t* label) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return NULL;
  // Verify param: label; type: string_byref_const
  DCHECK(label);
  if (!label)
    return NULL;

  // Execute
  CefRefPtr<CefMenuModel> _retval = CefMenuModelCppToC::Get(self)->AddSubMenu(
      command_id,
      CefString(label));

  // Return type: refptr_same
  return CefMenuModelCppToC::Wrap(_retval);
}

int CEF_CALLBACK menu_model_insert_separator_at(struct _cef_menu_model_t* self,
    int index) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->InsertSeparatorAt(
      index);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_insert_item_at(struct _cef_menu_model_t* self,
    int index, int command_id, const cef_string_t* label) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: label; type: string_byref_const
  DCHECK(label);
  if (!label)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->InsertItemAt(
      index,
      command_id,
      CefString(label));

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_insert_check_item_at(struct _cef_menu_model_t* self,
    int index, int command_id, const cef_string_t* label) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: label; type: string_byref_const
  DCHECK(label);
  if (!label)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->InsertCheckItemAt(
      index,
      command_id,
      CefString(label));

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_insert_radio_item_at(struct _cef_menu_model_t* self,
    int index, int command_id, const cef_string_t* label, int group_id) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: label; type: string_byref_const
  DCHECK(label);
  if (!label)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->InsertRadioItemAt(
      index,
      command_id,
      CefString(label),
      group_id);

  // Return type: bool
  return _retval;
}

struct _cef_menu_model_t* CEF_CALLBACK menu_model_insert_sub_menu_at(
    struct _cef_menu_model_t* self, int index, int command_id,
    const cef_string_t* label) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return NULL;
  // Verify param: label; type: string_byref_const
  DCHECK(label);
  if (!label)
    return NULL;

  // Execute
  CefRefPtr<CefMenuModel> _retval = CefMenuModelCppToC::Get(
      self)->InsertSubMenuAt(
      index,
      command_id,
      CefString(label));

  // Return type: refptr_same
  return CefMenuModelCppToC::Wrap(_retval);
}

int CEF_CALLBACK menu_model_remove(struct _cef_menu_model_t* self,
    int command_id) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->Remove(
      command_id);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_remove_at(struct _cef_menu_model_t* self,
    int index) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->RemoveAt(
      index);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_get_index_of(struct _cef_menu_model_t* self,
    int command_id) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  int _retval = CefMenuModelCppToC::Get(self)->GetIndexOf(
      command_id);

  // Return type: simple
  return _retval;
}

int CEF_CALLBACK menu_model_get_command_id_at(struct _cef_menu_model_t* self,
    int index) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  int _retval = CefMenuModelCppToC::Get(self)->GetCommandIdAt(
      index);

  // Return type: simple
  return _retval;
}

int CEF_CALLBACK menu_model_set_command_id_at(struct _cef_menu_model_t* self,
    int index, int command_id) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->SetCommandIdAt(
      index,
      command_id);

  // Return type: bool
  return _retval;
}

cef_string_userfree_t CEF_CALLBACK menu_model_get_label(
    struct _cef_menu_model_t* self, int command_id) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return NULL;

  // Execute
  CefString _retval = CefMenuModelCppToC::Get(self)->GetLabel(
      command_id);

  // Return type: string
  return _retval.DetachToUserFree();
}

cef_string_userfree_t CEF_CALLBACK menu_model_get_label_at(
    struct _cef_menu_model_t* self, int index) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return NULL;

  // Execute
  CefString _retval = CefMenuModelCppToC::Get(self)->GetLabelAt(
      index);

  // Return type: string
  return _retval.DetachToUserFree();
}

int CEF_CALLBACK menu_model_set_label(struct _cef_menu_model_t* self,
    int command_id, const cef_string_t* label) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: label; type: string_byref_const
  DCHECK(label);
  if (!label)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->SetLabel(
      command_id,
      CefString(label));

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_set_label_at(struct _cef_menu_model_t* self,
    int index, const cef_string_t* label) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: label; type: string_byref_const
  DCHECK(label);
  if (!label)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->SetLabelAt(
      index,
      CefString(label));

  // Return type: bool
  return _retval;
}

cef_menu_item_type_t CEF_CALLBACK menu_model_get_type(
    struct _cef_menu_model_t* self, int command_id) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return MENUITEMTYPE_NONE;

  // Execute
  cef_menu_item_type_t _retval = CefMenuModelCppToC::Get(self)->GetType(
      command_id);

  // Return type: simple
  return _retval;
}

cef_menu_item_type_t CEF_CALLBACK menu_model_get_type_at(
    struct _cef_menu_model_t* self, int index) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return MENUITEMTYPE_NONE;

  // Execute
  cef_menu_item_type_t _retval = CefMenuModelCppToC::Get(self)->GetTypeAt(
      index);

  // Return type: simple
  return _retval;
}

int CEF_CALLBACK menu_model_get_group_id(struct _cef_menu_model_t* self,
    int command_id) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  int _retval = CefMenuModelCppToC::Get(self)->GetGroupId(
      command_id);

  // Return type: simple
  return _retval;
}

int CEF_CALLBACK menu_model_get_group_id_at(struct _cef_menu_model_t* self,
    int index) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  int _retval = CefMenuModelCppToC::Get(self)->GetGroupIdAt(
      index);

  // Return type: simple
  return _retval;
}

int CEF_CALLBACK menu_model_set_group_id(struct _cef_menu_model_t* self,
    int command_id, int group_id) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->SetGroupId(
      command_id,
      group_id);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_set_group_id_at(struct _cef_menu_model_t* self,
    int index, int group_id) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->SetGroupIdAt(
      index,
      group_id);

  // Return type: bool
  return _retval;
}

struct _cef_menu_model_t* CEF_CALLBACK menu_model_get_sub_menu(
    struct _cef_menu_model_t* self, int command_id) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return NULL;

  // Execute
  CefRefPtr<CefMenuModel> _retval = CefMenuModelCppToC::Get(self)->GetSubMenu(
      command_id);

  // Return type: refptr_same
  return CefMenuModelCppToC::Wrap(_retval);
}

struct _cef_menu_model_t* CEF_CALLBACK menu_model_get_sub_menu_at(
    struct _cef_menu_model_t* self, int index) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return NULL;

  // Execute
  CefRefPtr<CefMenuModel> _retval = CefMenuModelCppToC::Get(self)->GetSubMenuAt(
      index);

  // Return type: refptr_same
  return CefMenuModelCppToC::Wrap(_retval);
}

int CEF_CALLBACK menu_model_is_visible(struct _cef_menu_model_t* self,
    int command_id) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->IsVisible(
      command_id);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_is_visible_at(struct _cef_menu_model_t* self,
    int index) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->IsVisibleAt(
      index);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_set_visible(struct _cef_menu_model_t* self,
    int command_id, int visible) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->SetVisible(
      command_id,
      visible?true:false);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_set_visible_at(struct _cef_menu_model_t* self,
    int index, int visible) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->SetVisibleAt(
      index,
      visible?true:false);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_is_enabled(struct _cef_menu_model_t* self,
    int command_id) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->IsEnabled(
      command_id);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_is_enabled_at(struct _cef_menu_model_t* self,
    int index) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->IsEnabledAt(
      index);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_set_enabled(struct _cef_menu_model_t* self,
    int command_id, int enabled) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->SetEnabled(
      command_id,
      enabled?true:false);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_set_enabled_at(struct _cef_menu_model_t* self,
    int index, int enabled) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->SetEnabledAt(
      index,
      enabled?true:false);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_is_checked(struct _cef_menu_model_t* self,
    int command_id) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->IsChecked(
      command_id);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_is_checked_at(struct _cef_menu_model_t* self,
    int index) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->IsCheckedAt(
      index);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_set_checked(struct _cef_menu_model_t* self,
    int command_id, int checked) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->SetChecked(
      command_id,
      checked?true:false);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_set_checked_at(struct _cef_menu_model_t* self,
    int index, int checked) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->SetCheckedAt(
      index,
      checked?true:false);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_has_accelerator(struct _cef_menu_model_t* self,
    int command_id) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->HasAccelerator(
      command_id);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_has_accelerator_at(struct _cef_menu_model_t* self,
    int index) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->HasAcceleratorAt(
      index);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_set_accelerator(struct _cef_menu_model_t* self,
    int command_id, int key_code, int shift_pressed, int ctrl_pressed,
    int alt_pressed) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->SetAccelerator(
      command_id,
      key_code,
      shift_pressed?true:false,
      ctrl_pressed?true:false,
      alt_pressed?true:false);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_set_accelerator_at(struct _cef_menu_model_t* self,
    int index, int key_code, int shift_pressed, int ctrl_pressed,
    int alt_pressed) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->SetAcceleratorAt(
      index,
      key_code,
      shift_pressed?true:false,
      ctrl_pressed?true:false,
      alt_pressed?true:false);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_remove_accelerator(struct _cef_menu_model_t* self,
    int command_id) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->RemoveAccelerator(
      command_id);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_remove_accelerator_at(
    struct _cef_menu_model_t* self, int index) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->RemoveAcceleratorAt(
      index);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_get_accelerator(struct _cef_menu_model_t* self,
    int command_id, int* key_code, int* shift_pressed, int* ctrl_pressed,
    int* alt_pressed) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: key_code; type: simple_byref
  DCHECK(key_code);
  if (!key_code)
    return 0;
  // Verify param: shift_pressed; type: bool_byref
  DCHECK(shift_pressed);
  if (!shift_pressed)
    return 0;
  // Verify param: ctrl_pressed; type: bool_byref
  DCHECK(ctrl_pressed);
  if (!ctrl_pressed)
    return 0;
  // Verify param: alt_pressed; type: bool_byref
  DCHECK(alt_pressed);
  if (!alt_pressed)
    return 0;

  // Translate param: key_code; type: simple_byref
  int key_codeVal = key_code?*key_code:0;
  // Translate param: shift_pressed; type: bool_byref
  bool shift_pressedBool = (shift_pressed && *shift_pressed)?true:false;
  // Translate param: ctrl_pressed; type: bool_byref
  bool ctrl_pressedBool = (ctrl_pressed && *ctrl_pressed)?true:false;
  // Translate param: alt_pressed; type: bool_byref
  bool alt_pressedBool = (alt_pressed && *alt_pressed)?true:false;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->GetAccelerator(
      command_id,
      key_codeVal,
      shift_pressedBool,
      ctrl_pressedBool,
      alt_pressedBool);

  // Restore param: key_code; type: simple_byref
  if (key_code)
    *key_code = key_codeVal;
  // Restore param: shift_pressed; type: bool_byref
  if (shift_pressed)
    *shift_pressed = shift_pressedBool?true:false;
  // Restore param: ctrl_pressed; type: bool_byref
  if (ctrl_pressed)
    *ctrl_pressed = ctrl_pressedBool?true:false;
  // Restore param: alt_pressed; type: bool_byref
  if (alt_pressed)
    *alt_pressed = alt_pressedBool?true:false;

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK menu_model_get_accelerator_at(struct _cef_menu_model_t* self,
    int index, int* key_code, int* shift_pressed, int* ctrl_pressed,
    int* alt_pressed) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: key_code; type: simple_byref
  DCHECK(key_code);
  if (!key_code)
    return 0;
  // Verify param: shift_pressed; type: bool_byref
  DCHECK(shift_pressed);
  if (!shift_pressed)
    return 0;
  // Verify param: ctrl_pressed; type: bool_byref
  DCHECK(ctrl_pressed);
  if (!ctrl_pressed)
    return 0;
  // Verify param: alt_pressed; type: bool_byref
  DCHECK(alt_pressed);
  if (!alt_pressed)
    return 0;

  // Translate param: key_code; type: simple_byref
  int key_codeVal = key_code?*key_code:0;
  // Translate param: shift_pressed; type: bool_byref
  bool shift_pressedBool = (shift_pressed && *shift_pressed)?true:false;
  // Translate param: ctrl_pressed; type: bool_byref
  bool ctrl_pressedBool = (ctrl_pressed && *ctrl_pressed)?true:false;
  // Translate param: alt_pressed; type: bool_byref
  bool alt_pressedBool = (alt_pressed && *alt_pressed)?true:false;

  // Execute
  bool _retval = CefMenuModelCppToC::Get(self)->GetAcceleratorAt(
      index,
      key_codeVal,
      shift_pressedBool,
      ctrl_pressedBool,
      alt_pressedBool);

  // Restore param: key_code; type: simple_byref
  if (key_code)
    *key_code = key_codeVal;
  // Restore param: shift_pressed; type: bool_byref
  if (shift_pressed)
    *shift_pressed = shift_pressedBool?true:false;
  // Restore param: ctrl_pressed; type: bool_byref
  if (ctrl_pressed)
    *ctrl_pressed = ctrl_pressedBool?true:false;
  // Restore param: alt_pressed; type: bool_byref
  if (alt_pressed)
    *alt_pressed = alt_pressedBool?true:false;

  // Return type: bool
  return _retval;
}


// CONSTRUCTOR - Do not edit by hand.

CefMenuModelCppToC::CefMenuModelCppToC(CefMenuModel* cls)
    : CefCppToC<CefMenuModelCppToC, CefMenuModel, cef_menu_model_t>(cls) {
  struct_.struct_.clear = menu_model_clear;
  struct_.struct_.get_count = menu_model_get_count;
  struct_.struct_.add_separator = menu_model_add_separator;
  struct_.struct_.add_item = menu_model_add_item;
  struct_.struct_.add_check_item = menu_model_add_check_item;
  struct_.struct_.add_radio_item = menu_model_add_radio_item;
  struct_.struct_.add_sub_menu = menu_model_add_sub_menu;
  struct_.struct_.insert_separator_at = menu_model_insert_separator_at;
  struct_.struct_.insert_item_at = menu_model_insert_item_at;
  struct_.struct_.insert_check_item_at = menu_model_insert_check_item_at;
  struct_.struct_.insert_radio_item_at = menu_model_insert_radio_item_at;
  struct_.struct_.insert_sub_menu_at = menu_model_insert_sub_menu_at;
  struct_.struct_.remove = menu_model_remove;
  struct_.struct_.remove_at = menu_model_remove_at;
  struct_.struct_.get_index_of = menu_model_get_index_of;
  struct_.struct_.get_command_id_at = menu_model_get_command_id_at;
  struct_.struct_.set_command_id_at = menu_model_set_command_id_at;
  struct_.struct_.get_label = menu_model_get_label;
  struct_.struct_.get_label_at = menu_model_get_label_at;
  struct_.struct_.set_label = menu_model_set_label;
  struct_.struct_.set_label_at = menu_model_set_label_at;
  struct_.struct_.get_type = menu_model_get_type;
  struct_.struct_.get_type_at = menu_model_get_type_at;
  struct_.struct_.get_group_id = menu_model_get_group_id;
  struct_.struct_.get_group_id_at = menu_model_get_group_id_at;
  struct_.struct_.set_group_id = menu_model_set_group_id;
  struct_.struct_.set_group_id_at = menu_model_set_group_id_at;
  struct_.struct_.get_sub_menu = menu_model_get_sub_menu;
  struct_.struct_.get_sub_menu_at = menu_model_get_sub_menu_at;
  struct_.struct_.is_visible = menu_model_is_visible;
  struct_.struct_.is_visible_at = menu_model_is_visible_at;
  struct_.struct_.set_visible = menu_model_set_visible;
  struct_.struct_.set_visible_at = menu_model_set_visible_at;
  struct_.struct_.is_enabled = menu_model_is_enabled;
  struct_.struct_.is_enabled_at = menu_model_is_enabled_at;
  struct_.struct_.set_enabled = menu_model_set_enabled;
  struct_.struct_.set_enabled_at = menu_model_set_enabled_at;
  struct_.struct_.is_checked = menu_model_is_checked;
  struct_.struct_.is_checked_at = menu_model_is_checked_at;
  struct_.struct_.set_checked = menu_model_set_checked;
  struct_.struct_.set_checked_at = menu_model_set_checked_at;
  struct_.struct_.has_accelerator = menu_model_has_accelerator;
  struct_.struct_.has_accelerator_at = menu_model_has_accelerator_at;
  struct_.struct_.set_accelerator = menu_model_set_accelerator;
  struct_.struct_.set_accelerator_at = menu_model_set_accelerator_at;
  struct_.struct_.remove_accelerator = menu_model_remove_accelerator;
  struct_.struct_.remove_accelerator_at = menu_model_remove_accelerator_at;
  struct_.struct_.get_accelerator = menu_model_get_accelerator;
  struct_.struct_.get_accelerator_at = menu_model_get_accelerator_at;
}

#ifndef NDEBUG
template<> base::AtomicRefCount CefCppToC<CefMenuModelCppToC, CefMenuModel,
    cef_menu_model_t>::DebugObjCt = 0;
#endif

