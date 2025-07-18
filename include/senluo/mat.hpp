#ifndef SENLUO_MAT_HPP
#define SENLUO_MAT_HPP

#include <functional>
#include <utility>

#include "tools/constant.hpp"
#include "tools/math.hpp"
#include "core.hpp"
#include "vec.hpp"

#include "tools/macro_define.hpp"

namespace senluo
{
    namespace detail 
    {
        struct lazy_mat_mul_fn : adaptor<lazy_mat_mul_fn>
        {
            template<size_t I, size_t S, size_t J>
            static constexpr auto get_element_layout()
            {
                return []<size_t...K>(std::index_sequence<K...>)
                {
                    return tuple
                    {
                        array{ 0uz },
                        tuple{ array{ 1uz, K, I }... },
                        array{ 2uz, J }
                    };
                }(std::make_index_sequence<S>{});
            }

            template<size_t N, size_t S, size_t J>
            static constexpr auto get_column_layout()
            {
                return []<size_t...I>(std::index_sequence<I...>)
                {
                    return make_tuple(get_element_layout<I, S, J>()...);
                }(std::make_index_sequence<N>{});
            }

            template<size_t N, size_t S, size_t M>
            static constexpr auto get_layout()
            {
                return []<size_t...J>(std::index_sequence<J...>)
                {
                    return make_tuple(get_column_layout<N, S, J>()...);
                }(std::make_index_sequence<M>{});
            }

            template<class L, class R>
            constexpr decltype(auto) adapt(L&& l, R&& r) const
            {
                constexpr size_t n = tensor_shape<L>[1uz];
                constexpr size_t s = size<L>;
                constexpr size_t m = size<R>;
                
                constexpr auto layout = get_layout<n, s, m>();
                constexpr auto op_tree = detail::replicate(operation_t::apply_invoke, shape<int[m][n]>);

                return decltype(combine(decltype(dot){}, FWD(l), FWD(r)) | relayout<layout> | operate<op_tree>)
                {
                    dot, unwrap_fwd(FWD(l)), unwrap_fwd(FWD(r))
                };
            }
        };
    }
   
    inline namespace functors
    {
        inline constexpr detail::lazy_mat_mul_fn lazy_mat_mul{};
    }

    namespace detail 
    {
        struct lazy_mat_mul_vec_fn : adaptor<lazy_mat_mul_vec_fn>
        {
            template<class M, class V>
            constexpr decltype(auto) adapt(M&& mat, V&& vec)const
            {
                constexpr size_t row_count = size<M>;
                return zip_transform(dot, FWD(mat) | transpose<>, FWD(vec) | repeat<row_count>);
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::lazy_mat_mul_vec_fn lazy_mat_mul_vec{};
    }
}

namespace senluo
{
    template<class T>
    struct default_mat_container
    {
        template<size_t I>
        static consteval auto get_column_type_tag()
        {
            return []<size_t...J>(std::index_sequence<J...>)
            {
                return type_tag<default_container_t<subtree_t<T, I, J>...>>{};
            }(std::make_index_sequence<mat_shape<T>[1uz]>{});
        }

        template<size_t I>
        using column_type = decltype(get_column_type_tag<I>())::type; 

        static consteval auto get_type_tag()
        {
            return []<size_t...I>(std::index_sequence<I...>)
            {
                return type_tag<default_container_t<column_type<I>...>>{};
            }(std::make_index_sequence<size<T>>{});
        }

        using type = decltype(get_type_tag())::type;
    };

    template<class T>
    using default_mat_container_t = default_mat_container<T>::type;

    namespace mat_ns 
    {
        template<class T>
        struct mat;

        template<class T>
        mat(T) -> mat<T>;
    }
    using mat_ns::mat;

    namespace detail
    {
        struct as_mat_fn : tree_adaptor_closure<as_mat_fn>
        {
            template<class T>
            constexpr mat<T> operator()(T&& t) const noexcept
            {
                return { FWD(t) };
            }

