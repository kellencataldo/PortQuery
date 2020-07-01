#pragma once

#include <string>
#include <type_traits>
#include <memory>


template<typename T> struct EnableBinaryOperators {
    static const bool m_enable = false;
};

template<typename T> typename std::enable_if<EnableBinaryOperators<T>::m_enable, T>::type operator|(const T lhs, const T rhs) {
    typedef typename std::underlying_type<T>::type underlying;
    return static_cast<T>(static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
}

template<typename T> typename std::enable_if<EnableBinaryOperators<T>::m_enable, T>::type operator&(const T lhs, const T rhs) {
    typedef typename std::underlying_type<T>::type underlying;
    return static_cast<T>(static_cast<underlying>(lhs) & static_cast<underlying>(rhs));
}

template<typename T> typename std::enable_if<EnableBinaryOperators<T>::m_enable, T>::type operator|=(T& lhs, const T rhs) {
    typedef typename std::underlying_type<T>::type underlying;
    lhs = static_cast<T>(static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
    return lhs;
}

// other operators go here
enum class NetworkProtocols {
    NONE = 0,
    TCP  = 1 << 0,
    UDP  = 1 << 1,

    // other protocols go here
};

template <> struct EnableBinaryOperators<NetworkProtocols> {
    static const bool m_enable = true;
};

