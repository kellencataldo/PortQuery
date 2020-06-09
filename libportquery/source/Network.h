#pragma once

#include <string>
#include <type_traits>

/*
 *
 * Original idea: bad, hard to read and maintain, also... undefined behavior. Fun working with structured bindings though
 

template<unsigned int... bitPositions> struct structuredBindingGen { };

template<int... bitPositions> struct binaryFlag {
    unsigned int m_value;
    static constexpr structuredBindingGen<bitPositions...> values = { };

    bool isSet(const binaryFlag<bitPositions...> other) const {
        return 0 != (m_value & other.m_value);
    }
};

template<unsigned int... bitPositions> struct std::tuple_size<structuredBindingGen<bitPositions...>> : 
    std::integral_constant<std::size_t, sizeof...(bitPositions)> { };

template<std::size_t position, unsigned int ... bitPositions> struct std::tuple_element<position, structuredBindingGen<bitPositions...>> {
    using type = binaryFlag<bitPositions...>; 
};

template<std::size_t position, unsigned int... bitPositions> binaryFlag<bitPositions...> get(structuredBindingGen<bitPositions...>) {
    unsigned int positionsArray[] = { bitPositions... };
    return { static_cast<unsigned int>(1) << positionsArray[position] };
}

template <unsigned int... bitPositions> binaryFlag <bitPositions...> operator|(binaryFlag<bitPositions...> lhs, binaryFlag<bitPositions...> rhs ) { 
    return { lhs.m_value | rhs.m_value }; 
}

using protocolFlags = binaryFlag<0,1>;
namespace Protocols { namespace { const auto [TCP, UDP] = protocolFlags::values; }; };
*/


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
    return static_cast<T>(static_cast<underlying>(lhs) |= static_cast<underlying>(rhs));
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


class Network {

    bool isValidAddress(const std::string& potentialURL);
};