            template<class T>
            constexpr mat<T&> operator()(mat<T>& t) const noexcept
            {
                return { t.raw_base() };
            }

            template<class T>
            constexpr mat<const T&> operator()(const mat<T>& t) const noexcept
            {
                return { t.raw_base() };
            }

            template<class T>
            constexpr mat<T> operator()(mat<T>&& t) const noexcept
            {
                return { FWD(t).raw_base() };
            }

            template<class T>
            constexpr mat<T> operator()(const mat<T>&& t) const noexcept
            {
                return { FWD(t).raw_base() };
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::as_mat_fn as_mat{};
    }

    template<class T>
    struct mat_ns::mat : based_on<T>
    {
        static constexpr size_t nc = mat_shape<T>[0];
        static constexpr size_t nr = mat_shape<T>[1];
        static constexpr size_t ne = nc * nr;

        template<size_t I, unwarp_derived_from<mat> Self>
        friend constexpr decltype(auto) tree_get(Self&& self)
        {
            return tree_get<I>(FWD(self).unwrap_base()) | as_vec;
        }

        template<class U, class Self> requires (mat_shape<T> == mat_shape<U>)
        explicit constexpr operator mat<U>(this Self&& self)
        {
            return mat<U>{ FWD(self).raw_base() | make<U> };
        }

        constexpr decltype(auto) operator[](this auto&& self, size_t i)
        noexcept(noexcept(FWD(self).raw_base()[i]))
        requires requires{FWD(self).raw_base()[i];}
        {
            return FWD(self).raw_base()[i];
        }

        template<size_t R, class Self> requires (R < nr)
        constexpr decltype(auto) r(this Self&& self)
        {
            return FWD(self).raw_base() | refer | component<R, 1uz> | as_vec;
        }

        template<size_t C, class Self> requires (C < nc)
        constexpr decltype(auto) c(this Self&& self)
        {
            return tree_get<C>(FWD(self).raw_base() | refer) | take<nc> | as_vec;
        }

        template<size_t R, size_t C, class Self> requires (R < nr) && (C < nc)
        constexpr decltype(auto) rc(this Self&& self)
        {
            return FWD(self).raw_base() | refer | subtree<C, R>;
        }

        template<size_t C, size_t R, class Self> requires (R < nr) && (C < nc)
        constexpr decltype(auto) cr(this Self&& self)
        {
            return FWD(self).raw_base() | refer | subtree<C, R>;
        }

        template<class...E> requires (sizeof...(E) == ne)
        static constexpr mat from_rows(E&&...elements)
        {
            const auto get_column = [elements_pack = fwd_as_tuple(FWD(elements)...)]<size_t C, size_t...R>(std::index_sequence<R...>)
            {
                return fwd_as_tuple(get<C + R * nc>(std::move(elements_pack))...);
            };

            return [&]<size_t...C>(std::index_sequence<C...>)
            {
                return mat{ make_tuple(get_column.template operator()<C>(std::make_index_sequence<nr>{})...) | make<T> };
            }(std::make_index_sequence<nc>{});
        }

        template<class...U> requires (sizeof...(U) == nr)
        static constexpr mat from_rows(vec<U>&&...rows)
        {
            return mat{ fwd_as_tuple(FWD(rows)...) | transpose<> | make<T> };
        }

        template<class...E> requires (sizeof...(E) == ne)
        static constexpr mat from_columns(E&&...elements)
        {
            const auto get_column = [elements_pack = fwd_as_tuple(FWD(elements)...)]<size_t C, size_t...R>(std::index_sequence<R...>)
            {
                return fwd_as_tuple(get<C * nr + R>(std::move(elements_pack))...);
            };

            return [&]<size_t...C>(std::index_sequence<C...>)
            {
                return mat{ make_tuple(get_column.template operator()<C>(std::make_index_sequence<nr>{})...) | make<T> };
            }(std::make_index_sequence<nc>{});
        }

