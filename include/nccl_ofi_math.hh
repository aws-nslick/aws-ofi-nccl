/*
 * Copyright (c) 2023 Amazon.com, Inc. or its affiliates. All rights reserved.
 */

#pragma once

#include <cassert>
#include <cstdint>
#include <type_traits>

#include <cmath>
#include <cstdint>
#include <type_traits>

#if defined(__cpp_lib_bitops) || (defined(__has_include) && __has_include(<bit>))
#include <bit>
#endif

#ifdef __cpp_lib_int_pow2
#include <numeric>
#endif

namespace aon::detail::math {

/*
 * @brief Returns the ceil of x/y.
 */
template <typename T> constexpr T div_ceil(T x, T y) {
  static_assert(std::is_integral<T>::value, "Type must be integral");
  return (x == 0) ? 0 : (x + y - 1) / y;
}

/*
 * @brief Returns true if and only if value is a power of two
 */
template <typename T> constexpr bool is_power_of_two(T x) {
  static_assert(std::is_integral<T>::value, "Type must be integral");
#ifdef __cpp_lib_int_pow2
  return std::has_single_bit(x);
#else
  return x && !(x & (x - 1));
#endif
}

/*
 * @brief Return true if and only if `x` is a multiple of `a`
 *
 * @param a Must be a power of two
 */
template <typename T> constexpr bool is_aligned(T x, T a) {
  static_assert(std::is_integral<T>::value, "Type must be integral");
  return (x & (a - 1)) == 0;
}

/*
 * @brief Return true if and only if pointer `p` is `a`-byte aligned
 *
 * @param a Must be a power of two
 */
template <typename T> constexpr bool is_ptr_aligned(T *p, std::uintptr_t a) {
  static_assert(std::is_pointer<T *>::value, "T must be a pointer type");
  static_assert(!std::is_function<typename std::remove_pointer<T>::type>::value, "Function pointers are not supported");
  return is_aligned(reinterpret_cast<std::uintptr_t>(p), a);
}

// Overload for const pointers
template <typename T> constexpr bool is_ptr_aligned(const T *p, std::uintptr_t a) {
  static_assert(std::is_pointer<T *>::value, "T must be a pointer type");
  static_assert(!std::is_function<typename std::remove_pointer<T>::type>::value, "Function pointers are not supported");
  return is_aligned(reinterpret_cast<std::uintptr_t>(p), a);
}

// Overload for void pointers
// clang-format off
#ifdef __cpp_lib_bit_cast
constexpr
#else
static inline
#endif
bool is_ptr_aligned(void *p, std::uintptr_t a) {
#ifdef __cpp_lib_bit_cast
  return is_aligned(std::bit_cast<std::uintptr_t>(p), a);
#else
  return is_aligned(reinterpret_cast<std::uintptr_t>(p), a);
#endif
}

// Overload for const void pointers
#ifdef __cpp_lib_bit_cast
constexpr
#else
static inline
#endif
bool is_ptr_aligned(const void *p, std::uintptr_t a) {
#ifdef __cpp_lib_bit_cast
  return is_aligned(std::bit_cast<std::uintptr_t>(p), a);
#else
  return is_aligned(reinterpret_cast<std::uintptr_t>(p), a);
#endif
}
// clang-format on

/*
 * @brief Round value down to be a multiple of alignment
 *
 * @param y Must be a power of two
 */
/*
 * @brief Round value down to be a multiple of alignment
 *
 * @param y Must be a power of two
 */
template <typename T> constexpr T round_down(T x, T y) {
  static_assert(std::is_integral<T>::value, "Type must be integral");
  return x & ~(y - 1);
}

/*
 * @brief Round value up to be a multiple of alignment
 *
 * @param y Must be a power of two
 */
template <typename T> constexpr T round_up(T x, T y) {
  static_assert(std::is_integral<T>::value, "Type must be integral");
#ifdef __cpp_lib_bitops
  return std::bit_ceil(x);
#else
  return round_down(x + y - 1, y);
#endif
}

} // namespace aon::detail::math
