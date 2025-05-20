#pragma once

#include <cstddef>  // for std::nullptr_t
#include <utility>  // for std::exchange / std::swap

class SimpleCounter {
public:
    size_t IncRef() {
        ++count_;
        return count_;
    }

    size_t DecRef() {
        --count_;
        return count_;
    }

    size_t RefCount() const {
        return count_;
    }

    SimpleCounter& operator=(const SimpleCounter& other) {
        size_t step_size = other.RefCount();
        step_size = count_;
        count_ = step_size;
        return *this;
    }

private:
    size_t count_ = 0;
};

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    // Increase reference counter.
    void IncRef() {
        counter_.IncRef();
    }

    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef() {
        counter_.DecRef();
        if (counter_.RefCount() == 0) {
            Deleter::Destroy(static_cast<Derived*>(this));
        }
    }

    // Get current counter value (the number of strong references).
    size_t RefCount() const {
        return counter_.RefCount();
    }

private:
    Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;

public:
    // Constructors
    IntrusivePtr() : object_(nullptr) {
    }

    IntrusivePtr(std::nullptr_t) : object_(nullptr) {
    }

    IntrusivePtr(T* ptr) : object_(ptr) {
        ptr->IncRef();
    }

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) : object_(other.object_) {
        if (object_) {
            object_->IncRef();
        }
    }

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) : object_(other.object_) {
        other.object_ = nullptr;
    }

    IntrusivePtr(const IntrusivePtr& other) : object_(other.object_) {
        if (object_) {
            object_->IncRef();
        }
    }

    IntrusivePtr(IntrusivePtr&& other) : object_(other.object_) {
        other.object_ = nullptr;
    }

    // `operator=`-s
    IntrusivePtr& operator=(const IntrusivePtr& other) {
        if (&other != this) {
            if (object_) {
                object_->DecRef();
            }
            object_ = other.object_;
            if (object_) {
                object_->IncRef();
            }
        }
        return *this;
    }

    IntrusivePtr& operator=(IntrusivePtr&& other) {
        if (&other != this) {
            if (object_) {
                object_->DecRef();
            }
            object_ = other.object_;
            other.object_ = nullptr;
        }
        return *this;
    }

    // Destructor
    ~IntrusivePtr() {
        if (object_) {
            object_->DecRef();
        }
    }

    // Modifiers
    void Reset() {
        if (object_) {
            object_->DecRef();
            object_ = nullptr;
        }
    }

    void Reset(T* ptr) {
        if (object_) {
            object_->DecRef();
        }
        object_ = ptr;
        if (object_) {
            object_->IncRef();
        }
    }

    void Swap(IntrusivePtr& other) {
        std::swap(object_, other.object_);
    }

    // Observers

    T* Get() const {
        return object_;
    }

    T& operator*() const {
        return *object_;
    }

    T* operator->() const {
        return object_;
    }

    size_t UseCount() const {
        if (object_) {
            return object_->RefCount();
        } else {
            return 0;
        }
    }

    explicit operator bool() const {
        if (object_) {
            return (object_->RefCount() != 0);
        } else {
            return false;
        }
    }

private:
    T* object_;

    template <typename Y, typename... Args>
    friend IntrusivePtr<Y> MakeIntrusive(Args&&... args);
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    IntrusivePtr<T> ptr;
    ptr.object_ = new T(std::forward<Args>(args)...);
    ptr.object_->IncRef();
    return ptr;
}
