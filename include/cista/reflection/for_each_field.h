#pragma once

#include <type_traits>
#include <utility>

#include "cista/reflection/to_tuple.h"

namespace cista {

template <typename T, typename Fn>
void for_each_ptr_field(T& t, Fn&& fn) {
  if constexpr (std::is_pointer_v<T>) {
    if (t != nullptr) {
      for_each_ptr_field(*t, std::forward<Fn>(fn));
    }
  } else if constexpr (std::is_scalar_v<T>) {
    fn(t);
  } else if constexpr (detail::has_cista_member_pointers_v<T>) {
    detail::for_each_member_address(
        t, fn, std::remove_const_t<T>::cista_members());
  } else {
    std::apply([&](auto&&... args) { (fn(args), ...); }, to_ptr_tuple(t));
  }
}

template <typename T, typename Fn>
void for_each_field(T& t, Fn&& fn) {
  if constexpr (std::is_pointer_v<T>) {
    if (t != nullptr) {
      for_each_field(*t, std::forward<Fn>(fn));
    }
  } else if constexpr (std::is_scalar_v<T>) {
    fn(t);
  } else if constexpr (detail::has_cista_member_pointers_v<T>) {
    detail::for_each_member_pointer(
        t, fn, std::remove_const_t<T>::cista_members());
  } else {
    std::apply([&](auto&&... args) { (fn(args), ...); }, to_tuple(t));
  }
}

template <typename T, typename Fn>
void for_each_field(Fn&& fn) {
  T t{};
  for_each_field<T>(t, std::forward<Fn>(fn));
}

}  // namespace cista