        template<class...U> requires (sizeof...(U) == nr)
        static constexpr mat from_columns(vec<U>&&...rows)
        {
            return mat{ fwd_as_tuple(FWD(rows)...) | make<T> };
        }

        template<class U> requires (nc == 3uz) && (nr == 3uz) && (size<U> == 2uz)
        static constexpr mat translate(const vec<U>& position) 
        {
            using namespace constants;
            return from_rows(
                _1, _0, position.x(),
                _0, _1, position.y(),
                _0, _0, _1
            );
        }

        //this "(m != 3) is a workround for msvc"
        template<class U> requires (nc != 3uz) && (nc == 4uz) && (nr == 4uz) && (size<U> == 3uz)
        static constexpr mat translate(const vec<U>& position)
        {
            using namespace constants;
            return from_rows(
                _1, _0, _0, position.x(),
                _0, _1, _0, position.y(),
                _0, _0, _1, position.z(),
                _0, _0, _0, _1
            );
        }

        template<class U> requires (nc == 3uz) && (nr == 3uz)
        static constexpr mat rotate(const U& angle)
        {
            const auto cos_v = cos(angle);
            const auto sin_v = sin(angle);
            using namespace constants;
            return from_rows(
                cos_v, -sin_v, _0,
                sin_v, cos_v, _0,
                _0, _0, _1
            );
        }

        template<class U> requires (nc == 4uz) && (nr == 4uz)
        static constexpr mat rotate_x(const U& angle)
        {
            const auto cos_v = cos(angle);
            const auto sin_v = sin(angle);
            using namespace constants;
            return from_rows(
                _1, _0, _0, _0,
                _0, cos_v, -sin_v, _0,
                _0, sin_v, cos_v, _0,
                _0, _0, _0, _1
            );
        }

        template<class U> requires (nc == 4uz) && (nr == 4uz)
        static constexpr mat rotate_y(const U& angle)
        {
            const auto cos_v = cos(angle);
            const auto sin_v = sin(angle);
            using namespace constants;
            return from_rows(
                cos_v, _0, -sin_v, _0,
                _0, _1, _0, _0,
                sin_v, _0, cos_v, _0,
                _0, _0, _0, _1
            );
        }

        template<class U> requires (nc == 4uz) && (nr == 4uz)
        static constexpr mat rotate_z(const U& angle)
        {
            const auto cos_v = cos(angle);
            const auto sin_v = sin(angle);
            using namespace constants;
            return from_rows(
                cos_v, -sin_v, _0, _0,
                sin_v, cos_v, _0, _0,
                _0, _0, _1, _0,
                _0, _0, _0, _1
            );
        }

        template<class U> requires (nc == 4uz) && (nr == 4uz) && (size<U> == 3uz)
        static constexpr mat rotate_xyz(const vec<U>& angle)
        {
            return rotate_z(angle.z()) * rotate_y(angle.y()) * rotate_x(angle.x());
        }

        template<class U> requires (nc == 3uz) && (nr == 3uz) && (size<U> == 2uz)
        static constexpr mat scale(const vec<U>& scale)
        {
            using namespace constants;
            return from_rows(
                scale.x(), _0, _0,
                _0, scale.y(), _0,
                _0, _0, _1
            );
        }

    };

    template<class T>
    struct tree_size<mat<T>>
    {
        static constexpr size_t value = size<T>;
    };
}

namespace senluo::mat_ns 
{
    template<class LHS, class RHS>
    constexpr auto plus(const mat<LHS>& lhs, const mat<RHS>& rhs)
    {
        if constexpr(std::same_as<LHS, RHS> && requires{ mat_zip_transform(std::plus<>{}, lhs.raw_base(), rhs.raw_base()) | make<LHS>; })
        {
            return mat<LHS>{ mat_zip_transform(std::plus<>{}, lhs.raw_base(), rhs.raw_base()) | make<LHS> };
        }
        else
        {
            using lazy_t = decltype(mat_zip_transform(std::plus<>{}, lhs.raw_base(), rhs.raw_base()));
            using container_t = default_mat_container_t<lazy_t>;
            return mat<container_t>{ zip_transform(std::plus<>{}, lhs.raw_base(), rhs.raw_base()) | make<container_t> };
        }
    }

