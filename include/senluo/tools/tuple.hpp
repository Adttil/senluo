#ifndef SENLUO_TUPLE_HPP
#define SENLUO_TUPLE_HPP
//this tuple is aggregate

#include <utility>

#include "general.hpp"

#include "macro_define.hpp"

namespace senluo
{
    namespace tuple_ns 
    {
		template<size_t I, class T>
    	struct tuple_unit
    	{
    	    T value;

    	    constexpr T& get(std::integral_constant<size_t, I> = {})& noexcept
    	    {
    	        return value;
    	    }

    	    constexpr const T& get(std::integral_constant<size_t, I> = {}) const& noexcept
    	    {
    	        return value;
    	    }

    	    constexpr T&& get(std::integral_constant<size_t, I> = {})&& noexcept
    	    {
    	        return (T&&)value;
    	    }

    	    constexpr const T&& get(std::integral_constant<size_t, I> = {}) const&& noexcept
    	    {
    	        return (const T&&)value;
    	    }

    	    constexpr T& get(std::type_identity<T> = {})& noexcept
    	    {
    	        return value;
    	    }

    	    constexpr const T& get(std::type_identity<T> = {}) const& noexcept
    	    {
    	        return value;
    	    }

    	    constexpr T&& get(std::type_identity<T> = {})&& noexcept
    	    {
    	        return std::move(value);
    	    }

    	    constexpr const T&& get(std::type_identity<T> = {}) const&& noexcept
    	    {
    	        return std::move(value);
    	    }

    	    static consteval std::type_identity<T> type_identity(std::integral_constant<size_t, I> = {}) noexcept
    	    {
    	        return {};
    	    }

    	    static consteval size_t index(std::type_identity<T> = {}) noexcept
    	    {
    	        return I;
    	    } 

			friend constexpr bool operator==(const tuple_unit&, const tuple_unit&) = default;
    	};

    	template<class Seq, class...T>
    	struct tuple_impl;

    	template<size_t...I, class...T>
    	struct tuple_impl<std::index_sequence<I...>, T...> : tuple_unit<I, T>...
    	{
    	    using tuple_unit<I, T>::get...;
    	    using tuple_unit<I, T>::type_identity...;
    	    using tuple_unit<I, T>::index...;

    	    static consteval size_t size() noexcept
    	    {
    	        return sizeof...(T);
    	    }

    	    template<size_t J, class Self>
    	    constexpr auto&& get(this Self&& self) noexcept
    	    {
    	        return FWD(self).get(std::integral_constant<size_t, J>{});
    	    }

    	    template<class U, class Self>
    	    constexpr auto&& get(this Self&& self) noexcept
    	    {
    	        return FWD(self).get(std::type_identity<U>{});
    	    }

    	    template<size_t J>
    	    static consteval auto type_identity() noexcept
    	    {
    	        return type_identity(std::integral_constant<size_t, J>{});
    	    }

    	    template<size_t J>
    	    using type_at = decltype(tuple_impl::type_identity<J>())::type;

    	    template<class U>
    	    static consteval size_t index() noexcept
    	    {
    	        return index(std::type_identity<U>{});
    	    }

			friend constexpr bool operator==(const tuple_impl&, const tuple_impl&) = default;
    	};

        template<class...T>
        struct tuple : tuple_impl<std::index_sequence_for<T...>, T...>
		{
			friend constexpr bool operator==(const tuple&, const tuple&) = default;
		};

        template<class...T>
        tuple(T...) -> tuple<T...>;

        template<size_t I, class Self>
        constexpr auto&& get(Self&& self) noexcept
        {
            return FWD(self).template get<I>();
        }

        template<class T, class Self>
        constexpr auto&& get(Self&& self) noexcept
        {
            return FWD(self).template get<T>();
        }
    }
    using tuple_ns::tuple;
}

template<class...T>
struct std::tuple_size<senluo::tuple<T...>> : std::integral_constant<size_t, sizeof...(T)>{};

template<size_t I, class...T>
struct std::tuple_element<I, senluo::tuple<T...>> 
: std::type_identity<typename senluo::tuple<T...>::template type_at<I>>{};

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