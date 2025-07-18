#ifndef SENLUO_TUPLE_HPP
#define SENLUO_TUPLE_HPP
//this tuple is aggregate

#include <cstddef>
#include <utility>

#include "general.hpp"
#include "constant.hpp"

#include "macro_define.hpp"

namespace senluo::tuple_ns
{
	template<class T, size_t I>
	struct tuple_unit
	{
		T element;

		constexpr auto&& get(constant_t<I>)& noexcept
		{
			return element;
		}

		constexpr auto&& get(constant_t<I>)const & noexcept
		{
			return element;
		}

		constexpr auto&& get(constant_t<I>)&& noexcept
		{
			return FWD(std::move(*this), element);
		}

		constexpr auto&& get(constant_t<I>)const && noexcept
		{
			return FWD(std::move(*this), element);
		}

		static consteval std::type_identity<T> type_identity_at(constant_t<I> = {})noexcept
		{
			return {};
		}

		static consteval size_t index_of(std::type_identity<T> = {})noexcept
		{
			return {};
		}

		friend bool operator==(const tuple_unit&, const tuple_unit&) = default;
	};

	template<class IndexSeq, class...T>
	struct tuple_impl;

	template<size_t...I, class...T>
	struct tuple_impl<std::index_sequence<I...>, T...> : tuple_unit<T, I>...
	{
		using tuple_unit<T, I>::get...;
		using tuple_unit<T, I>::type_identity_at...;
		using tuple_unit<T, I>::index_of...;

		static consteval size_t size()
		{
			return sizeof...(T);
		}

		template<size_t J, class Self>
		constexpr auto&& get(this Self&& self) noexcept
		{
			return FWD(self).get(constant_t<J>{});
		}

		template<size_t J>
		static consteval auto type_identity_at()noexcept
		{
			return type_identity_at(constant_t<J>{});
		}

		template<size_t J>
		using type_at = decltype(type_identity_at(constant_t<J>{}))::type;

		template<class U>
		static consteval size_t index_of()noexcept
		{
			return index_of(std::type_identity<U>{});
		}

		friend bool operator==(const tuple_impl&, const tuple_impl&) = default;
	};

    template<class...T>
    struct tuple : tuple_impl<std::index_sequence_for<T...>, T...>
	{
		friend bool operator==(const tuple&, const tuple&) = default;
	};

	template<class...T>
    tuple(T...) -> tuple<std::decay_t<T>...>;

	template<size_t I, class T>
	constexpr auto&& get(T&& tpl) noexcept
	{
		return std::forward<T>(tpl).get(constant_t<I>{});
	}
}

template<class...T>
struct std::tuple_size<senluo::tuple_ns::tuple<T...>>
 : std::integral_constant<std::size_t, sizeof...(T)>
{};

template<size_t I, class...T>
struct std::tuple_element<I, senluo::tuple_ns::tuple<T...>>
 : decltype(senluo::tuple_ns::tuple<T...>::template type_identity_at<I>())
{};

namespace senluo
{
	using tuple_ns::tuple;

	namespace detail 
	{
		template<size_t I, class...Args>
    	constexpr tuple<Args&&...>::template type_at<I> arg_at(Args&&...args)noexcept
		{
	    	return tuple<Args&&...>{ std::forward<Args>(args)... }.get(constant_t<I>{});
		}

		struct make_tuple_fn
		{
			template<class...Args>
    		static constexpr tuple<std::decay_t<Args>...> operator()(Args&&...args)
			AS_EXPRESSION(tuple<std::decay_t<Args>...>{ std::forward<Args>(args)... });
		};

		struct fwd_as_tuple_fn
		{
			template<class...Args>
    		static constexpr tuple<Args&&...> operator()(Args&&...args) noexcept
			{
			    return { std::forward<Args>(args)... };
			};
		};

		struct tuple_cat_fn
		{
    		static constexpr tuple<> operator()() noexcept
			{
				return {};
			}

			template<class...Args>
			static constexpr tuple<Args...> operator()(const tuple<Args...>& tpl)
			{
				return tpl;
			}

			template<class...Args1, class...Args2>
    		static constexpr tuple<Args1..., Args2...> operator()(const tuple<Args1...>& tpl1, const tuple<Args2...>& tpl2)
			{
			    return [&]<size_t...I, size_t...J>(std::index_sequence<I...>, std::index_sequence<J...>)
				{
					return tuple<Args1..., Args2...>{ tpl1.get(constant_t<I>{})..., tpl2.get(constant_t<J>{})... };
				}(std::make_index_sequence<sizeof...(Args1)>{}, std::make_index_sequence<sizeof...(Args2)>{});
			};

			template<class Tpl1, class Tpl2, class...Rest>
			static constexpr auto operator()(const Tpl1& tpl1, const Tpl2& tpl2, const Rest&...rest)
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