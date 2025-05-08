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

    template<class T> requires std::constructible_from<std::decay_t<T>, T&&>
    constexpr std::decay_t<T> decay_copy(T&& t) noexcept(noexcept(static_cast<std::decay_t<T>>(std::declval<T>())))
    {
        return static_cast<std::decay_t<T>>(FWD(t));
    }

    namespace detail 
    {
        struct pass_fn
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const noexcept(noexcept((T)FWD(t)))
            {
                return (T)FWD(t);
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::pass_fn pass{};
    }
}

namespace senluo 
{
	template<class T, template<class> class Impl>
    struct modifier
    {
        using base_type = T;

        template<class U>
        using modifier_type = Impl<U>;

        T base;
    };

    template<class T>
    concept modified = std::derived_from<T, modifier<typename T::base_type, T::template modifier_type>>
        && std::same_as<T, typename T::template modifier_type<typename T::base_type>>;

    template<class T>
    struct remove_cvref : std::remove_cvref<T>{};

    template<class T>
    using remove_cvref_t = remove_cvref<T>::type;

    template<modified T>
    struct remove_cvref<T>
    {
        using type = T::template modifier_type<std::remove_cvref_t<typename T::base_type>>;
    };

    template<class T>
    struct add_lref
    {
        using type = T&;
    };

    template<class T>
    using add_lref_t = add_lref<T>::type;

    template<modified T>
    struct add_lref<T>
    {
        using type = T::template modifier_type<typename T::base_type&>;
    };

    template<class T>
    struct add_rref
    {
        using type = T&&;
    };

    template<class T>
    using add_rref_t = add_rref<T>::type;

    template<modified T>
    struct add_rref<T>
    {
        using type = T::template modifier_type<typename T::base_type&&>;
    };

    template<class T>
    struct add_const
    {
        using type = const T;
    };

    template<class T>
    using add_const_t = add_const<T>::type;

    template<modified T>
    struct add_const<T>
    {
        using type = T::template modifier_type<const typename T::base_type>;
    };
}

namespace senluo::detail
{
    

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

	template<class T>
	constexpr auto&& to_readonly(T&& t)
	{
		if constexpr(std::is_object_v<T>)
		{
			return std::move(std::as_const(t));
		}
		else
		{
			return std::as_const(t);
		}
	}
}

#include "macro_undef.hpp"
#endif