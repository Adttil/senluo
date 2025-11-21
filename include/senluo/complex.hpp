#ifndef SENLUO_COMPLEX_HPP
#define SENLUO_COMPLEX_HPP

#include <functional>
#include <utility>
#include "tools/math.hpp"
#include "core.hpp"

#include "tools/macro_define.hpp"


namespace senluo
{
    template<class T>
    using default_complex_container_t = default_container_t<tree_get_t<0uz, T>, tree_get_t<1uz, T>>;

    namespace complex_ns
    {
        template<class T>
        struct complex;

        template<class T>
        complex(T) -> complex<T>;
    }
    using complex_ns::complex;

    template<class T>
    concept complex_wrapped = elementary<T> && requires(std::remove_cvref_t<T>& t) 
    {
        { []<class U>(complex<U>&)->complex<U>*{}(t) } -> std::same_as<std::remove_cvref_t<T>*>;
    };

    namespace detail 
    {
        struct as_complex_fn : adaptor_closure<as_complex_fn>
        {
            template<class T>
            constexpr complex<pass_t<T>> operator()(T&& t) const noexcept(noexcept(pass(FWD(t))))
            {
                return { FWD(t) };
            }

            template<class T>
            constexpr complex<T&> operator()(complex<T>& t) const noexcept
            {
                return { t.base };
            }

            template<class T>
            constexpr complex<const T&> operator()(const complex<T>& t) const noexcept
            {
                return { t.base };
            }

            template<class T>
            constexpr complex<T> operator()(complex<T>&& t) const noexcept(std::is_nothrow_move_constructible_v<complex<T>>)
            {
                return std::move(t);
            }

            template<class T>
            constexpr complex<T> operator()(const complex<T>&& t) const noexcept(std::is_nothrow_copy_constructible_v<complex<T>>)
            {
                return t;
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::as_complex_fn as_complex{};
    }

    template<class T>
    struct complex_ns::complex
    {
        using base_type = T;

        T base;

        template<size_t I, class Self>
        constexpr decltype(auto) tree_get(this Self&& self, custom_t = {})
        {
            return senluo::tree_get<I>(FWD(self, base));
        }

        static consteval size_t get_size(custom_t = {})
        {
            return 2uz;
        }

        template<class U>
        static constexpr complex make_from(U&& u, custom_t = {})
        AS_EXPRESSION(
            complex<T>{ FWD(u) | make<T> }
        )

        template<class U, class Self>
        constexpr operator complex<U>(this Self&& self)
        AS_EXPRESSION(
            complex<U>{ FWD(self, base) | make<U> }
        )

        template<complex_wrapped U>
        constexpr complex& operator=(U&& src)&
        {
            this->real() = FWD(src).real();
            this->imag() = FWD(src).imag();
            return *this;
        }

        constexpr decltype(auto) real(this auto&& self)
        {
            return senluo::tree_get<0uz>(FWD(self, base));
        }

        constexpr decltype(auto) imag(this auto&& self)
        {
            return senluo::tree_get<1uz>(FWD(self, base));
        }

        constexpr decltype(auto) norm(this auto&& self)
        {
            return FWD(self).real() * FWD(self).real() + FWD(self).imag() * FWD(self).imag();
        }

        friend constexpr bool operator==(const complex&, const complex&) = default;
    };
}

namespace senluo::detail
{
    template<class T, class Lazy>
    constexpr auto to_complex(Lazy&& lazy_val)
    {
        using base_t = std::remove_cvref_t<typename std::remove_cvref_t<T>::base_type>;
        if constexpr(requires{ FWD(lazy_val) | make<base_t>; })
        {
            return complex<base_t>{ FWD(lazy_val) | make<base_t> };
        }
        else
        {
            using container_t = default_complex_container_t<Lazy>;
            return complex<container_t>{ FWD(lazy_val) | make<container_t> };
        }
    }

    template<class L, class R>
    struct complex_mul_tree
    {
        L lhs;
        R rhs;

        template<size_t I, class Self>
        constexpr decltype(auto) tree_get(this Self&& self, custom_t = {})
        {
            if constexpr(I == 0uz)
            {
                return senluo::tree_get<0uz>(FWD(self, lhs)) * senluo::tree_get<0uz>(FWD(self, rhs))
                - senluo::tree_get<1uz>(FWD(self, lhs)) * senluo::tree_get<1uz>(FWD(self, rhs));
            }
            else
            {
                return senluo::tree_get<0uz>(FWD(self, lhs)) * senluo::tree_get<1uz>(FWD(self, rhs))
                + senluo::tree_get<1uz>(FWD(self, lhs)) * senluo::tree_get<0uz>(FWD(self, rhs));
            }
        }

        static consteval size_t get_size(custom_t = {}) noexcept
        {
            return 2uz;
        }
    };

    template<class L, class R, class N>
    struct complex_div_tree
    {
        L lhs;
        R rhs;
        N r_norm = rhs.norm();

