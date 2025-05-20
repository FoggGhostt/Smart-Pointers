#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <utility>

struct ControlBlockBase {
    int ref_count = 1;

    void DecrRef() {
        --ref_count;
    }

    void IncrRef() {
        ++ref_count;
    }
    virtual ~ControlBlockBase() = default;
};

template <typename T>
struct ControlBlockWithObject : ControlBlockBase {
    template <typename... Args>
    ControlBlockWithObject(Args&&... args) : object(std::forward<Args>(args)...) {
        ref_count = 1;
    }

    ~ControlBlockWithObject() = default;

    T* GetObjectPtr() {
        return &object;
    }

    T object;
};

template <typename T>
struct ControlBlockWithPointer : ControlBlockBase {
    ControlBlockWithPointer(T* obj) : object(obj) {
        ref_count = 1;
    }

    ~ControlBlockWithPointer() {
        delete object;
    }
    T* object;
};

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() : cb_(nullptr), observed_pole_(nullptr) {
    }

    SharedPtr(std::nullptr_t) : cb_(nullptr), observed_pole_(nullptr) {
    }

    explicit SharedPtr(T* ptr) : cb_(new ControlBlockWithPointer<T>(ptr)), observed_pole_(ptr) {
    }

    template <typename Y>
    explicit SharedPtr(Y* ptr)
        : cb_(new ControlBlockWithPointer<Y>(ptr)), observed_pole_(static_cast<T*>(ptr)) {
    }

    SharedPtr(const SharedPtr& other) : cb_(other.cb_), observed_pole_(other.observed_pole_) {
        if (cb_) {
            cb_->IncrRef();
        }
    }

    SharedPtr(SharedPtr&& other) : cb_(other.cb_), observed_pole_(other.observed_pole_) {
        // if (other.cb_) {
        //     other.cb_->DecrRef();
        // }
        other.cb_ = nullptr;
        other.observed_pole_ = nullptr;
    }

    template <typename U>
    SharedPtr(const SharedPtr<U>& other) : cb_(other.cb_), observed_pole_(other.observed_pole_) {
        if (cb_) {
            cb_->IncrRef();
        }
    }

    template <typename U>
    SharedPtr(SharedPtr<U>&& other) : cb_(other.cb_), observed_pole_(other.observed_pole_) {
        other.cb_ = nullptr;
        other.observed_pole_ = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    friend class SharedPtr;  // Позволяет использовать SharedPtr<Y> внутри SharedPtr<T>

    // Aliasing constructor

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) : cb_(other.cb_), observed_pole_(ptr) {
        other.cb_->IncrRef();
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other) {
            if (cb_) {
                cb_->DecrRef();
                if (cb_->ref_count == 0) {
                    delete cb_;
                }
            }
            cb_ = other.cb_;
            observed_pole_ = other.observed_pole_;
            if (cb_) {
                cb_->IncrRef();
            }
        }
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        if (this != &other) {
            if (cb_) {
                cb_->DecrRef();
                if (cb_->ref_count == 0) {
                    delete cb_;
                }
            }
            cb_ = other.cb_;
            observed_pole_ = other.observed_pole_;
            other.cb_ = nullptr;
            other.observed_pole_ = nullptr;
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        if (cb_) {
            cb_->DecrRef();
            if (cb_->ref_count == 0) {
                delete cb_;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (cb_) {
            cb_->DecrRef();
            delete cb_;
        }
        cb_ = nullptr;
        observed_pole_ = nullptr;
    }

    void Reset(T* ptr) {
        if (cb_) {
            cb_->DecrRef();
            if (cb_->ref_count == 0) {
                delete cb_;
            }
        }
        cb_ = new ControlBlockWithPointer(ptr);
        observed_pole_ = ptr;
    }

    template <typename Y>
    void Reset(Y* ptr) {
        if (cb_) {
            cb_->DecrRef();
            if (cb_->ref_count == 0) {
                delete cb_;
            }
        }
        cb_ = new ControlBlockWithPointer<Y>(ptr);
        observed_pole_ = ptr;
    }

    void Swap(SharedPtr& other) {
        std::swap(observed_pole_, other.observed_pole_);
        std::swap(cb_, other.cb_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return observed_pole_;
    }

    T& operator*() const {
        return *observed_pole_;
    }

    T* operator->() const {
        return observed_pole_;
    }

    size_t UseCount() const {
        if (cb_) {
            return cb_->ref_count;
        } else {
            return 0;
        }
    }

    explicit operator bool() const {
        return (cb_ != nullptr);
    }

private:
    ControlBlockBase* cb_ = nullptr;
    T* observed_pole_ = nullptr;

    template <typename Y, typename... Args>
    friend SharedPtr<Y> MakeShared(Args&&... args);

    template <typename Y, typename... Args>
    friend SharedPtr<Y> MakeShared(Args&&... args);
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right);

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    SharedPtr<T> ptr;
    ptr.cb_ = new ControlBlockWithObject<T>(std::forward<Args>(args)...);
    auto step_obj = static_cast<ControlBlockWithObject<T>*>(ptr.cb_);
    ptr.observed_pole_ = step_obj->GetObjectPtr();
    return ptr;
}

template <typename T, typename Y, typename... Args>
SharedPtr<Y> MakeShared(Args&&... args) {
    SharedPtr<Y> ptr;
    ptr.cb_ = new ControlBlockWithObject<Y>(std::forward<Args>(args)...);
    auto step_obj = static_cast<ControlBlockWithObject<T>*>(ptr.cb_);
    ptr.observed_pole_ = step_obj->GetObjectPtr();
    return ptr;
}

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};
