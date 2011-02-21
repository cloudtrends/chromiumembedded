// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// ---------------------------------------------------------------------------
//
// A portion of this file was generated by the CEF translator tool.  When
// making changes by hand only do so within the body of existing static and
// virtual method implementations. See the translator.README.txt file in the
// tools directory for more information.
//

#include "libcef_dll/cpptoc/base_cpptoc.h"
#include "libcef_dll/cpptoc/v8handler_cpptoc.h"
#include "libcef_dll/ctocpp/v8context_ctocpp.h"
#include "libcef_dll/ctocpp/v8value_ctocpp.h"
#include "libcef_dll/transfer_util.h"


// STATIC METHODS - Body may be edited by hand.

CefRefPtr<CefV8Value> CefV8Value::CreateUndefined()
{
  cef_v8value_t* impl = cef_v8value_create_undefined();
  if(impl)
    return CefV8ValueCToCpp::Wrap(impl);
  return NULL;
}

CefRefPtr<CefV8Value> CefV8Value::CreateNull()
{
  cef_v8value_t* impl = cef_v8value_create_null();
  if(impl)
    return CefV8ValueCToCpp::Wrap(impl);
  return NULL;
}

CefRefPtr<CefV8Value> CefV8Value::CreateBool(bool value)
{
  cef_v8value_t* impl = cef_v8value_create_bool(value);
  if(impl)
    return CefV8ValueCToCpp::Wrap(impl);
  return NULL;
}

CefRefPtr<CefV8Value> CefV8Value::CreateInt(int value)
{
  cef_v8value_t* impl = cef_v8value_create_int(value);
  if(impl)
    return CefV8ValueCToCpp::Wrap(impl);
  return NULL;
}

CefRefPtr<CefV8Value> CefV8Value::CreateDouble(double value)
{
  cef_v8value_t* impl = cef_v8value_create_double(value);
  if(impl)
    return CefV8ValueCToCpp::Wrap(impl);
  return NULL;
}

CefRefPtr<CefV8Value> CefV8Value::CreateString(const CefString& value)
{
  cef_v8value_t* impl = cef_v8value_create_string(value.GetStruct());
  if(impl)
    return CefV8ValueCToCpp::Wrap(impl);
  return NULL;
}

CefRefPtr<CefV8Value> CefV8Value::CreateObject(CefRefPtr<CefBase> user_data)
{
  cef_base_t* baseStruct = NULL;
  if(user_data)
    baseStruct = CefBaseCppToC::Wrap(user_data);

  cef_v8value_t* impl = cef_v8value_create_object(baseStruct);
  if(impl)
    return CefV8ValueCToCpp::Wrap(impl);
  return NULL;
}

CefRefPtr<CefV8Value> CefV8Value::CreateArray()
{
  cef_v8value_t* impl = cef_v8value_create_array();
  if(impl)
    return CefV8ValueCToCpp::Wrap(impl);
  return NULL;
}

CefRefPtr<CefV8Value> CefV8Value::CreateFunction(const CefString& name,
    CefRefPtr<CefV8Handler> handler)
{
  cef_v8handler_t* handlerStruct = NULL;
  if(handler.get())
    handlerStruct = CefV8HandlerCppToC::Wrap(handler);

  cef_v8value_t* impl = cef_v8value_create_function(name.GetStruct(),
      handlerStruct);
  if(impl)
    return CefV8ValueCToCpp::Wrap(impl);
  return NULL;
}


// VIRTUAL METHODS - Body may be edited by hand.

bool CefV8ValueCToCpp::IsUndefined()
{
  if(CEF_MEMBER_MISSING(struct_, is_undefined))
    return false;

  return struct_->is_undefined(struct_)?true:false;
}

bool CefV8ValueCToCpp::IsNull()
{
  if(CEF_MEMBER_MISSING(struct_, is_null))
    return false;

  return struct_->is_null(struct_)?true:false;
}

bool CefV8ValueCToCpp::IsBool()
{
  if(CEF_MEMBER_MISSING(struct_, is_bool))
    return false;

  return struct_->is_bool(struct_)?true:false;
}

bool CefV8ValueCToCpp::IsInt()
{
  if(CEF_MEMBER_MISSING(struct_, is_int))
    return false;

  return struct_->is_int(struct_)?true:false;
}

