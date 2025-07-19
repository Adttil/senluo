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
                return type_tag<default_container_t<tree_get_t<I, T>...>>{};
            }(std::make_index_sequence<size<T>>{});
        }

        using type = decltype(get_type_tag())::type;
    };

    template<class T>
    using default_vec_container_t = default_vec_container<T>::type;

    template<class T>
    struct vec;

    template<class T>
    vec(T) -> vec<T>;

    namespace detail 
    {
        struct as_vec_fn : adaptor_closure<as_vec_fn>
        {
            template<class T>
            constexpr vec<T> operator()(T&& t) const noexcept
            {
                return { FWD(t) };
            }

            template<class T>
            constexpr vec<T&> operator()(vec<T>& t) const noexcept
            {
                return { t.raw_base() };
            }

            template<class T>
            constexpr vec<const T&> operator()(const vec<T>& t) const noexcept
            {
                return { t.raw_base() };
            }

            template<class T>
            constexpr vec<T> operator()(vec<T>&& t) const noexcept
            {
                return { FWD(t).raw_base() };
            }

            template<class T>
            constexpr vec<T> operator()(const vec<T>&& t) const noexcept
            {
                return { FWD(t).raw_base() };
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::as_vec_fn as_vec{};
    }

    template<class T>
    struct vec : based_on<T>
    {
#include "code_generate/vec_access.code"

        template<size_t I, unwarp_derived_from<vec> Self>
        friend constexpr decltype(auto) tree_get(Self&& self)
        {
            return tree_get<I>(FWD(self).unwrap_base());
        }

        template<class U, class Self> requires (size<T> == size<U>)
        constexpr operator vec<U>(this Self&& self)
        {
            return vec<U>{ FWD(self).raw_base() | make<U> };
        }

        constexpr decltype(auto) operator[](this auto&& self, size_t i)
        noexcept(noexcept(FWD(self).raw_base()[i]))
        requires requires{FWD(self).raw_base()[i];}
        {
            return FWD(self).raw_base()[i];
        }

        template<class U> requires (size<T> == size<U>)
        constexpr vec& operator=(const vec<U>& src)
        {
            [&]<size_t...I>(std::index_sequence<I...>)
            {
                (..., (tree_get<I>(this->raw_base()) = tree_get<I>(src.raw_base())));
            }(std::make_index_sequence<size<T>>{});
            return *this;
        }
    };

    template<class T>
    struct tree_size<vec<T>>
    {
        static constexpr size_t value = size<T>;
    };
}

namespace senluo
{
    template<class Op, class LHS, class RHS>
    constexpr auto default_operate(const vec<LHS>& lhs, const vec<RHS>& rhs)
    {
        if constexpr(std::same_as<LHS, RHS> && requires{ zip_transform(Op{}, lhs.raw_base(), rhs.raw_base()) | make<LHS>; })
        {
            return vec<LHS>{ zip_transform(Op{}, lhs.raw_base(), rhs.raw_base()) | make<LHS> };
        }
        else return[&]<size_t...I>(std::index_sequence<I...>)
        {
            using lazy_t = decltype(zip_transform(Op{}, lhs.raw_base(), rhs.raw_base()));
            using container_t = default_container_t<tree_get_t<I, lazy_t>...>;
            return vec<container_t>{ zip_transform(Op{}, lhs.raw_base(), rhs.raw_base()) | make<container_t> };
        }(std::make_index_sequence<size<LHS>>{});
    }

    template<class LHS, class RHS>
    constexpr auto plus(const vec<LHS>& lhs, const vec<RHS>& rhs)
    {
        return default_operate<std::plus<>>(lhs, rhs);
    }

    template<class RHS, class LHS>
    constexpr auto swap_plus(const vec<RHS>& rhs, const vec<LHS>& lhs)
    {
        return default_operate<std::plus<>>(lhs, rhs);
    }

    template<class LHS, class RHS>
    constexpr auto minus(const vec<LHS>& lhs, const vec<RHS>& rhs)
    {
        return default_operate<std::minus<>>(lhs, rhs);
    }

    template<class RHS, class LHS>
    constexpr auto swap_minus(const vec<RHS>& rhs, const vec<LHS>& lhs)
    {
        return default_operate<std::minus<>>(lhs, rhs);
    }

    template<class LHS, class RHS>
    constexpr auto multiplies(const vec<LHS>& lhs, const vec<RHS>& rhs)
    {
        return default_operate<std::multiplies<>>(lhs, rhs);
    }

    template<class RHS, class LHS>
    constexpr auto swap_multiplies(const vec<RHS>& rhs, const vec<LHS>& lhs)
    {
        return default_operate<std::multiplies<>>(lhs, rhs);
    }

    template<class LHS, class RHS>
    constexpr auto divides(const vec<LHS>& lhs, const vec<RHS>& rhs)
    {
        return default_operate<std::divides<>>(lhs, rhs);
    }

    template<class RHS, class LHS>
    constexpr auto swap_divides(const vec<RHS>& rhs, const vec<LHS>& lhs)
    {
        return default_operate<std::divides<>>(lhs, rhs);
    }

    template<class LHS, class RHS>
    constexpr auto modulus(const vec<LHS>& lhs, const vec<RHS>& rhs)
    {
        return default_operate<std::modulus<>>(lhs, rhs);
    }

    template<class RHS, class LHS>
    constexpr auto swap_modulus(const vec<RHS>& rhs, const vec<LHS>& lhs)
    {
        return default_operate<std::modulus<>>(lhs, rhs);
    }

    template<class T>
    constexpr auto negate(const vec<T>& v)
    {
        if constexpr(requires{ v.raw_base() | transform(std::negate<>{}) | make<T>; })
        {
            return vec<T>{ v.raw_base() | transform(std::negate<>{}) | make<T> };
        }
        else
        {
            using lazy_t = decltype(v.raw_base() | transform(std::negate<>{}));
            using container_t = default_vec_container_t<lazy_t>;
            return vec<container_t>{ v.raw_base() | transform(std::negate<>{}) | make<T> };
        }
    }

    template<class LHS, class RHS>
    constexpr auto multiplies(const LHS& lhs, const vec<RHS>& rhs)
    {
        if constexpr(requires{ rhs.raw_base() | transform([&](const auto& e){ return lhs * e; }) | make<RHS>; })
        {
            return vec<RHS>{ rhs.raw_base() | transform([&](const auto& e){ return lhs * e; }) | make<RHS> };
        }
        else
        {
            using lazy_t = decltype(rhs.raw_base() | transform([&](const auto& e){ return lhs * e; }));
            using container_t = default_vec_container_t<lazy_t>;
            return vec<container_t>{ rhs.raw_base() | transform([&](const auto& e){ return lhs * e; }) | make<container_t> };
        }
    }

    template<class RHS, class LHS>
    constexpr auto swap_multiplies(const vec<RHS>& rhs, const LHS& lhs)
    {
        if constexpr(requires{ rhs.raw_base() | transform([&](const auto& e){ return lhs * e; }) | make<RHS>; })
        {
            return vec<RHS>{ rhs.raw_base() | transform([&](const auto& e){ return lhs * e; }) | make<RHS> };
        }
        else
        {
            using lazy_t = decltype(rhs.raw_base() | transform([&](const auto& e){ return lhs * e; }));
            using container_t = default_vec_container_t<lazy_t>;
            return vec<container_t>{ rhs.raw_base() | transform([&](const auto& e){ return lhs * e; }) | make<container_t> };
        }
    }

    template<class LHS, class RHS>
    constexpr auto multiplies(const vec<LHS>& lhs, const RHS& rhs)
    {
        if constexpr(requires{ lhs.raw_base() | transform([&](const auto& e){ return rhs * e; }) | make<LHS>; })
        {
            return vec<LHS>{ lhs.raw_base() | transform([&](const auto& e){ return rhs * e; }) | make<LHS> };
        }
        else
        {
            using lazy_t = decltype(lhs.raw_base() | transform([&](const auto& e){ return rhs * e; }));
            using container_t = default_vec_container_t<lazy_t>;
            return vec<container_t>{ lhs.raw_base() | transform([&](const auto& e){ return rhs * e; }) | make<container_t> };
        }
    }

    template<class RHS, class LHS>
    constexpr auto swap_multiplies(const RHS& rhs, const vec<LHS>& lhs)
    {
        if constexpr(requires{ lhs.raw_base() | transform([&](const auto& e){ return rhs * e; }) | make<LHS>; })
        {
            return vec<LHS>{ lhs.raw_base() | transform([&](const auto& e){ return rhs * e; }) | make<LHS> };
        }
        else
        {
            using lazy_t = decltype(lhs.raw_base() | transform([&](const auto& e){ return rhs * e; }));
            using container_t = default_vec_container_t<lazy_t>;
            return vec<container_t>{ lhs.raw_base() | transform([&](const auto& e){ return rhs * e; }) | make<container_t> };
        }
    }

    template<class LHS, class RHS>
    constexpr auto divides(const vec<LHS>& lhs, const RHS& rhs)
    {
        if constexpr(requires{ lhs.raw_base() | transform([&](const auto& e){ return e / rhs; }) | make<LHS>; })
        {
            return vec<LHS>{ lhs.raw_base() | transform([&](const auto& e){ return e / rhs; }) | make<LHS> };
        }
        else
        {
            using lazy_t = decltype(lhs.raw_base() | transform([&](const auto& e){ return e / rhs; }));
            using container_t = default_vec_container_t<lazy_t>;
            return vec<container_t>{ lhs.raw_base() | transform([&](const auto& e){ return e / rhs; }) | make<container_t> };
        }
    }

    template<class RHS, class LHS>
    constexpr auto swap_divides(const RHS& rhs, const vec<LHS>& lhs)
    {
        if constexpr(requires{ lhs.raw_base() | transform([&](const auto& e){ return e / rhs; }) | make<LHS>; })
        {
            return vec<LHS>{ lhs.raw_base() | transform([&](const auto& e){ return e / rhs; }) | make<LHS> };
        }
        else
        {
            using lazy_t = decltype(lhs.raw_base() | transform([&](const auto& e){ return e / rhs; }));
            using container_t = default_vec_container_t<lazy_t>;
            return vec<container_t>{ lhs.raw_base() | transform([&](const auto& e){ return e / rhs; }) | make<container_t> };
        }
    }

    template<class LHS, class RHS>
    constexpr decltype(auto) operator+(const vec<LHS>& lhs, const vec<RHS>& rhs)
    {
        return plus(lhs, rhs);
    }

    template<class LHS, class RHS>
    constexpr decltype(auto) operator-(const vec<LHS>& lhs, const vec<RHS>& rhs)
    {
        return minus(lhs, rhs);
    }

    template<class LHS, class RHS>
    constexpr decltype(auto) operator*(const vec<LHS>& lhs, const vec<RHS>& rhs)
    {
        return multiplies(lhs, rhs);
    }

    template<class LHS, class RHS>
    constexpr decltype(auto) operator/(const vec<LHS>& lhs, const vec<RHS>& rhs)
    {
        return divides(lhs, rhs);
    }

    template<class LHS, class RHS>
    constexpr decltype(auto) operator%(const vec<LHS>& lhs, const vec<RHS>& rhs)
    {
        return modulus(lhs, rhs);
    }

    template<class T>
    constexpr decltype(auto) operator-(const vec<T>& v)
    {
        return negate(v);
    }

    template<class LHS, class RHS>
    constexpr decltype(auto) operator*(const LHS& lhs, const vec<RHS>& rhs)
    {
        return multiplies(lhs, rhs);
    }

    template<class LHS, class RHS>
    constexpr decltype(auto) operator*(const vec<LHS>& lhs, const RHS& rhs)
    {
        return multiplies(lhs, rhs);
    }

    template<class LHS, class RHS>
    constexpr decltype(auto) operator/(const vec<LHS>& lhs, const RHS& rhs)
    {
        return divides(lhs, rhs);
    }
}

namespace senluo
{
    namespace detail::dot_fn_ns
    {
        void dot();
        void swap_dot();

        struct dot_fn
        {
            template<class LHS, class RHS>
            constexpr decltype(auto) operator()(LHS&& lhs, RHS&& rhs) const
            {
                if constexpr(requires{ dot(FWD(lhs) | as_vec, FWD(rhs) | as_vec); })
                {
                    return dot(FWD(lhs) | as_vec, FWD(rhs) | as_vec);
                }
                else if constexpr(requires{ swap_dot(FWD(rhs) | as_vec, FWD(lhs) | as_vec); })
                {
                    return swap_dot(FWD(rhs) | as_vec, FWD(lhs) | as_vec);
                }   
                else return [&]<size_t...I>(std::index_sequence<I...>) -> decltype(auto)
                {
                    return (... + (tree_get<I>(lhs) * tree_get<I>(rhs)));
                }(std::make_index_sequence<size<LHS>>{});
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::dot_fn_ns::dot_fn dot{};
    }
}

namespace senluo
{
  
}

#include "tools/macro_undef.hpp"
#endif // SENLUO_VEC_HPP