    template<class RHS, class LHS>
    constexpr auto swap_plus(const mat<RHS>& rhs, const mat<LHS>& lhs)
    {
        return plus(lhs, rhs);
    }

    template<class LHS, class RHS>
    constexpr auto minus(const mat<LHS>& lhs, const mat<RHS>& rhs)
    {
        if constexpr(std::same_as<LHS, RHS> && requires{ mat_zip_transform(std::minus<>{}, lhs.raw_base(), rhs.raw_base()) | make<LHS>; })
        {
            return mat<LHS>{ mat_zip_transform(std::minus<>{}, lhs.raw_base(), rhs.raw_base()) | make<LHS> };
        }
        else
        {
            using lazy_t = decltype(mat_zip_transform(std::minus<>{}, lhs.raw_base(), rhs.raw_base()));
            using container_t = default_mat_container_t<lazy_t>;
            return mat<container_t>{ zip_transform(std::minus<>{}, lhs.raw_base(), rhs.raw_base()) | make<container_t> };
        }
    }

    template<class RHS, class LHS>
    constexpr auto swap_minus(const mat<RHS>& rhs, const mat<LHS>& lhs)
    {
        return minus(lhs, rhs);
    }

    template<class LHS, class RHS>
    constexpr auto multiplies(const mat<LHS>& lhs, const mat<RHS>& rhs)
    {
        if constexpr(std::same_as<LHS, RHS> && requires{ lazy_mat_mul(lhs.base(), rhs.base()) | make<LHS>; })
        {
            return mat<LHS>{ lazy_mat_mul(lhs.base(), rhs.base()) | make<LHS> };
        }
        else
        {
            using lazy_t = decltype(lazy_mat_mul(lhs.base(), rhs.base()));
            using container_t = default_mat_container_t<lazy_t>;
            return mat<container_t>{ lazy_mat_mul(lhs.base(), rhs.base()) | make<container_t> };
        }
    }

    template<class RHS, class LHS>
    constexpr auto swap_multiplies(const mat<RHS>& rhs, const mat<LHS>& lhs)
    {
        return multiplies(lhs, rhs);
    }

    template<class T>
    constexpr auto negate(const mat<T>& m)
    {
        if constexpr(requires{ m.raw_ase() | mat_transform(std::negate<>{}) | make<T>; })
        {
            return mat<T>{ m.raw_base() | mat_transform(std::negate<>{}) | make<T> };
        }
        else
        {
            using lazy_t = decltype(m.raw_base() | mat_transform(std::negate<>{}));
            using container_t = default_mat_container_t<lazy_t>;
            return mat<container_t>{ m.raw_base() | mat_transform(std::negate<>{}) | make<T> };
        }
    }

    template<class LHS, class RHS>
    constexpr auto multiplies(const mat<LHS>& lhs, const vec<RHS>& rhs)
    {
        if constexpr(std::same_as<LHS, RHS> && requires{ lazy_mat_mul_vec(lhs.raw_base(), rhs.raw_base()) | make<LHS>; })
        {
            return vec<LHS>{ lazy_mat_mul_vec(lhs.raw_base(), rhs.raw_base()) | make<LHS> };
        }
        else
        {
            using lazy_t = decltype(lazy_mat_mul_vec(lhs.raw_base(), rhs.raw_base()));
            using container_t = default_vec_container_t<lazy_t>;
            return vec<container_t>{ lazy_mat_mul_vec(lhs.raw_base(), rhs.raw_base()) | make<container_t> };
        }
    }

    template<class RHS, class LHS>
    constexpr auto swap_multiplies(const vec<RHS>& rhs, const mat<LHS>& lhs)
    {
        return multiplies(lhs, rhs);
    }

