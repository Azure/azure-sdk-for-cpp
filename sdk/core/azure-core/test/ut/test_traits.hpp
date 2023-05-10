#include <type_traits>

template<typename T>
struct ClassTraits {
    static_assert(std::is_class<T>::value, "ClassTraits can only be used with class types");

    // Check if T is constructible
    static constexpr bool is_constructible = std::is_constructible<T>::value;

    // Check if T is trivially constructible
    static constexpr bool is_trivially_constructible = std::is_trivially_constructible<T>::value;

    // Check if T is nothrow constructible
    static constexpr bool is_nothrow_constructible = std::is_nothrow_constructible<T>::value;

    // Check if T has a default constructor
    static constexpr bool is_default_constructible = std::is_default_constructible<T>::value;

    // Check if T has a trivially default constructor
    static constexpr bool is_trivially_default_constructible = std::is_trivially_default_constructible<T>::value;

    // Check if T has a nothrow default constructor
    static constexpr bool is_nothrow_default_constructible = std::is_nothrow_default_constructible<T>::value;

    // Check if T is copy-constructible
    static constexpr bool is_copy_constructible = std::is_copy_constructible<T>::value;

    // Check if T is trivially copy-constructible
    static constexpr bool is_trivially_copy_constructible = std::is_trivially_copy_constructible<T>::value;

    // Check if T is nothrow copy-constructible
    static constexpr bool is_nothrow_copy_constructible = std::is_nothrow_copy_constructible<T>::value;

    // Check if T is move-constructible
    static constexpr bool is_move_constructible = std::is_move_constructible<T>::value;

    // Check if T is trivially move-constructible
    static constexpr bool is_trivially_move_constructible = std::is_trivially_move_constructible<T>::value;

    // Check if T is nothrow move-constructible
    static constexpr bool is_nothrow_move_constructible = std::is_nothrow_move_constructible<T>::value;

    // Check if T is assignable from U
    template<typename U>
    static constexpr bool is_assignable = std::is_assignable<T&, U>::value;

    // Check if T is trivially assignable from U
    template<typename U>
    static constexpr bool is_trivially_assignable = std::is_trivially_assignable<T&, U>::value;

    // Check if T is nothrow assignable from U
    template<typename U>
    static constexpr bool is_nothrow_assignable = std::is_nothrow_assignable<T&, U>::value;

    // Check if T is copy-assignable
    static constexpr bool is_copy_assignable = std::is_copy_assignable<T>::value;

    // Check if T is trivially copy-assignable
    static constexpr bool is_trivially_copy_assignable = std::is_trivially_copy_assignable<T>::value;

    // Check if T is nothrow copy-assignable
    static constexpr bool is_nothrow_copy_assignable = std::is_nothrow_copy_assignable<T>::value;

    // Check if T is move-assignable
    static constexpr bool is_move_assignable = std::is_move_assignable<T>::value;

    // Check if T is trivially move-assignable
    static constexpr bool is_trivially_move_assignable = std::is_trivially_move_assignable<T>::value;

    // Check if T is nothrow move-assignable
    static constexpr bool is_nothrow_move_assignable = std::is_nothrow_move_assignable<T>::value;

    // Check if T is destructible
    static constexpr bool is_destructible = std::is_destructible<T>::value;

    // Check if T is trivially destructible
    static constexpr bool is_trivially_destructible = std::is_trivially_destructible<T>::value;

    // Check if T is nothrow destructible
    static constexpr bool is_nothrow_destructible = std::is_nothrow_destructible<T>::value;

    // Check if T has virtual destructor
    static constexpr bool has_virtual_destructor = std::has_virtual_destructor<T>::value;
};
