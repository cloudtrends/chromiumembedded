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

#include "libcef_dll/cpptoc/v8context_cpptoc.h"
#include "libcef_dll/cpptoc/v8exception_cpptoc.h"
#include "libcef_dll/cpptoc/v8value_cpptoc.h"
#include "libcef_dll/ctocpp/base_ctocpp.h"
#include "libcef_dll/ctocpp/v8accessor_ctocpp.h"
#include "libcef_dll/ctocpp/v8handler_ctocpp.h"
#include "libcef_dll/transfer_util.h"


// GLOBAL FUNCTIONS - Body may be edited by hand.

CEF_EXPORT cef_v8value_t* cef_v8value_create_undefined()
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  CefRefPtr<CefV8Value> _retval = CefV8Value::CreateUndefined();

  // Return type: refptr_same
  return CefV8ValueCppToC::Wrap(_retval);
}

CEF_EXPORT cef_v8value_t* cef_v8value_create_null()
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  CefRefPtr<CefV8Value> _retval = CefV8Value::CreateNull();

  // Return type: refptr_same
  return CefV8ValueCppToC::Wrap(_retval);
}

CEF_EXPORT cef_v8value_t* cef_v8value_create_bool(int value)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  CefRefPtr<CefV8Value> _retval = CefV8Value::CreateBool(
      value?true:false);

  // Return type: refptr_same
  return CefV8ValueCppToC::Wrap(_retval);
}

CEF_EXPORT cef_v8value_t* cef_v8value_create_int(int value)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  CefRefPtr<CefV8Value> _retval = CefV8Value::CreateInt(
      value);

  // Return type: refptr_same
  return CefV8ValueCppToC::Wrap(_retval);
}

CEF_EXPORT cef_v8value_t* cef_v8value_create_double(double value)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  CefRefPtr<CefV8Value> _retval = CefV8Value::CreateDouble(
      value);

  // Return type: refptr_same
  return CefV8ValueCppToC::Wrap(_retval);
}

CEF_EXPORT cef_v8value_t* cef_v8value_create_date(const cef_time_t* date)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: date; type: simple_byref_const
  DCHECK(date);
  if (!date)
    return NULL;

  // Translate param: date; type: simple_byref_const
  CefTime dateVal = date?*date:CefTime();

  // Execute
  CefRefPtr<CefV8Value> _retval = CefV8Value::CreateDate(
      dateVal);

  // Return type: refptr_same
  return CefV8ValueCppToC::Wrap(_retval);
}

CEF_EXPORT cef_v8value_t* cef_v8value_create_string(const cef_string_t* value)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Unverified params: value

  // Execute
  CefRefPtr<CefV8Value> _retval = CefV8Value::CreateString(
      CefString(value));

  // Return type: refptr_same
  return CefV8ValueCppToC::Wrap(_retval);
}

CEF_EXPORT cef_v8value_t* cef_v8value_create_object_with_accessor(
    cef_base_t* user_data, cef_v8accessor_t* accessor)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Unverified params: user_data, accessor

  // Execute
  CefRefPtr<CefV8Value> _retval = CefV8Value::CreateObject(
      CefBaseCToCpp::Wrap(user_data),
      CefV8AccessorCToCpp::Wrap(accessor));

  // Return type: refptr_same
  return CefV8ValueCppToC::Wrap(_retval);
}

CEF_EXPORT cef_v8value_t* cef_v8value_create_array()
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  CefRefPtr<CefV8Value> _retval = CefV8Value::CreateArray();

  // Return type: refptr_same
  return CefV8ValueCppToC::Wrap(_retval);
}

