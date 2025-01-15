#ifndef SENLUO_GENERAL_HPP
#define SENLUO_GENERAL_HPP

#include <concepts>
#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

#include "macro_define.hpp"

#if !defined(__cpp_size_t_suffix) || __cpp_size_t_suffix <= 202006L
constexpr std::size_t operator""uz(unsigned long long x)
{
    return static_cast<std::size_t>(x);
}
#endif

namespace senluo
{
	using std::size_t;

    inline constexpr size_t invalid_index = std::numeric_limits<size_t>::max();

    struct custom_t{};

	template<typename T>
	struct type_tag
	{
		using type = T;
	};

	template<class T, class U>
    concept derived_from = std::derived_from<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

	template<class T, class...U>
	using fwd_type = std::conditional_t< (... && std::is_rvalue_reference_v<U&&>),
			    						 std::remove_reference_t<T>&&,
			    						 std::remove_reference_t<T>& >;

    template <class T = bool>
    struct choice_t
	{
	    T strategy{};
	    bool nothrow = false;
	};
}

namespace senluo
{

    template<size_t I>
    constexpr auto&& arg_at(auto&&...args)noexcept
	{
	    return std::get<I>(std::forward_as_tuple(FWD(args)...));
	}

    constexpr bool equal(auto&& x, auto&& y) 
	    //noexcept(not requires{ FWD(x) == FWD(y); } || requires{ requires noexcept(FWD(x) == FWD(y)); })
	{
	    if constexpr(requires{ FWD(x) == FWD(y); })
		{
		    return FWD(x) == FWD(y);
		}
	    else
		{
		    return false;
		}
	}
}

namespace senluo
{
	template<class T>
	constexpr decltype(auto) to_readonly(T&& t)
	{
		return std::as_const(t);
	}
}


#include "macro_undef.hpp"
#endif