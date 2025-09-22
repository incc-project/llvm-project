//===--- Memory.h - ILLVM memory utils ------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// ILLVM memory utils.
//
//===----------------------------------------------------------------------===/

#ifndef ILLVM_MEMORY_H
#define ILLVM_MEMORY_H

#include <memory>

namespace illvm {

// Nonnull
template <typename T>
class OwnerPtr {
private:
  bool initialized = false;
  std::unique_ptr<T> ptr = nullptr;

public:
  OwnerPtr() = default;

  ~OwnerPtr() {
    assert(initialized);
  }

  void init(std::unique_ptr<T>&& p) {
    assert(p != nullptr);
    initialized = true;
    ptr = std::move(p);
  }

  OwnerPtr(OwnerPtr&&) noexcept = default;
  OwnerPtr& operator=(OwnerPtr&&) noexcept = default;

  OwnerPtr(const OwnerPtr&) = delete;
  OwnerPtr& operator=(const OwnerPtr&) = delete;

  T* get() const noexcept { assert(initialized); return ptr.get(); }
  T& operator*() const noexcept { assert(initialized); return *ptr; }
  T* operator->() const noexcept { assert(initialized); return ptr.get(); }
};

// Nonnull
template <typename T>
class BorrowedPtr {
private:
  bool initialized = false;
  T* ptr = nullptr;

public:
  BorrowedPtr() = default;

  ~BorrowedPtr() {
    assert(initialized);
  }

  void init(T* p) {
    assert(p != nullptr);
    initialized = true;
    ptr = p;
  }

  T* get() const noexcept { assert(initialized); return ptr; }
  T& operator*() const noexcept { assert(initialized); return *ptr; }
  T* operator->() const noexcept { assert(initialized); return ptr; }
};

template <typename T>
using OPtr = OwnerPtr<T>;

template <typename T>
using BPtr = BorrowedPtr<T>;

}

#endif // ILLVM_MEMORY_H
