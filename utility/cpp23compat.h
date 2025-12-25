// -*- C++ -*-

/*

  Heap Layers: An Extensible Memory Allocation Infrastructure

  Copyright (C) 2000-2024 by Emery Berger
  http://www.emeryberger.com
  emery@cs.umass.edu

  Heap Layers is distributed under the terms of the Apache 2.0 license.

  You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0

*/

/**
 * @file cpp23compat.h
 * @brief C++20/23 compatibility macros with fallbacks for C++17.
 *
 * This header provides portable access to modern C++ features:
 * - [[likely]] / [[unlikely]] branch hints
 * - [[assume(expr)]] optimizer hints
 * - std::unreachable() for unreachable code paths
 * - std::bit_cast for type-safe reinterpretation
 * - std::start_lifetime_as for implicit object creation
 * - [[no_unique_address]] for empty base optimization
 */

#ifndef HL_CPP23COMPAT_H
#define HL_CPP23COMPAT_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <new>

// Include <bit> if available for std::bit_cast
#if defined(__cpp_lib_bit_cast) && __cpp_lib_bit_cast >= 201806L
  #include <bit>
#endif

// Include <memory> if start_lifetime_as is available
#if defined(__cpp_lib_start_lifetime_as) && __cpp_lib_start_lifetime_as >= 202207L
  #include <memory>
#endif

//============================================================================
// C++ Standard Version Constants
//============================================================================

#define HL_CPP17 201703L
#define HL_CPP20 202002L
#define HL_CPP23 202302L

//============================================================================
// [[likely]] / [[unlikely]] (C++20)
//============================================================================

#if __cplusplus >= HL_CPP20
  #define HL_LIKELY   [[likely]]
  #define HL_UNLIKELY [[unlikely]]
#else
  #define HL_LIKELY
  #define HL_UNLIKELY
#endif

// Branch prediction hints for use in conditions
#if defined(__GNUC__) || defined(__clang__)
  #define HL_EXPECT_TRUE(x)  __builtin_expect(!!(x), 1)
  #define HL_EXPECT_FALSE(x) __builtin_expect(!!(x), 0)
#else
  #define HL_EXPECT_TRUE(x)  (x)
  #define HL_EXPECT_FALSE(x) (x)
#endif

//============================================================================
// [[assume(expr)]] (C++23)
//============================================================================

#if __cplusplus >= HL_CPP23 && defined(__cpp_lib_assume)
  #define HL_ASSUME(expr) [[assume(expr)]]
#elif defined(__clang__) && __clang_major__ >= 6
  #define HL_ASSUME(expr) __builtin_assume(expr)
#elif defined(__GNUC__) && __GNUC__ >= 13
  #define HL_ASSUME(expr) __attribute__((assume(expr)))
#elif defined(_MSC_VER)
  #define HL_ASSUME(expr) __assume(expr)
#else
  // No-op fallback - optimizer gets no hint
  #define HL_ASSUME(expr) ((void)0)
#endif

//============================================================================
// std::unreachable() (C++23)
//============================================================================

#if __cplusplus >= HL_CPP23 && defined(__cpp_lib_unreachable)
  #include <utility>
  #define HL_UNREACHABLE() std::unreachable()
#elif defined(__GNUC__) || defined(__clang__)
  #define HL_UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
  #define HL_UNREACHABLE() __assume(0)
#else
  #define HL_UNREACHABLE() ((void)0)
#endif

//============================================================================
// [[no_unique_address]] (C++20)
//============================================================================

#if __cplusplus >= HL_CPP20
  #define HL_NO_UNIQUE_ADDRESS [[no_unique_address]]
#elif defined(_MSC_VER) && _MSC_VER >= 1929
  #define HL_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
  #define HL_NO_UNIQUE_ADDRESS
#endif

//============================================================================
// Conditional constexpr by standard version
//============================================================================

#if __cplusplus >= HL_CPP17
  #define HL_CONSTEXPR_17 constexpr
  #define HL_IF_CONSTEXPR if constexpr
#else
  #define HL_CONSTEXPR_17 inline
  #define HL_IF_CONSTEXPR if
#endif

#if __cplusplus >= HL_CPP20
  #define HL_CONSTEXPR_20 constexpr
#else
  #define HL_CONSTEXPR_20 inline
#endif

#if __cplusplus >= HL_CPP23
  #define HL_CONSTEXPR_23 constexpr
#else
  #define HL_CONSTEXPR_23 inline
#endif

//============================================================================
// std::bit_cast (C++20)
//============================================================================

namespace HL {

#if defined(__cpp_lib_bit_cast) && __cpp_lib_bit_cast >= 201806L

  template<typename To, typename From>
  constexpr To bit_cast(const From& src) noexcept {
    return std::bit_cast<To>(src);
  }

#else

  template<typename To, typename From>
  inline To bit_cast(const From& src) noexcept {
    static_assert(sizeof(To) == sizeof(From),
                  "bit_cast requires types of equal size");
    static_assert(std::is_trivially_copyable<To>::value,
                  "bit_cast destination must be trivially copyable");
    static_assert(std::is_trivially_copyable<From>::value,
                  "bit_cast source must be trivially copyable");
    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
  }

#endif

//============================================================================
// std::start_lifetime_as (C++23)
//============================================================================

#if defined(__cpp_lib_start_lifetime_as) && __cpp_lib_start_lifetime_as >= 202207L

  template<typename T>
  T* start_lifetime_as(void* p) noexcept {
    return std::start_lifetime_as<T>(p);
  }

  template<typename T>
  const T* start_lifetime_as(const void* p) noexcept {
    return std::start_lifetime_as<const T>(p);
  }

#elif defined(__cpp_lib_launder) && __cpp_lib_launder >= 201606L
  // Fallback using std::launder (C++17)

  template<typename T>
  T* start_lifetime_as(void* p) noexcept {
    return std::launder(reinterpret_cast<T*>(p));
  }

  template<typename T>
  const T* start_lifetime_as(const void* p) noexcept {
    return std::launder(reinterpret_cast<const T*>(p));
  }

#else
  // Pre-C++17 fallback using reinterpret_cast directly
  // Less safe but the only option without std::launder

  template<typename T>
  T* start_lifetime_as(void* p) noexcept {
    return reinterpret_cast<T*>(p);
  }

  template<typename T>
  const T* start_lifetime_as(const void* p) noexcept {
    return reinterpret_cast<const T*>(p);
  }

#endif

//============================================================================
// Memory ordering aliases for readability
//============================================================================

} // namespace HL

#endif // HL_CPP23COMPAT_H