bool CefV8ValueCToCpp::IsDouble()
{
  if(CEF_MEMBER_MISSING(struct_, is_double))
    return false;

  return struct_->is_double(struct_)?true:false;
}

bool CefV8ValueCToCpp::IsString()
{
  if(CEF_MEMBER_MISSING(struct_, is_string))
    return false;

  return struct_->is_string(struct_)?true:false;
}

bool CefV8ValueCToCpp::IsObject()
{
  if(CEF_MEMBER_MISSING(struct_, is_object))
    return false;

  return struct_->is_object(struct_)?true:false;
}

bool CefV8ValueCToCpp::IsArray()
{
  if(CEF_MEMBER_MISSING(struct_, is_array))
    return false;

  return struct_->is_array(struct_)?true:false;
}

bool CefV8ValueCToCpp::IsFunction()
{
  if(CEF_MEMBER_MISSING(struct_, is_function))
    return false;

  return struct_->is_function(struct_)?true:false;
}

bool CefV8ValueCToCpp::GetBoolValue()
{
  if(CEF_MEMBER_MISSING(struct_, get_bool_value))
    return false;

  return struct_->get_bool_value(struct_)?true:false;
}

int CefV8ValueCToCpp::GetIntValue()
{
  if(CEF_MEMBER_MISSING(struct_, get_int_value))
    return 0;

  return struct_->get_int_value(struct_);
}

double CefV8ValueCToCpp::GetDoubleValue()
{
  if(CEF_MEMBER_MISSING(struct_, get_double_value))
    return 0.;

  return struct_->get_double_value(struct_);
}

CefString CefV8ValueCToCpp::GetStringValue()
{
  CefString str;
  if(CEF_MEMBER_MISSING(struct_, get_string_value))
    return str;

  cef_string_userfree_t strPtr = struct_->get_string_value(struct_);
  str.AttachToUserFree(strPtr);
  return str;
}

bool CefV8ValueCToCpp::HasValue(const CefString& key)
{
  if(CEF_MEMBER_MISSING(struct_, has_value_bykey))
    return false;

  return struct_->has_value_bykey(struct_, key.GetStruct())?true:false;
}

bool CefV8ValueCToCpp::HasValue(int index)
{
  if(CEF_MEMBER_MISSING(struct_, has_value_byindex))
    return false;

  return struct_->has_value_byindex(struct_, index)?true:false;
}

bool CefV8ValueCToCpp::DeleteValue(const CefString& key)
{
  if(CEF_MEMBER_MISSING(struct_, delete_value_bykey))
    return false;

  return struct_->delete_value_bykey(struct_, key.GetStruct())?true:false;
}

bool CefV8ValueCToCpp::DeleteValue(int index)
{
  if(CEF_MEMBER_MISSING(struct_, delete_value_byindex))
    return false;

  return struct_->delete_value_byindex(struct_, index)?true:false;
}

CefRefPtr<CefV8Value> CefV8ValueCToCpp::GetValue(const CefString& key)
{
  if(CEF_MEMBER_MISSING(struct_, get_value_bykey))
    return false;

  cef_v8value_t* valueStruct = struct_->get_value_bykey(struct_,
      key.GetStruct());
  if(valueStruct)
    return CefV8ValueCToCpp::Wrap(valueStruct);
  return NULL;
}

CefRefPtr<CefV8Value> CefV8ValueCToCpp::GetValue(int index)
{
  if(CEF_MEMBER_MISSING(struct_, get_value_byindex))
    return false;

  cef_v8value_t* valueStruct = struct_->get_value_byindex(struct_, index);
  if(valueStruct)
    return CefV8ValueCToCpp::Wrap(valueStruct);
  return NULL;
}

bool CefV8ValueCToCpp::SetValue(const CefString& key,
    CefRefPtr<CefV8Value> value)
{
  if(CEF_MEMBER_MISSING(struct_, set_value_bykey))
    return false;

  return struct_->set_value_bykey(struct_, key.GetStruct(),
      CefV8ValueCToCpp::Unwrap(value))?true:false;
}

