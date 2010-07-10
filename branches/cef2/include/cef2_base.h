// Copyright (c) 2010 Marshall A. Greenblatt. Portions Copyright (c)
// 2006-2008 Google Inc. All rights reserved.
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


#ifndef _CEF2_BASE_H
#define _CEF2_BASE_H

// Bring in platform-specific definitions.
#ifdef _WIN32
#include "cef2_win.h"
#endif


// Interface defining the the reference count implementation methods. All
// framework classes must implement the CefBase class.
class CefBase
{
public:
  // The AddRef method increments the reference count for the object. It should
  // be called for every new copy of a pointer to a given object. The resulting
  // reference count value is returned and should be used for diagnostic/testing
  // purposes only.
  virtual int AddRef() =0;

  // The Release method decrements the reference count for the object. If the
  // reference count on the object falls to 0, then the object should free
  // itself from memory.  The resulting reference count value is returned and
  // should be used for diagnostic/testing purposes only.
  virtual int Release() =0;

  // Return the current number of references.
  virtual int GetRefCt() = 0;
};

// Template that provides atomic implementations of AddRef() and Release()
// along with Lock() and Unlock() methods to protect critical sections of
// code from simultaneous access by multiple threads.
//
// The below example demonstrates how to use the CefThreadSafeBase template.
//
// class MyHandler : public CefThreadSafeBase<CefHandler>
// {
//    std::wstring m_title;
//
//    virtual RetVal HandleTitleChange(const std::wstring& title)
//    {
//       Lock();   // Begin protecting code
//       m_title = title;
//       Unlock(); // Done protecting code
//       return RV_HANDLED;
//    }
//    ...
// }
//
template <class ClassName>
class CefThreadSafeBase : public ClassName
{
public:
  CefThreadSafeBase()
  {
    m_dwRef = 0L;
  }
  virtual ~CefThreadSafeBase()
  {
  }

  // Atomic reference increment.
  virtual int AddRef()
  {
    return CefAtomicIncrement(&m_dwRef);
  }

  // Atomic reference decrement.  Delete this object when no references remain.
  virtual int Release()
  {
    int retval = CefAtomicDecrement(&m_dwRef);
    if(retval == 0)
      delete this;
    return retval;
  }

  // Return the current number of references.
  virtual int GetRefCt() { return m_dwRef; }

  // Use the Lock() and Unlock() methods to protect a section of code from
  // simultaneous access by multiple threads.
  void Lock() { m_critsec.Lock(); }
  void Unlock() { m_critsec.Unlock(); }

  // Provide this method required by Chrome when posting tasks.
  static bool ImplementsThreadSafeReferenceCounting() { return true; }

  // A helper class that acquires the lock for the given CefThreadSafeBase while
  // the CefAutoLock is in scope.
  class AutoLock
  {
  public:
    AutoLock(CefThreadSafeBase* base) : base_(base)
    {
      base_->Lock();
    }
    ~AutoLock()
    {
      base_->Unlock();
    }

  private:
    CefThreadSafeBase* base_;
  };

protected:
  long m_dwRef;
  CefCriticalSection m_critsec;
};


