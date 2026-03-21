#pragma once

#include <optional>
#include <type_traits>
#include <utility>

#include "cista/exception.h"

namespace cista {

template <typename T>
struct optional {
  using value_type = T;

  optional() = default;
  optional(std::nullopt_t) noexcept {}

  template <typename... Ts>
  optional(Ts&&... t) noexcept(std::is_nothrow_constructible_v<T, Ts...>) {
    new (&storage_[0]) T(std::forward<Ts>(t)...);
    valid_ = true;
  }

  optional(optional const& other) { copy(other); }

  optional& operator=(optional const& other) {
    if (this != &other) {
      reset();
      copy(other);
    }
    return *this;
  }

  optional(optional&& other) noexcept { move(std::move(other)); }

  optional& operator=(optional&& other) noexcept {
    if (this != &other) {
      reset();
      move(std::move(other));
    }
    return *this;
  }

  optional& operator=(std::nullopt_t) noexcept {
    reset();
    return *this;
  }

  ~optional() { reset(); }

  void reset() noexcept {
    if (valid_) {
      reinterpret_cast<T*>(&storage_[0])->~T();
      valid_ = false;
    }
  }

  template <typename... Ts>
  T& emplace(Ts&&... ts) {
    reset();
    new (&storage_[0]) T(std::forward<Ts>(ts)...);
    valid_ = true;
    return value();
  }

  T const& value() const {
    if (!valid_) {
      throw_exception(std::bad_optional_access{});
    }
    return *reinterpret_cast<T const*>(&storage_[0]);
  }

  T& value() {
    if (!valid_) {
      throw_exception(std::bad_optional_access{});
    }
    return *reinterpret_cast<T*>(&storage_[0]);
  }

  bool has_value() const noexcept { return valid_; }
  explicit operator bool() const noexcept { return valid_; }

  T* operator->() noexcept { return reinterpret_cast<T*>(&storage_[0]); }
  T const* operator->() const noexcept {
    return reinterpret_cast<T const*>(&storage_[0]);
  }

  T& operator*() noexcept { return *reinterpret_cast<T*>(&storage_[0]); }
  T const& operator*() const noexcept {
    return *reinterpret_cast<T const*>(&storage_[0]);
  }

 private:
  void copy(optional const& other) {
    if (other.has_value()) {
      new (&storage_[0]) T(other.value());
      valid_ = true;
    }
  }

  void move(optional&& other) {
    if (other.has_value()) {
      new (&storage_[0]) T(std::move(other.value()));
      valid_ = true;
    }
  }

 public:
  bool valid_{false};
  alignas(T) unsigned char storage_[sizeof(T)];
};

}  // namespace cista