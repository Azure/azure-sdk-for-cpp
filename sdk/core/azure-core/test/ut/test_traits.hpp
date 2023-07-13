// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <type_traits>

template <typename T, typename... Args> struct ClassTraits
{
  static_assert(std::is_class<T>::value, "ClassTraits can only be used with class types");

  // Check if T is constructible from Args
  static constexpr bool is_constructible() { return std::is_constructible<T, Args...>::value; }

  // Check if T is trivially constructible from Args
  static constexpr bool is_trivially_constructible()
  {
    return std::is_trivially_constructible<T, Args...>::value;
  }

  // Check if T is nothrow constructible from Args
  static constexpr bool is_nothrow_constructible()
  {
    return std::is_nothrow_constructible<T, Args...>::value;
  }

  // Check if T has a default constructor
  static constexpr bool is_default_constructible()
  {
    return std::is_default_constructible<T>::value;
  }

  // Check if T has a trivially default constructor
  static constexpr bool is_trivially_default_constructible()
  {
    return std::is_trivially_default_constructible<T>::value;
  }

  // Check if T has a nothrow default constructor
  static constexpr bool is_nothrow_default_constructible()
  {
    return std::is_nothrow_default_constructible<T>::value;
  }

  // Check if T is copy-constructible
  static constexpr bool is_copy_constructible() { return std::is_copy_constructible<T>::value; }

  // Check if T is trivially copy-constructible
  static constexpr bool is_trivially_copy_constructible()
  {
    return std::is_trivially_copy_constructible<T>::value;
  }

  // Check if T is nothrow copy-constructible
  static constexpr bool is_nothrow_copy_constructible()
  {
    return std::is_nothrow_copy_constructible<T>::value;
  }

  // Check if T is move-constructible
  static constexpr bool is_move_constructible() { return std::is_move_constructible<T>::value; }

  // Check if T is trivially move-constructible
  static constexpr bool is_trivially_move_constructible()
  {
    return std::is_trivially_move_constructible<T>::value;
  }

  // Check if T is nothrow move-constructible
  static constexpr bool is_nothrow_move_constructible()
  {
    return std::is_nothrow_move_constructible<T>::value;
  }

  // Check if T is assignable from U
  template <typename U> static constexpr bool is_assignable()
  {
    return std::is_assignable<T&, U>::value;
  }

  // Check if T is trivially assignable from U
  template <typename U> static constexpr bool is_trivially_assignable()
  {
    return std::is_trivially_assignable<T&, U>::value;
  }

  // Check if T is nothrow assignable from U
  template <typename U> static constexpr bool is_nothrow_assignable()
  {
    return std::is_nothrow_assignable<T&, U>::value;
  }

  // Check if T is copy-assignable
  static constexpr bool is_copy_assignable() { return std::is_copy_assignable<T>::value; }

  // Check if T is trivially copy-assignable
  static constexpr bool is_trivially_copy_assignable()
  {
    return std::is_trivially_copy_assignable<T>::value;
  }

  // Check if T is nothrow copy-assignable
  static constexpr bool is_nothrow_copy_assignable()
  {
    return std::is_nothrow_copy_assignable<T>::value;
  }

  // Check if T is move-assignable
  static constexpr bool is_move_assignable() { return std::is_move_assignable<T>::value; }

  // Check if T is trivially move-assignable
  static constexpr bool is_trivially_move_assignable()
  {
    return std::is_trivially_move_assignable<T>::value;
  }

  // Check if T is nothrow move-assignable
  static constexpr bool is_nothrow_move_assignable()
  {
    return std::is_nothrow_move_assignable<T>::value;
  }

  // Check if T is destructible
  static constexpr bool is_destructible() { return std::is_destructible<T>::value; }

  // Check if T is trivially destructible
  static constexpr bool is_trivially_destructible()
  {
    return std::is_trivially_destructible<T>::value;
  }

  // Check if T is nothrow destructible
  static constexpr bool is_nothrow_destructible() { return std::is_nothrow_destructible<T>::value; }

  // Check if T has virtual destructor
  static constexpr bool has_virtual_destructor() { return std::has_virtual_destructor<T>::value; }
};