// Smart pointer implementation borrowed from base/ref_counted.h
//
// A smart pointer class for reference counted objects.  Use this class instead
// of calling AddRef and Release manually on a reference counted object to
// avoid common memory leaks caused by forgetting to Release an object
// reference.  Sample usage:
//
//   class MyFoo : public CefBase {
//    ...
//   };
//
//   void some_function() {
//     // The MyFoo object that |foo| represents starts with a single
//     // reference.
//     CefRefPtr<MyFoo> foo = new MyFoo();
//     foo->Method(param);
//     // |foo| is released when this function returns
//   }
//
//   void some_other_function() {
//     CefRefPtr<MyFoo> foo = new MyFoo();
//     ...
//     foo = NULL;  // explicitly releases |foo|
//     ...
//     if (foo)
//       foo->Method(param);
//   }
//
// The above examples show how CefRefPtr<T> acts like a pointer to T.
// Given two CefRefPtr<T> classes, it is also possible to exchange
// references between the two objects, like so:
//
//   {
//     CefRefPtr<MyFoo> a = new MyFoo();
//     CefRefPtr<MyFoo> b;
//
//     b.swap(a);
//     // now, |b| references the MyFoo object, and |a| references NULL.
//   }
//
// To make both |a| and |b| in the above example reference the same MyFoo
// object, simply use the assignment operator:
//
//   {
//     CefRefPtr<MyFoo> a = new MyFoo();
//     CefRefPtr<MyFoo> b;
//
//     b = a;
//     // now, |a| and |b| each own a reference to the same MyFoo object.
//     // the reference count of the underlying MyFoo object will be 2.
//   }
//
// Reference counted objects can also be passed as function parameters and
// used as function return values:
//
//   void some_func_with_param(CefRefPtr<MyFoo> param) {
//     // A reference is added to the MyFoo object that |param| represents
//     // during the scope of some_func_with_param() and released when
//     // some_func_with_param() goes out of scope.
//   }
//
//   CefRefPtr<MyFoo> some_func_with_retval() {
//     // The MyFoo object that |foox| represents starts with a single
//     // reference.
//     CefRefPtr<MyFoo> foox = new MyFoo();
//
//     // Creating the return value adds an additional reference.
//     return foox;
//
//     // When some_func_with_retval() goes out of scope the original |foox|
//     // reference is released.
//   }
//
//   void and_another_function() {
//     CefRefPtr<MyFoo> foo = new MyFoo();
//
//     // pass |foo| as a parameter.
//     some_function(foo);
//
//     CefRefPtr<MyFoo> foo2 = some_func_with_retval();
//     // Now, since we kept a reference to the some_func_with_retval() return
//     // value, |foo2| is the only class pointing to the MyFoo object created
//     in some_func_with_retval(), and it has a reference count of 1.
//
//     some_func_with_retval();
//     // Now, since we didn't keep a reference to the some_func_with_retval()
//     // return value, the MyFoo object created in some_func_with_retval()
//     // will automatically be released.
//   }
//
// And in standard containers:
//
//   {
//      // Create a vector that holds MyFoo objects.
//      std::vector<CefRefPtr<MyFoo> > MyFooVec;
//
//     // The MyFoo object that |foo| represents starts with a single
//     // reference.
//     CefRefPtr<MyFoo> foo = new MyFoo();
//
//     // When the MyFoo object is added to |MyFooVec| the reference count
//     // is increased to 2.
//     MyFooVec.push_back(foo);
//   }
//
template <class T>
class CefRefPtr {
 public:
  CefRefPtr() : ptr_(NULL) {
  }

  CefRefPtr(T* p) : ptr_(p) {
    if (ptr_)
      ptr_->AddRef();
  }

  CefRefPtr(const CefRefPtr<T>& r) : ptr_(r.ptr_) {
    if (ptr_)
      ptr_->AddRef();
  }

  ~CefRefPtr() {
    if (ptr_)
      ptr_->Release();
  }

  T* get() const { return ptr_; }
  operator T*() const { return ptr_; }
  T* operator->() const { return ptr_; }

  CefRefPtr<T>& operator=(T* p) {
    // AddRef first so that self assignment should work
    if (p)
      p->AddRef();
    if (ptr_ )
      ptr_ ->Release();
    ptr_ = p;
    return *this;
  }

  CefRefPtr<T>& operator=(const CefRefPtr<T>& r) {
    return *this = r.ptr_;
  }

  void swap(T** pp) {
    T* p = ptr_;
    ptr_ = *pp;
    *pp = p;
  }

  void swap(CefRefPtr<T>& r) {
    swap(&r.ptr_);
  }

 private:
  T* ptr_;
};

#endif // _CEF2_BASE_H
