#pragma once

#include <array>
#include <exception>

class BadWeakPtr : public std::exception {};

template <typename T>
class WeakPtr;

template <typename T>
class SharedPtr;

struct ControlBlockBase {
    int ref_count = 1;
    int weak_ref_count = 0;
    bool is_deleted = false;

    bool IsRefZero() {
        return ((ref_count == 0) && (weak_ref_count == 0));
    }

    void DecrRef() {
        --ref_count;
        if (ref_count == 0 && weak_ref_count == 0) {
            delete this;
        } else if (ref_count == 0) {
            SharedDestructor();
        }
    }

    void IncrRef() {
        ++ref_count;
    }

    void DecrWeakRef(bool flag) {
        if (!flag) {
            --weak_ref_count;
            if (ref_count == 0 && weak_ref_count == 0) {
                delete this;
            } else if (ref_count == 0) {
                SharedDestructor();
            }
        }
    }

    void IncrWeakRef(bool flag) {
        if (!flag) {
            ++weak_ref_count;
        }
    }

    virtual void* GetObjectPtr() = 0;
    virtual ~ControlBlockBase() = default;
    virtual void SharedDestructor() = 0;
};

template <typename T>
struct ControlBlockWithObject : ControlBlockBase {
    template <typename... Args>
    ControlBlockWithObject(Args&&... args) {
        ref_count = 1;
        weak_ref_count = 0;
        is_deleted = false;
        new (&buffer) T(std::forward<Args>(args)...);
    }

    void SharedDestructor() {
        if (ref_count == 0 && !is_deleted) {
            T* ptr = static_cast<T*>(GetObjectPtr());
            ptr->~T();
            is_deleted = true;
        }
    }

    ~ControlBlockWithObject() {
        SharedDestructor();
    }

    void* GetObjectPtr() {
        return reinterpret_cast<T*>(&buffer);
    }

    alignas(T) std::array<char, sizeof(T)> buffer;
};

template <typename T>
struct ControlBlockWithPointer : ControlBlockBase {
    ControlBlockWithPointer(T* obj) : object(obj) {
        ref_count = 1;
        weak_ref_count = 0;
        is_deleted = false;
    }

    void SharedDestructor() {
        if (ref_count == 0 && !is_deleted) {
            delete object;
            is_deleted = true;
        }
    }

    void* GetObjectPtr() {
        return object;
    }

    ~ControlBlockWithPointer() {
        SharedDestructor();
    }

    T* object;
};
