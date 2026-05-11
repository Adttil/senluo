#ifndef SENLUO_GENERAL_HPP
#define SENLUO_GENERAL_HPP

#include <cstddef>
#include <type_traits>

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
}

namespace senluo::detail
{
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

    constexpr bool equal(auto&& x, auto&& y) 
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

#include "macro_undef.hpp"
#endif