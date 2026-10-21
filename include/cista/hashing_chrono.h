#pragma once

// Kept out of hashing.h: in C++20, <chrono> pulls in <format> and <locale>.

#include <chrono>

#include "cista/hashing.h"

namespace cista {

template <typename Rep, typename Period>
struct hashing<std::chrono::duration<Rep, Period>> {
  hash_t operator()(std::chrono::duration<Rep, Period> const& el,
                    hash_t const seed = BASE_HASH) {
    return hashing<Rep>{}(el.count(), seed);
  }
};

template <typename Clock, typename Duration>
struct hashing<std::chrono::time_point<Clock, Duration>> {
  hash_t operator()(std::chrono::time_point<Clock, Duration> const& el,
                    hash_t const seed = BASE_HASH) {
    return hashing<Duration>{}(el.time_since_epoch(), seed);
  }
};

}  // namespace cista