        template<size_t I, class Self>
        constexpr decltype(auto) tree_get(this Self&& self, custom_t = {})
        {
            if constexpr(I == 0uz)
            {
                return (
                    senluo::tree_get<0uz>(FWD(self, lhs)) * senluo::tree_get<0uz>(FWD(self, rhs))
                    + senluo::tree_get<1uz>(FWD(self, lhs)) * senluo::tree_get<1uz>(FWD(self, rhs))
                )
                / FWD(self, r_norm);
            }
            else
            {
                return (
                    senluo::tree_get<1uz>(FWD(self, lhs)) * senluo::tree_get<0uz>(FWD(self, rhs))
                    - senluo::tree_get<0uz>(FWD(self, lhs)) * senluo::tree_get<1uz>(FWD(self, rhs))
                )
                / FWD(self, r_norm)
                ;
            }
        }

        static consteval size_t get_size(custom_t = {}) noexcept
        {
            return 2uz;
        }
    };

    struct lazy_complex_mul_fn : adaptor<lazy_complex_mul_fn>
    {
        template<class L, class R>
        static constexpr auto adapt(L&& l, R&& r)
        {
            return complex_mul_tree<decltype(detail::astrict_unchecked<stricture_t::readonly>(FWD(l))),
                                decltype(detail::astrict_unchecked<stricture_t::readonly>(FWD(r)))>
            {
                detail::astrict_unchecked<stricture_t::readonly>(FWD(l)),
                detail::astrict_unchecked<stricture_t::readonly>(FWD(r))
            };
        }
    };

    struct lazy_complex_div_fn : adaptor<lazy_complex_div_fn>
    {
        template<class L, class R>
        static constexpr auto adapt(L&& l, R&& r)
        {
            decltype(auto) rhs = detail::astrict_unchecked<stricture_t::readonly>(FWD(r));
            decltype(auto) r_norm = rhs.norm();
            return complex_div_tree<decltype(detail::astrict_unchecked<stricture_t::readonly>(FWD(l))),
                                decltype(rhs),
                                pass_t<decltype(rhs.norm())>>
            {
                detail::astrict_unchecked<stricture_t::readonly>(FWD(l)),
                FWD(rhs),
                FWD(r_norm)
            };
        }
    };
}

namespace senluo 
{
    inline namespace functors
    {
        inline constexpr detail::lazy_complex_mul_fn lazy_complex_mul{};
        inline constexpr detail::lazy_complex_div_fn lazy_complex_div{};
    }
}

namespace senluo::complex_ns
{
    template<complex_wrapped LHS, complex_wrapped RHS>
    constexpr decltype(auto) operator+(LHS&& lhs, RHS&& rhs)
    {
        return detail::to_complex<LHS>(zip_transform(std::plus<>{}, FWD(lhs), FWD(rhs)));
    }

    template<class LHS, complex_wrapped RHS>
    constexpr decltype(auto) operator+(LHS&& lhs, RHS&& rhs)
    {
        return detail::to_complex<LHS>(zip_transform(std::plus<>{}, combine(FWD(lhs), constant_t<0>{}), FWD(rhs)));
    }

    template<complex_wrapped LHS, class RHS>
    constexpr decltype(auto) operator+(LHS&& lhs, RHS&& rhs)
    {
        return detail::to_complex<LHS>(zip_transform(std::plus<>{}, FWD(lhs), combine(FWD(rhs), constant_t<0>{})));
    }

    template<class LHS, class RHS>
    constexpr decltype(auto) operator-(LHS&& lhs, RHS&& rhs)
    {
        return detail::to_complex<LHS>(zip_transform(std::minus<>{}, FWD(lhs), FWD(rhs)));
    }

    template<complex_wrapped LHS, complex_wrapped RHS>
    constexpr decltype(auto) operator*(LHS&& lhs, RHS&& rhs)
    {
        return detail::to_complex<LHS>(lazy_complex_mul(FWD(lhs), FWD(rhs)));
    }

    template<complex_wrapped LHS, complex_wrapped RHS>
    constexpr decltype(auto) operator/(LHS&& lhs, RHS&& rhs)
    {
        return detail::to_complex<LHS>(lazy_complex_div(FWD(lhs), FWD(rhs)));
    }

    template<class T>
    constexpr decltype(auto) operator-(T&& t)
    {
        return detail::to_complex<T>(transform(FWD(t), std::negate<>{}));
    }

    template<elementary LHS, complex_wrapped RHS>
    constexpr decltype(auto) operator*(LHS&& lhs, RHS&& rhs)
    {
        return detail::to_complex<RHS>(transform(FWD(rhs), [lhs = pass(FWD(lhs))](auto&& e){ return lhs * FWD(e); }));
    }

    template<complex_wrapped LHS, elementary RHS>
    constexpr decltype(auto) operator*(LHS&& lhs, RHS&& rhs)
    {
        return FWD(rhs) * FWD(lhs);
    }

    template<complex_wrapped LHS, elementary RHS>
    constexpr decltype(auto) operator/(LHS&& lhs, RHS&& rhs)
    {
        return detail::to_complex<LHS>(transform(FWD(lhs), [rhs = pass(FWD(rhs))](auto&& e){ return FWD(e) / rhs; }));
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