CEF_EXPORT cef_v8value_t* cef_v8value_create_function(const cef_string_t* name,
    cef_v8handler_t* handler)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: name; type: string_byref_const
  DCHECK(name);
  if (!name)
    return NULL;
  // Verify param: handler; type: refptr_diff
  DCHECK(handler);
  if (!handler)
    return NULL;

  // Execute
  CefRefPtr<CefV8Value> _retval = CefV8Value::CreateFunction(
      CefString(name),
      CefV8HandlerCToCpp::Wrap(handler));

  // Return type: refptr_same
  return CefV8ValueCppToC::Wrap(_retval);
}


// MEMBER FUNCTIONS - Body may be edited by hand.

int CEF_CALLBACK v8value_is_undefined(struct _cef_v8value_t* self)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->IsUndefined();

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8value_is_null(struct _cef_v8value_t* self)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->IsNull();

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8value_is_bool(struct _cef_v8value_t* self)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->IsBool();

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8value_is_int(struct _cef_v8value_t* self)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->IsInt();

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8value_is_double(struct _cef_v8value_t* self)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->IsDouble();

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8value_is_date(struct _cef_v8value_t* self)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->IsDate();

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8value_is_string(struct _cef_v8value_t* self)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->IsString();

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8value_is_object(struct _cef_v8value_t* self)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->IsObject();

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8value_is_array(struct _cef_v8value_t* self)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->IsArray();

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8value_is_function(struct _cef_v8value_t* self)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->IsFunction();

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8value_is_same(struct _cef_v8value_t* self,
    struct _cef_v8value_t* that)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: that; type: refptr_same
  DCHECK(that);
  if (!that)
    return 0;

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->IsSame(
      CefV8ValueCppToC::Unwrap(that));

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8value_get_bool_value(struct _cef_v8value_t* self)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->GetBoolValue();

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8value_get_int_value(struct _cef_v8value_t* self)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  int _retval = CefV8ValueCppToC::Get(self)->GetIntValue();

  // Return type: simple
  return _retval;
}

double CEF_CALLBACK v8value_get_double_value(struct _cef_v8value_t* self)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  double _retval = CefV8ValueCppToC::Get(self)->GetDoubleValue();

  // Return type: simple
  return _retval;
}

cef_time_t CEF_CALLBACK v8value_get_date_value(struct _cef_v8value_t* self)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return CefTime();

  // Execute
  cef_time_t _retval = CefV8ValueCppToC::Get(self)->GetDateValue();

  // Return type: simple
  return _retval;
}

cef_string_userfree_t CEF_CALLBACK v8value_get_string_value(
    struct _cef_v8value_t* self)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return NULL;

  // Execute
  CefString _retval = CefV8ValueCppToC::Get(self)->GetStringValue();

  // Return type: string
  return _retval.DetachToUserFree();
}

int CEF_CALLBACK v8value_has_value_bykey(struct _cef_v8value_t* self,
    const cef_string_t* key)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: key; type: string_byref_const
  DCHECK(key);
  if (!key)
    return 0;

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->HasValue(
      CefString(key));

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8value_has_value_byindex(struct _cef_v8value_t* self,
    int index)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: index; type: simple_byval
  DCHECK(index >= 0);
  if (index < 0)
    return 0;

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->HasValue(
      index);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8value_delete_value_bykey(struct _cef_v8value_t* self,
    const cef_string_t* key)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: key; type: string_byref_const
  DCHECK(key);
  if (!key)
    return 0;

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->DeleteValue(
      CefString(key));

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8value_delete_value_byindex(struct _cef_v8value_t* self,
    int index)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: index; type: simple_byval
  DCHECK(index >= 0);
  if (index < 0)
    return 0;

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->DeleteValue(
      index);

  // Return type: bool
  return _retval;
}

struct _cef_v8value_t* CEF_CALLBACK v8value_get_value_bykey(
    struct _cef_v8value_t* self, const cef_string_t* key)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return NULL;
  // Verify param: key; type: string_byref_const
  DCHECK(key);
  if (!key)
    return NULL;

  // Execute
  CefRefPtr<CefV8Value> _retval = CefV8ValueCppToC::Get(self)->GetValue(
      CefString(key));

  // Return type: refptr_same
  return CefV8ValueCppToC::Wrap(_retval);
}

