#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() : cb_(nullptr), observed_pole_(nullptr) {
    }

    WeakPtr(const WeakPtr& other) : cb_(other.cb_), observed_pole_(other.observed_pole_) {
        if (cb_) {
            cb_->IncrWeakRef();
        }
    }

    WeakPtr(WeakPtr&& other) : cb_(other.cb_), observed_pole_(other.observed_pole_) {
        other.cb_ = nullptr;
        other.observed_pole_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) : cb_(other.cb_), observed_pole_(other.observed_pole_) {
        if (other.cb_) {
            other.cb_->IncrWeakRef();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (cb_) {
            cb_->DecrWeakRef();
        }
        cb_ = other.cb_;
        observed_pole_ = other.observed_pole_;
        if (cb_) {
            cb_->IncrWeakRef();
        }
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
        if (cb_) {
            cb_->DecrWeakRef();
        }
        cb_ = other.cb_;
        observed_pole_ = other.observed_pole_;
        other.cb_ = nullptr;
        other.observed_pole_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        if (cb_) {
            cb_->DecrWeakRef();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (cb_) {
            cb_->DecrWeakRef();
            cb_ = nullptr;
            observed_pole_ = nullptr;
        }
    }

    void Swap(WeakPtr& other) {
        std::swap(cb_, other.cb_);
        std::swap(observed_pole_, other.observed_pole_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (cb_) {
            return cb_->ref_count;
        }
        return 0;
    }

    bool Expired() const {
        if (cb_ && cb_->ref_count > 0) {
            return (cb_->is_deleted);
        }
        return true;
    }

    template <typename Y>
    friend class SharedPtr;
    SharedPtr<T> Lock() const {
        if (!Expired()) {
            return SharedPtr<T>(*this);
        }
        return SharedPtr<T>();
    }

private:
    ControlBlockBase* cb_;
    T* observed_pole_;
};