bool CefV8ValueCToCpp::SetValue(int index, CefRefPtr<CefV8Value> value)
{
  if(CEF_MEMBER_MISSING(struct_, set_value_byindex))
    return false;

  return struct_->set_value_byindex(struct_, index,
      CefV8ValueCToCpp::Unwrap(value))?true:false;
}

bool CefV8ValueCToCpp::GetKeys(std::vector<CefString>& keys)
{
  if(CEF_MEMBER_MISSING(struct_, get_keys))
    return false;

  cef_string_list_t list = cef_string_list_alloc();
  if(struct_->get_keys(struct_, list)) {
    transfer_string_list_contents(list, keys);
    cef_string_list_free(list);
    return true;
  }
  return false;
}

CefRefPtr<CefBase> CefV8ValueCToCpp::GetUserData()
{
  if(CEF_MEMBER_MISSING(struct_, get_user_data))
    return false;

  cef_base_t* baseStruct = struct_->get_user_data(struct_);
  if(baseStruct)
    return CefBaseCppToC::Unwrap(baseStruct);
  return NULL;
}

int CefV8ValueCToCpp::GetArrayLength()
{
  if(CEF_MEMBER_MISSING(struct_, get_array_length))
    return 0;

  return struct_->get_array_length(struct_);
}

CefString CefV8ValueCToCpp::GetFunctionName()
{
  CefString str;
  if(CEF_MEMBER_MISSING(struct_, get_function_name))
    return str;

  cef_string_userfree_t strPtr = struct_->get_function_name(struct_);
  str.AttachToUserFree(strPtr);
  return str;
}

CefRefPtr<CefV8Handler> CefV8ValueCToCpp::GetFunctionHandler()
{
  if(CEF_MEMBER_MISSING(struct_, get_function_handler))
    return false;

  cef_v8handler_t* handlerStruct = struct_->get_function_handler(struct_);
  if(handlerStruct)
    return CefV8HandlerCppToC::Unwrap(handlerStruct);
  return NULL;
}

bool CefV8ValueCToCpp::ExecuteFunction(CefRefPtr<CefV8Value> object,
    const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
    CefString& exception)
{
  if(CEF_MEMBER_MISSING(struct_, execute_function))
    return false;

  cef_v8value_t** argsStructPtr = NULL;
  int argsSize = arguments.size();
  if(argsSize > 0) {
    argsStructPtr = new cef_v8value_t*[argsSize];
    for(int i = 0; i < argsSize; ++i)
      argsStructPtr[i] = CefV8ValueCToCpp::Unwrap(arguments[i]);
  }

  cef_v8value_t* retvalStruct = NULL;

  int rv = struct_->execute_function(struct_, 
      object.get() ? CefV8ValueCToCpp::Unwrap(object): NULL,
      argsSize, argsStructPtr, &retvalStruct, exception.GetWritableStruct());
  if(retvalStruct)
    retval = CefV8ValueCToCpp::Wrap(retvalStruct);

  if(argsStructPtr)
    delete [] argsStructPtr;

  return rv?true:false;
}

bool CefV8ValueCToCpp::ExecuteFunctionWithContext(
    CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Value> object,
    const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
    CefString& exception)
{
  if(CEF_MEMBER_MISSING(struct_, execute_function_with_context))
    return false;

  cef_v8value_t** argsStructPtr = NULL;
  int argsSize = arguments.size();
  if(argsSize > 0) {
    argsStructPtr = new cef_v8value_t*[argsSize];
    for(int i = 0; i < argsSize; ++i)
      argsStructPtr[i] = CefV8ValueCToCpp::Unwrap(arguments[i]);
  }

  cef_v8value_t* retvalStruct = NULL;

  int rv = struct_->execute_function_with_context(struct_, 
      context.get() ? CefV8ContextCToCpp::Unwrap(context): NULL, 
      object.get() ? CefV8ValueCToCpp::Unwrap(object): NULL, 
      argsSize, argsStructPtr, &retvalStruct, exception.GetWritableStruct());

  if(retvalStruct)
    retval = CefV8ValueCToCpp::Wrap(retvalStruct);

  if(argsStructPtr)
    delete [] argsStructPtr;

  return rv?true:false;
}


#ifdef _DEBUG
template<> long CefCToCpp<CefV8ValueCToCpp, CefV8Value,
    cef_v8value_t>::DebugObjCt = 0;
#endif

