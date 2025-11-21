#ifndef SENLUO_VEC_HPP
#define SENLUO_VEC_HPP

#include <functional>
#include <utility>

#include "core.hpp"

#include "tools/macro_define.hpp"

namespace senluo
{
    template<class T>
    struct default_vec_container
    {
        static consteval auto get_type_tag()
        {
            return []<size_t...I>(std::index_sequence<I...>)
            {
                return std::type_identity<default_container_t<tree_get_t<I, T>...>>{};
            }(std::make_index_sequence<size<T>>{});
        }

        using type = decltype(get_type_tag())::type;
    };

    template<class T>
    using default_vec_container_t = default_vec_container<T>::type;

    namespace vec_ns
    {
        template<class T>
        struct vec;

        template<class T>
        vec(T) -> vec<T>;
    }
    using vec_ns::vec;

    template<class T>
    concept vec_wrapped = requires(std::remove_cvref_t<T>& t) 
    {
        { []<class U>(vec<U>&)->vec<U>*{}(t) } -> std::same_as<std::remove_cvref_t<T>*>;
    };

    namespace detail 
    {
        struct as_vec_fn : adaptor_closure<as_vec_fn>
        {
            template<class T>
            constexpr vec<pass_t<T>> operator()(T&& t) const noexcept(noexcept(pass(FWD(t))))
            {
                return { FWD(t) };
            }

            template<class T>
            constexpr vec<T&> operator()(vec<T>& t) const noexcept
            {
                return { t.base };
            }

            template<class T>
            constexpr vec<const T&> operator()(const vec<T>& t) const noexcept
            {
                return { t.base };
            }

            template<class T>
            constexpr vec<T> operator()(vec<T>&& t) const noexcept(std::is_nothrow_move_constructible_v<vec<T>>)
            {
                return std::move(t);
            }

            template<class T>
            constexpr vec<T> operator()(const vec<T>&& t) const noexcept(std::is_nothrow_copy_constructible_v<vec<T>>)
            {
                return t;
            }
        };

        struct fwd_as_vec_fn : adaptor_closure<fwd_as_vec_fn>
        {

        };
    }

    inline namespace functors
    {
        inline constexpr detail::as_vec_fn as_vec{};
    }

    template<class T>
    struct vec_ns::vec
    {
        using base_type = T;

        T base;

        static consteval bool is_elementary(custom_t = {}) noexcept
        {
            return false;
        }

        template<size_t I, class Self>
        constexpr decltype(auto) tree_get(this Self&& self, custom_t = {})
        {
            return senluo::tree_get<I>(FWD(self, base));
        }

        static consteval size_t get_size(custom_t = {}) noexcept
        {
            return size<T>;
        }

        template<class U>
        static constexpr vec make_from(U&& u, custom_t = {})
        AS_EXPRESSION(
            vec<T>{ FWD(u) | make<T> }
        )

        template<class U, class Self> requires (size<T> == size<U>)
        constexpr operator vec<U>(this Self&& self)
        {
            return vec<U>{ FWD(self, base) | make<U> };
        }

        template<vec_wrapped U> requires (size<T> == size<U>)
        constexpr vec& operator=(U&& src)&
        {
            [&]<size_t...I>(std::index_sequence<I...>)
            {
                (..., (senluo::tree_get<I>(*this) = senluo::tree_get<I>(FWD(src))));
            }(std::make_index_sequence<size<T>>{});
            return *this;
        }

#include "code_generate/vec_access.code"

        friend constexpr bool operator==(const vec&, const vec&) = default;
    };
}

namespace senluo::detail
{
    template<class T, class Lazy>
    constexpr auto to_vec(Lazy&& lazy_val)
    {
        using base_t = std::remove_cvref_t<typename std::remove_cvref_t<T>::base_type>;
        if constexpr(requires{ FWD(lazy_val) | make<base_t>; })
        {
            return vec<base_t>{ FWD(lazy_val) | make<base_t> };
        }
        else
        {
            using container_t = default_vec_container_t<Lazy>;
            return vec<container_t>{ FWD(lazy_val) | make<container_t> };
        }
    }
}

namespace senluo::vec_ns
{
    template<class LHS, class RHS>
    constexpr decltype(auto) operator+(LHS&& lhs, RHS&& rhs)
    {
        return detail::to_vec<LHS>(zip_transform(std::plus<>{}, FWD(lhs), FWD(rhs)));
    }

    template<class LHS, class RHS>
    constexpr decltype(auto) operator-(LHS&& lhs, RHS&& rhs)
    {
        return detail::to_vec<LHS>(zip_transform(std::minus<>{}, FWD(lhs), FWD(rhs)));
    }

    template<vec_wrapped LHS, vec_wrapped RHS>
    constexpr decltype(auto) operator*(LHS&& lhs, RHS&& rhs)
    {
        return detail::to_vec<LHS>(zip_transform(std::multiplies<>{}, FWD(lhs), FWD(rhs)));
    }

    template<vec_wrapped LHS, vec_wrapped RHS>
    constexpr decltype(auto) operator/(LHS&& lhs, RHS&& rhs)
    {
        return detail::to_vec<LHS>(zip_transform(std::divides<>{}, FWD(lhs), FWD(rhs)));
    }

    template<class LHS, class RHS>
    constexpr decltype(auto) operator%(LHS&& lhs, RHS&& rhs)
    {
        return detail::to_vec<LHS>(zip_transform(std::modulus<>{}, FWD(lhs), FWD(rhs)));
    }

    template<class T>
    constexpr decltype(auto) operator-(T&& t)
    {
        return detail::to_vec<T>(transform(FWD(t), std::negate<>{}));
    }

    template<class LHS, vec_wrapped RHS>
    constexpr decltype(auto) operator*(LHS&& lhs, RHS&& rhs)
    {
        return detail::to_vec<RHS>(transform(FWD(rhs), [lhs = pass(FWD(lhs))](auto&& e){ return lhs * FWD(e); }));
    }

    template<vec_wrapped LHS, class RHS>
    constexpr decltype(auto) operator*(LHS&& lhs, RHS&& rhs)
    {
        return FWD(rhs) * FWD(lhs);
    }

    template<vec_wrapped LHS, class RHS>
    constexpr decltype(auto) operator/(LHS&& lhs, RHS&& rhs)
    {
        return detail::to_vec<LHS>(transform(FWD(lhs), [rhs = pass(FWD(rhs))](auto&& e){ return FWD(e) / rhs; }));
    }
}

namespace senluo
{
    namespace detail
    {
        struct dot_fn
        {
            template<class LHS, class RHS>
            constexpr decltype(auto) operator()(LHS&& lhs, RHS&& rhs) const
            {
                return [&]<size_t...I>(std::index_sequence<I...>) -> decltype(auto)
                {
                    return (... + (tree_get<I>(lhs) * tree_get<I>(rhs)));
                }(std::make_index_sequence<size<LHS>>{});
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::dot_fn dot{};
    }
}

#include "tools/macro_undef.hpp"
#endif // SENLUO_VEC_HPP