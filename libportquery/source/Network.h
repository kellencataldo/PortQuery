#pragma once

#include <string>
#include <tuple>


template<unsigned int... bitPositions> struct structuredBindingGen { };

template<int... bitPositions> struct binaryFlag {
    unsigned int value;
    static constexpr structuredBindingGen<bitPositions...> values = { };
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
    return { lhs.value | rhs.value }; 
}


namespace Network {

    using protocolBitPositions = binaryFlag<0,1>;
    namespace protocols { auto [TCP, UDP] = protocolBitPositions::values; };

    bool isValidAddress(const std::string& potentialURL);
    
}

