//
// Created by root on 31.07.18.
//

#ifndef REVERSE_ENGINE_FIX_ENUM_HH
#define REVERSE_ENGINE_FIX_ENUM_HH

#include <type_traits>

/**
 * Description: Enum class now wokrs like enum. Defined for: ~, &, |, ^, &=, |=, ^=.
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


#endif //REVERSE_ENGINE_FIX_ENUM_HH