struct _cef_v8value_t* CEF_CALLBACK v8value_get_value_byindex(
    struct _cef_v8value_t* self, int index)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return NULL;
  // Verify param: index; type: simple_byval
  DCHECK(index >= 0);
  if (index < 0)
    return NULL;

  // Execute
  CefRefPtr<CefV8Value> _retval = CefV8ValueCppToC::Get(self)->GetValue(
      index);

  // Return type: refptr_same
  return CefV8ValueCppToC::Wrap(_retval);
}

int CEF_CALLBACK v8value_set_value_bykey(struct _cef_v8value_t* self,
    const cef_string_t* key, struct _cef_v8value_t* value,
    enum cef_v8_propertyattribute_t attribute)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: key; type: string_byref_const
  DCHECK(key);
  if (!key)
    return 0;
  // Verify param: value; type: refptr_same
  DCHECK(value);
  if (!value)
    return 0;

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->SetValue(
      CefString(key),
      CefV8ValueCppToC::Unwrap(value),
      attribute);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8value_set_value_byindex(struct _cef_v8value_t* self,
    int index, struct _cef_v8value_t* value)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: index; type: simple_byval
  DCHECK(index >= 0);
  if (index < 0)
    return 0;
  // Verify param: value; type: refptr_same
  DCHECK(value);
  if (!value)
    return 0;

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->SetValue(
      index,
      CefV8ValueCppToC::Unwrap(value));

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8value_set_value_byaccessor(struct _cef_v8value_t* self,
    const cef_string_t* key, enum cef_v8_accesscontrol_t settings,
    enum cef_v8_propertyattribute_t attribute)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: key; type: string_byref_const
  DCHECK(key);
  if (!key)
    return 0;

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->SetValue(
      CefString(key),
      settings,
      attribute);

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8value_get_keys(struct _cef_v8value_t* self,
    cef_string_list_t keys)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: keys; type: string_vec_byref
  DCHECK(keys);
  if (!keys)
    return 0;

  // Translate param: keys; type: string_vec_byref
  std::vector<CefString> keysList;
  transfer_string_list_contents(keys, keysList);

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->GetKeys(
      keysList);

  // Restore param: keys; type: string_vec_byref
  cef_string_list_clear(keys);
  transfer_string_list_contents(keysList, keys);

  // Return type: bool
  return _retval;
}

cef_base_t* CEF_CALLBACK v8value_get_user_data(struct _cef_v8value_t* self)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return NULL;

  // Execute
  CefRefPtr<CefBase> _retval = CefV8ValueCppToC::Get(self)->GetUserData();

  // Return type: refptr_diff
  return CefBaseCToCpp::Unwrap(_retval);
}

int CEF_CALLBACK v8value_get_array_length(struct _cef_v8value_t* self)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  int _retval = CefV8ValueCppToC::Get(self)->GetArrayLength();

  // Return type: simple
  return _retval;
}

cef_string_userfree_t CEF_CALLBACK v8value_get_function_name(
    struct _cef_v8value_t* self)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return NULL;

  // Execute
  CefString _retval = CefV8ValueCppToC::Get(self)->GetFunctionName();

  // Return type: string
  return _retval.DetachToUserFree();
}

cef_v8handler_t* CEF_CALLBACK v8value_get_function_handler(
    struct _cef_v8value_t* self)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return NULL;

  // Execute
  CefRefPtr<CefV8Handler> _retval = CefV8ValueCppToC::Get(
      self)->GetFunctionHandler();

  // Return type: refptr_diff
  return CefV8HandlerCToCpp::Unwrap(_retval);
}

