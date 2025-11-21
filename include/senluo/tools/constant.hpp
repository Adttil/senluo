#ifndef SENLUO_CONSTANT_H
#define SENLUO_CONSTANT_H

#include <type_traits>
#include "macro_define.hpp"

namespace senluo
{
    template<auto value>
    struct constant_t : std::integral_constant<decltype(value), value> 
	{
		friend constexpr bool operator==(const constant_t&, const constant_t&) = default;

		constexpr operator decltype(value)()const
		{
			return value;
		}

		template<class T> requires std::convertible_to<decltype(value), T>
		constexpr explicit operator T()const
		{
			return static_cast<T>(value);
		}
	};

	template<auto Value>
	inline constexpr constant_t<Value> constant{};

    template<auto value1, auto value2>
    constexpr auto operator+(constant_t<value1>, constant_t<value2>) 
	{
	    return constant_t<value1 + value2>{};
	}

    template<auto value1, auto value2>
    constexpr auto operator-(constant_t<value1>, constant_t<value2>)
	{
	    return constant_t<value1 - value2>{};
	}

    template<auto value1, auto value2>
    constexpr auto operator*(constant_t<value1>, constant_t<value2>)
	{
	    return constant_t<value1 * value2>{};
	}

    template<auto value1, auto value2>
    constexpr auto operator/(constant_t<value1>, constant_t<value2>)
	{
	    return constant_t<value1 / value2>{};
	}

    template<auto value1, auto value2>
    constexpr auto operator%(constant_t<value1>, constant_t<value2>)
	{
	    return constant_t<value1 % value2>{};
	}
}

namespace senluo::constants
{
	inline constexpr constant_t<0> _0;
	inline constexpr constant_t<1> _1;
}

// +0
namespace senluo
{
	template<auto Zero, typename T> requires requires{ requires (Zero == 0); }
    constexpr auto operator+(constant_t<Zero>, T&& value)
	{
	    return FWD(value);
	}

    template<auto Zero, typename T> requires requires{ requires (Zero == 0); }
    constexpr auto operator+(T&& value, constant_t<Zero>)
	{
	    return FWD(value);
	}

    template<auto Zero1, auto Zero2> requires requires{ requires (Zero1 == 0); } && requires{ requires (Zero2 == 0); }
    constexpr auto operator*(constant_t<Zero1>, constant_t<Zero2>)
	{
	    return constant<Zero1 * Zero2>();
	}
}

//x0
namespace senluo
{
    template<auto zero> requires requires{ requires zero == 0; }
    constexpr auto operator*(constant_t<zero>, auto&&)
	{
	    return constant_t<zero>{};
	}

    template<auto zero> requires requires{ requires zero == 0; }
    constexpr auto operator*(auto&&, constant_t<zero>)
	{
	    return constant_t<zero>{};
	}
}

//x1
namespace senluo
{
	template<auto One, typename T> requires requires{ requires (One == 1); }
    constexpr auto operator*(constant_t<One>, T&& value)
	{
	    return FWD(value);
	}

    template<auto One, typename T> requires requires{ requires (One == 1); }
    constexpr auto operator*(T&& value, constant_t<One>)
	{
	    return FWD(value);
	}

	// template<auto One1, auto One2> requires requires{ requires (One1 == 1) && (One2 == 1); }
    // constexpr T operator*(constant_t<One1>, constant_t<One2>)
	// {
	//     return FWD(value);
	// }
}

#include "macro_undef.hpp"
#endif