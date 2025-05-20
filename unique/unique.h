#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t
#include <type_traits>

template <typename T>
struct Slug {
    Slug() = default;

    template <typename U = T>
        requires(std::is_convertible_v<U*, T*>)
    Slug(const Slug<U>&) {
    }

    template <typename U = T>
        requires(std::is_convertible_v<U*, T*>)
    Slug(Slug<U>&&) noexcept {
    }

    void operator()(T* ptr) {
        if (ptr) {
            delete ptr;
        }
    }
};

template <typename T>
struct Slug<T[]> {
    Slug() = default;

    template <typename U = T>
        requires(std::is_convertible_v<U*, T*>)
    Slug(const Slug<U>&) {
    }

    template <typename U = T>
        requires(std::is_convertible_v<U*, T*>)
    Slug(Slug<U>&&) noexcept {
    }

    void operator()(T* ptr) {
        if (ptr) {
            delete[] ptr;
        }
    }
};

// Primary template
template <typename T, typename Deleter = Slug<T>>
class UniquePtr {
public:
    explicit UniquePtr(T* ptr = nullptr) : data_(ptr, Deleter()) {
    }

    UniquePtr(T* ptr, Deleter deleter) : data_(ptr, std::move(deleter)) {
    }

    UniquePtr(UniquePtr&& other) noexcept {
        std::swap(data_, other.data_);
    }

    template <typename U = T, typename OtherDeleter = Deleter>
        requires(std::is_convertible_v<U*, T*> && std::is_convertible_v<OtherDeleter, Deleter>)
    UniquePtr(UniquePtr<U, OtherDeleter>&& other) noexcept
        : data_(other.Release(), std::move(other.GetDeleter())) {
    }

    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            Reset();
            Swap(other);
        }
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) {
        data_.GetSecond()(data_.GetFirst());
        data_.GetFirst() = nullptr;
        return *this;
    }

    ~UniquePtr() {
        data_.GetSecond()(data_.GetFirst());
    }

    T* Release() {
        T* ptr = data_.GetFirst();
        data_.GetFirst() = nullptr;
        return ptr;
    }

    void Reset(T* ptr = nullptr) {
        T* old_ptr = data_.GetFirst();
        data_.GetFirst() = ptr;
        if (old_ptr) {
            data_.GetSecond()(old_ptr);
        }
    }

    void Swap(UniquePtr& other) {
        std::swap(other.data_.GetFirst(), data_.GetFirst());
        std::swap(other.data_.GetSecond(), data_.GetSecond());
    }

    T* Get() {
        return data_.GetFirst();
    }

    const T* Get() const {
        return data_.GetFirst();
    }

    Deleter& GetDeleter() {
        return data_.GetSecond();
    }

    const Deleter& GetDeleter() const {
        return data_.GetSecond();
    }

    explicit operator bool() const {
        return (data_.GetFirst() != nullptr);
    }

    template <typename S = T>
        requires(!std::is_void_v<S>)
    const S& operator*() const {
        return *data_.GetFirst();
    }

    const T* operator->() const {
        return data_.GetFirst();
    }

    T* operator->() {
        return data_.GetFirst();
    }

private:
    CompressedPair<T*, Deleter> data_;
};

template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    explicit UniquePtr(T* ptr = nullptr) : data_(ptr, Deleter()) {
    }

    UniquePtr(T* ptr, Deleter deleter) : data_(ptr, std::move(deleter)) {
    }

    UniquePtr(UniquePtr&& other) noexcept {
        std::swap(data_, other.data_);
    }

    template <typename U = T, typename OtherDeleter = Deleter>
        requires(std::is_convertible_v<U*, T*> && std::is_convertible_v<OtherDeleter, Deleter>)
    UniquePtr(UniquePtr<U, OtherDeleter>&& other) noexcept
        : data_(other.Release(), std::move(other.GetDeleter())) {
    }

    template <typename U, typename OtherDeleter>
        requires(std::is_convertible_v<U*, T*> && std::is_convertible_v<OtherDeleter, Deleter>)
    UniquePtr& operator=(UniquePtr<U, OtherDeleter>&& other) noexcept {
        if (this != &other) {
            Reset();
            data_ = std::move(other.data_);  // Используем перемещение
        }
        return *this;
    }

    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            Reset();
            std::swap(data_, other.data_);
        }
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) {
        data_.GetSecond()(data_.GetFirst());
        data_.GetFirst() = nullptr;
        return *this;
    }

    ~UniquePtr() {
        data_.GetSecond()(data_.GetFirst());
    }

    T* Release() {
        T* ptr = data_.GetFirst();
        data_.GetFirst() = nullptr;
        return ptr;
    }

    void Reset(T* ptr = nullptr) {
        T* old_ptr = data_.GetFirst();
        data_.GetFirst() = ptr;
        if (old_ptr) {
            data_.GetSecond()(old_ptr);
        }
    }

    void Swap(UniquePtr& other) {
        std::swap(other.data_.GetFirst(), data_.GetFirst());
        std::swap(other.data_.GetSecond(), data_.GetSecond());
    }

    T* Get() {
        return data_.GetFirst();
    }

    const T* Get() const {
        return data_.GetFirst();
    }

    Deleter& GetDeleter() {
        return data_.GetSecond();
    }

    const Deleter& GetDeleter() const {
        return data_.GetSecond();
    }

    explicit operator bool() const {
        return (data_.GetFirst() != nullptr);
    }

    template <typename S = T>
        requires(!std::is_void_v<S>)
    const S& operator*() const {
        return *data_.GetFirst();
    }

    const T* operator->() const {
        return data_.GetFirst();
    }

    T* operator->() {
        return data_.GetFirst();
    }

    const T& operator[](size_t ind) const {
        return *(data_.GetFirst() + ind);
    }

    T& operator[](size_t ind) {
        return *(data_.GetFirst() + ind);
    }

private:
    CompressedPair<T*, Deleter> data_;
};