int CEF_CALLBACK v8value_execute_function(struct _cef_v8value_t* self,
    struct _cef_v8value_t* object, size_t argumentsCount,
    struct _cef_v8value_t* const* arguments, struct _cef_v8value_t** retval,
    cef_v8exception_t** exception, int rethrow_exception)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: arguments; type: refptr_vec_same_byref_const
  DCHECK(argumentsCount == 0 || arguments);
  if (argumentsCount > 0 && !arguments)
    return 0;
  // Verify param: retval; type: refptr_same_byref
  DCHECK(retval);
  if (!retval)
    return 0;
  // Verify param: exception; type: refptr_same_byref
  DCHECK(exception);
  if (!exception)
    return 0;
  // Unverified params: object

  // Translate param: arguments; type: refptr_vec_same_byref_const
  std::vector<CefRefPtr<CefV8Value> > argumentsList;
  if (argumentsCount > 0) {
    for (size_t i = 0; i < argumentsCount; ++i) {
      argumentsList.push_back(CefV8ValueCppToC::Unwrap(arguments[i]));
    }
  }
  // Translate param: retval; type: refptr_same_byref
  CefRefPtr<CefV8Value> retvalPtr;
  if (retval && *retval)
    retvalPtr = CefV8ValueCppToC::Unwrap(*retval);
  CefV8Value* retvalOrig = retvalPtr.get();
  // Translate param: exception; type: refptr_same_byref
  CefRefPtr<CefV8Exception> exceptionPtr;
  if (exception && *exception)
    exceptionPtr = CefV8ExceptionCppToC::Unwrap(*exception);
  CefV8Exception* exceptionOrig = exceptionPtr.get();

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->ExecuteFunction(
      CefV8ValueCppToC::Unwrap(object),
      argumentsList,
      retvalPtr,
      exceptionPtr,
      rethrow_exception?true:false);

  // Restore param: retval; type: refptr_same_byref
  if (retval) {
    if (retvalPtr.get()) {
      if (retvalPtr.get() != retvalOrig) {
        *retval = CefV8ValueCppToC::Wrap(retvalPtr);
      }
    } else {
      *retval = NULL;
    }
  }
  // Restore param: exception; type: refptr_same_byref
  if (exception) {
    if (exceptionPtr.get()) {
      if (exceptionPtr.get() != exceptionOrig) {
        *exception = CefV8ExceptionCppToC::Wrap(exceptionPtr);
      }
    } else {
      *exception = NULL;
    }
  }

  // Return type: bool
  return _retval;
}

int CEF_CALLBACK v8value_execute_function_with_context(
    struct _cef_v8value_t* self, cef_v8context_t* context,
    struct _cef_v8value_t* object, size_t argumentsCount,
    struct _cef_v8value_t* const* arguments, struct _cef_v8value_t** retval,
    cef_v8exception_t** exception, int rethrow_exception)
{
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;
  // Verify param: context; type: refptr_same
  DCHECK(context);
  if (!context)
    return 0;
  // Verify param: arguments; type: refptr_vec_same_byref_const
  DCHECK(argumentsCount == 0 || arguments);
  if (argumentsCount > 0 && !arguments)
    return 0;
  // Verify param: retval; type: refptr_same_byref
  DCHECK(retval);
  if (!retval)
    return 0;
  // Verify param: exception; type: refptr_same_byref
  DCHECK(exception);
  if (!exception)
    return 0;
  // Unverified params: object

  // Translate param: arguments; type: refptr_vec_same_byref_const
  std::vector<CefRefPtr<CefV8Value> > argumentsList;
  if (argumentsCount > 0) {
    for (size_t i = 0; i < argumentsCount; ++i) {
      argumentsList.push_back(CefV8ValueCppToC::Unwrap(arguments[i]));
    }
  }
  // Translate param: retval; type: refptr_same_byref
  CefRefPtr<CefV8Value> retvalPtr;
  if (retval && *retval)
    retvalPtr = CefV8ValueCppToC::Unwrap(*retval);
  CefV8Value* retvalOrig = retvalPtr.get();
  // Translate param: exception; type: refptr_same_byref
  CefRefPtr<CefV8Exception> exceptionPtr;
  if (exception && *exception)
    exceptionPtr = CefV8ExceptionCppToC::Unwrap(*exception);
  CefV8Exception* exceptionOrig = exceptionPtr.get();

  // Execute
  bool _retval = CefV8ValueCppToC::Get(self)->ExecuteFunctionWithContext(
      CefV8ContextCppToC::Unwrap(context),
      CefV8ValueCppToC::Unwrap(object),
      argumentsList,
      retvalPtr,
      exceptionPtr,
      rethrow_exception?true:false);

  // Restore param: retval; type: refptr_same_byref
  if (retval) {
    if (retvalPtr.get()) {
      if (retvalPtr.get() != retvalOrig) {
        *retval = CefV8ValueCppToC::Wrap(retvalPtr);
      }
    } else {
      *retval = NULL;
    }
  }
  // Restore param: exception; type: refptr_same_byref
  if (exception) {
    if (exceptionPtr.get()) {
      if (exceptionPtr.get() != exceptionOrig) {
        *exception = CefV8ExceptionCppToC::Wrap(exceptionPtr);
      }
    } else {
      *exception = NULL;
    }
  }

  // Return type: bool
  return _retval;
}


