#pragma once

#include <type_traits>
#include <utility>

template <std::size_t Pos, typename T,
          bool is_EBO_relevant = std::is_empty_v<T> && !std::is_final_v<T>>
class PairElem {};

template <std::size_t Pos, typename T>
class PairElem<Pos, T, true> : T {
public:
    PairElem() = default;

    PairElem(const T& value) : T(value) {
    }

    PairElem(T&& value) : T(std::move(value)) {
    }

    T& GetElem() {
        return *this;
    }

    const T& GetElem() const {
        return *this;
    }
};

template <std::size_t Pos, typename T>
class PairElem<Pos, T, false> {
public:
    PairElem() : value_() {
    }

    PairElem(const T& value) : value_(value) {
    }

    PairElem(T&& value) : value_(std::move(value)) {
    }

    T& GetElem() {
        return value_;
    }

    const T& GetElem() const {
        return value_;
    }

private:
    T value_;
};

template <typename F, typename S>
class CompressedPair : PairElem<1, F>, PairElem<2, S> {
public:
    CompressedPair() = default;

    CompressedPair(const F& first, const S& second)
        : PairElem<1, F>(first), PairElem<2, S>(second) {
    }

    CompressedPair(const F& first, S&& second)
        : PairElem<1, F>(first), PairElem<2, S>(std::move(second)) {
    }

    CompressedPair(F&& first, const S& second)
        : PairElem<1, F>(std::move(first)), PairElem<2, S>(second) {
    }

    CompressedPair(F&& first, S&& second)
        : PairElem<1, F>(std::move(first)), PairElem<2, S>(std::move(second)) {
    }

    F& GetFirst() {
        return PairElem<1, F>::GetElem();
    }

    const F& GetFirst() const {
        return PairElem<1, F>::GetElem();
    }

    S& GetSecond() {
        return PairElem<2, S>::GetElem();
    }

    const S& GetSecond() const {
        return PairElem<2, S>::GetElem();
    }
};

template <typename F, typename S>
class CompressedPair<F*, S> : PairElem<1, F*>, PairElem<2, S> {
public:
    CompressedPair() = default;

    CompressedPair(F* first, S& second) : PairElem<1, F*>(first), PairElem<2, S>(second) {
    }

    CompressedPair(F* first, const S& second) : PairElem<1, F*>(first), PairElem<2, S>(second) {
    }

    CompressedPair(F* first, S&& second)
        : PairElem<1, F*>(first), PairElem<2, S>(std::move(second)) {
    }

    F*& GetFirst() {
        return PairElem<1, F*>::GetElem();
    }

    const F* GetFirst() const {
        return PairElem<1, F*>::GetElem();
    }

    S& GetSecond() {
        return PairElem<2, S>::GetElem();
    }

    const S& GetSecond() const {
        return PairElem<2, S>::GetElem();
    }
};