    template<class LHS, class RHS>
    constexpr auto multiplies(const LHS& lhs, const mat<RHS>& rhs)
    {
        if constexpr(requires{ rhs.raw_base() | mat_transform([&](const auto& e){ return lhs * e; }) | make<RHS>; })
        {
            return mat<RHS>{ rhs.raw_base() | mat_transform([&](const auto& e){ return lhs * e; }) | make<RHS> };
        }
        else
        {
            using lazy_t = decltype(rhs.raw_base() | mat_transform([&](const auto& e){ return lhs * e; }));
            using container_t = default_mat_container_t<lazy_t>;
            return mat<container_t>{ rhs.raw_base() | mat_transform([&](const auto& e){ return lhs * e; }) | make<container_t> };
        }
    }

    template<class RHS, class LHS>
    constexpr auto swap_multiplies(const mat<RHS>& rhs, const LHS& lhs)
    {
        return multiplies(lhs, rhs);
    }

    template<class LHS, class RHS>
    constexpr auto multiplies(const mat<LHS>& lhs, const RHS& rhs)
    {
        return multiplies(rhs, lhs);
    }

    template<class RHS, class LHS>
    constexpr auto swap_multiplies(const RHS& rhs, const mat<LHS>& lhs)
    {
        return multiplies(lhs, rhs);
    }

    template<class LHS, class RHS>
    constexpr auto divides(const mat<LHS>& lhs, const RHS& rhs)
    {
        if constexpr(requires{ lhs.raw_base() | mat_transform([&](const auto& e){ return e / rhs; }) | make<LHS>; })
        {
            return mat<LHS>{ lhs.raw_base() | mat_transform([&](const auto& e){ return e / rhs; }) | make<LHS> };
        }
        else
        {
            using lazy_t = decltype(lhs.raw_base() | mat_transform([&](const auto& e){ return e / rhs; }));
            using container_t = default_mat_container_t<lazy_t>;
            return mat<container_t>{ lhs.raw_base() | mat_transform([&](const auto& e){ return e / rhs; }) | make<container_t> };
        }
    }

    template<class RHS, class LHS>
    constexpr auto swap_divides(const RHS& rhs, const mat<LHS>& lhs)
    {
        return divides(lhs, rhs);
    }
    
    template<class LHS, class RHS>
    constexpr decltype(auto) operator+(const mat<LHS>& lhs, const mat<RHS>& rhs)
    {
        return plus(lhs, rhs);
    }

    template<class LHS, class RHS>
    constexpr decltype(auto) operator-(const mat<LHS>& lhs, const mat<RHS>& rhs)
    {
        return minus(lhs, rhs);
    }

    template<class LHS, class RHS>
    constexpr decltype(auto) operator*(const mat<LHS>& lhs, const mat<RHS>& rhs)
    {
        return multiplies(lhs, rhs);
    }

    template<class T>
    constexpr decltype(auto) operator-(const mat<T>& m)
    {
        return negate(m);
    }

    template<class LHS, class RHS>
    constexpr decltype(auto) operator*(const mat<LHS>& lhs, const vec<RHS>& rhs)
    {
        return multiplies(lhs, rhs);
    }

    template<class LHS, class RHS>
    constexpr decltype(auto) operator*(const LHS& lhs, const mat<RHS>& rhs)
    {
        return multiplies(lhs, rhs);
    }

    template<class LHS, class RHS>
    constexpr decltype(auto) operator*(const mat<LHS>& lhs, const RHS& rhs)
    {
        return multiplies(lhs, rhs);
    }

    template<class LHS, class RHS>
    constexpr decltype(auto) operator/(const mat<LHS>& lhs, const RHS& rhs)
    {
        return divides(lhs, rhs);
    }
}

namespace senluo
{
    
}

#include "tools/macro_undef.hpp"
#endif // SENLUO_MAT_HPP