// CONSTRUCTOR - Do not edit by hand.

CefV8ValueCppToC::CefV8ValueCppToC(CefV8Value* cls)
    : CefCppToC<CefV8ValueCppToC, CefV8Value, cef_v8value_t>(cls)
{
  struct_.struct_.is_undefined = v8value_is_undefined;
  struct_.struct_.is_null = v8value_is_null;
  struct_.struct_.is_bool = v8value_is_bool;
  struct_.struct_.is_int = v8value_is_int;
  struct_.struct_.is_double = v8value_is_double;
  struct_.struct_.is_date = v8value_is_date;
  struct_.struct_.is_string = v8value_is_string;
  struct_.struct_.is_object = v8value_is_object;
  struct_.struct_.is_array = v8value_is_array;
  struct_.struct_.is_function = v8value_is_function;
  struct_.struct_.is_same = v8value_is_same;
  struct_.struct_.get_bool_value = v8value_get_bool_value;
  struct_.struct_.get_int_value = v8value_get_int_value;
  struct_.struct_.get_double_value = v8value_get_double_value;
  struct_.struct_.get_date_value = v8value_get_date_value;
  struct_.struct_.get_string_value = v8value_get_string_value;
  struct_.struct_.has_value_bykey = v8value_has_value_bykey;
  struct_.struct_.has_value_byindex = v8value_has_value_byindex;
  struct_.struct_.delete_value_bykey = v8value_delete_value_bykey;
  struct_.struct_.delete_value_byindex = v8value_delete_value_byindex;
  struct_.struct_.get_value_bykey = v8value_get_value_bykey;
  struct_.struct_.get_value_byindex = v8value_get_value_byindex;
  struct_.struct_.set_value_bykey = v8value_set_value_bykey;
  struct_.struct_.set_value_byindex = v8value_set_value_byindex;
  struct_.struct_.set_value_byaccessor = v8value_set_value_byaccessor;
  struct_.struct_.get_keys = v8value_get_keys;
  struct_.struct_.get_user_data = v8value_get_user_data;
  struct_.struct_.get_array_length = v8value_get_array_length;
  struct_.struct_.get_function_name = v8value_get_function_name;
  struct_.struct_.get_function_handler = v8value_get_function_handler;
  struct_.struct_.execute_function = v8value_execute_function;
  struct_.struct_.execute_function_with_context =
      v8value_execute_function_with_context;
}

#ifndef NDEBUG
template<> long CefCppToC<CefV8ValueCppToC, CefV8Value,
    cef_v8value_t>::DebugObjCt = 0;
#endif

