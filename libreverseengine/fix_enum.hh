//
// Created by root on 31.07.18.
//

#ifndef REVERSE_ENGINE_FIX_ENUM_HH
#define REVERSE_ENGINE_FIX_ENUM_HH

#include <type_traits>

/**
 * Description: Enum class now works like enum. Defined for: ~, &, |, ^, &=, |=, ^=.
 * Performance: performance not affected if used -O3 flag, but a bit slower if -O0 used (other flags unchecked).
 */

template <typename E, typename U = typename std::underlying_type<E>::type > \
inline E operator ~(E lhs) { return static_cast<E>(~static_cast<U>(lhs)); };

//https://softwareengineering.stackexchange.com/questions/194412/using-scoped-enums-for-bit-flags-in-c
//https://paste.ubuntu.com/23884820/
//see also https://softwareengineering.stackexchange.com/revisions/205567/1
#define LIFT_ENUM_OP(op, assignop) \
    template <typename E, typename U = typename std::underlying_type<E>::type > \
    static inline E operator op(E lhs, E rhs) { return static_cast<E>(static_cast<U>(lhs) op static_cast<U>(rhs)); } \
    template <typename E, typename U = typename std::underlying_type<E>::type > \
    static inline E& operator assignop(E& lhs, E rhs) { lhs = lhs op rhs; return lhs; }

LIFT_ENUM_OP(&,&=)
LIFT_ENUM_OP(|,|=)
LIFT_ENUM_OP(^,^=)

#undef LIFT_ENUM_OP


constexpr std::size_t operator""_KiB(unsigned long long v) {
    return v<<10;
}
constexpr std::size_t operator""_MiB(unsigned long long v) {
    return v<<20;
}
constexpr std::size_t operator""_GiB(unsigned long long v) {
    return v<<30;
}

#ifndef FORCE_INLINE
# if __MSVC__ // _MSC_VER
#  define FORCE_INLINE __forceinline
# elif __INTEL_COMPILER
#  define FORCE_INLINE inline __attribute__((always_inline))
//#  define FORCE_INLINE _Pragma("forceinline")
# elif __GNUC__
#  define FORCE_INLINE inline __attribute__((always_inline))
# else
#  define FORCE_INLINE inline
#  warning "Check your compiler"
# endif
#endif

#ifndef NEVER_INLINE
# if __MSVC__ // _MSC_VER
#  warning "Not implemented"
# elif __INTEL_COMPILER
#  define NEVER_INLINE __attribute__((noinline))
//#  define NEVER_INLINE _Pragma("noinline")
# elif __GNUC__
#  define NEVER_INLINE __attribute__((noinline))
# else
#  define NEVER_INLINE
#  warning "Check your compiler"
# endif
#endif


#endif //REVERSE_ENGINE_FIX_ENUM_HH
