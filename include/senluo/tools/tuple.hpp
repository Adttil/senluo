#ifndef SENLUO_TUPLE_HPP
#define SENLUO_TUPLE_HPP
//this tuple is aggregate

#include "general.hpp"

#include "macro_define.hpp"

namespace senluo::tuple_ns
{
    template<class...T>
    struct tuple;

#include "../code_generate/tuple_specialization.code"

	template<class T, class...Rest>
    struct tuple<T, Rest...>
	{
	    SENLUO(no_unique_address) T              first;
	    SENLUO(no_unique_address) tuple<Rest...> rest;

		template<size_t I, derived_from<tuple> Self> requires (I <= sizeof...(Rest))
	    friend constexpr auto&& get(Self&& self) noexcept
		{
		    if constexpr (I)
			{
			    return get<I - 1>(FWD(self, rest));
			}
		    else
			{
			    return FWD(self, first);
			}
		}

	    friend constexpr bool operator==(const tuple&, const tuple&) = default;
	};

	template<class...T>
    tuple(T...) -> tuple<std::decay_t<T>...>;
}

template<class...T>
struct std::tuple_size<senluo::tuple_ns::tuple<T...>> : std::integral_constant<std::size_t, sizeof...(T)> {};

template<size_t I, class...T>
struct std::tuple_element<I, senluo::tuple_ns::tuple<T...>> : std::tuple_element<I, std::tuple<T...>> {};

namespace senluo
{
	using tuple_ns::tuple;
	namespace detail 
	{
		template<size_t I, class...T>
    	constexpr std::tuple_element_t<I, tuple<T&&...>> arg_at(T&&...args)noexcept
		{
	    	return get<I>(tuple<T&&...>{ FWD(args)... });
		}

		struct make_tuple_fn
		{
			template<class...Args>
    		constexpr tuple<std::decay_t<Args>...> operator()(Args&&...args) const
			AS_EXPRESSION(tuple<std::decay_t<Args>...>{ FWD(args)... });
		};

		struct fwd_as_tuple_fn
		{
			template<class...Args>
    		constexpr tuple<Args&&...> operator()(Args&&...args) const noexcept
			{
			    return { FWD(args)... };
			};
		};

		struct tuple_cat_fn
		{
    		constexpr tuple<> operator()()const noexcept
			{
				return {};
			}

			template<class...Args>
			constexpr tuple<Args...> operator()(const tuple<Args...>& tpl)const
			{
				return tpl;
			}

			template<class...Args1, class...Args2>
    		constexpr tuple<Args1..., Args2...> operator()(const tuple<Args1...>& tpl1, const tuple<Args2...>& tpl2) const
			{
			    return [&]<size_t...I, size_t...J>(std::index_sequence<I...>, std::index_sequence<J...>)
				{
					return tuple<Args1..., Args2...>{ get<I>(tpl1)..., get<J>(tpl2)... };
				}(std::make_index_sequence<sizeof...(Args1)>{}, std::make_index_sequence<sizeof...(Args2)>{});
			};

			template<class Tpl1, class Tpl2, class...Rest>
			constexpr auto operator()(const Tpl1& tpl1, const Tpl2& tpl2, const Rest&...rest) const
			{
				return tuple_cat_fn{}(tuple_cat_fn{}(tpl1, tpl2), rest...);
			}
		};
	}

	inline constexpr detail::make_tuple_fn make_tuple{};

	inline constexpr detail::fwd_as_tuple_fn fwd_as_tuple{};

	inline constexpr detail::tuple_cat_fn tuple_cat{};
}

#include "macro_undef.hpp"
#endif