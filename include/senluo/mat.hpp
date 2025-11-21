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
    template<class T>
    struct default_mat_container
    {
        template<size_t I>
        static consteval auto get_column_type_tag()
        {
            return []<size_t...J>(std::index_sequence<J...>)
            {
                return std::type_identity<default_container_t<subtree_t<T, I, J>...>>{};
            }(std::make_index_sequence<mat_shape<T>[1uz]>{});
        }

        template<size_t I>
        using column_type = decltype(get_column_type_tag<I>())::type; 

        static consteval auto get_type_tag()
        {
            return []<size_t...I>(std::index_sequence<I...>)
            {
                return std::type_identity<default_container_t<column_type<I>...>>{};
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

    template<class T>
    concept mat_wrapped = requires(std::remove_cvref_t<T>& t)
    {
        { []<class U>(mat<U>&)->mat<U>*{}(t) } -> std::same_as<std::remove_cvref_t<T>*>;
    };

    namespace detail
    {
        struct as_mat_fn : adaptor_closure<as_mat_fn>
        {
            template<class T>
            constexpr mat<pass_t<T>> operator()(T&& t) const noexcept(noexcept(pass(FWD(t))))
            {
                return { FWD(t) };
            }

            template<class T>
            constexpr mat<T&> operator()(mat<T>& t) const noexcept
            {
                return { t.base };
            }

            template<class T>
            constexpr mat<const T&> operator()(const mat<T>& t) const noexcept
            {
                return { t.base };
            }

            template<class T>
            constexpr mat<T> operator()(mat<T>&& t) const noexcept(std::is_nothrow_move_constructible_v<mat<T>>)
            {
                return std::move(t);
            }

            template<class T>
            constexpr mat<T> operator()(const mat<T>&& t) const noexcept(std::is_nothrow_copy_constructible_v<mat<T>>)
            {
                return t;
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::as_mat_fn as_mat{};
    }

    template<class T>
    struct mat_ns::mat
    {
        using base_type = T;
        static constexpr size_t nc = mat_shape<T>[0];
        static constexpr size_t nr = mat_shape<T>[1];
        static constexpr size_t ne = nc * nr;
        
        T base;
        
        static consteval bool is_elementary(custom_t = {}) noexcept
        {
            return false;
        }

        template<size_t I, class Self>
        constexpr decltype(auto) tree_get(this Self&& self, custom_t = {})
        {
            return senluo::tree_get<I>(FWD(self, base)) | take<nr> | as_vec;
        }

        static consteval size_t get_size(custom_t = {}) noexcept
        {
            return size<T>;
        }

        template<class U>
        static constexpr mat make_from(U&& u, custom_t = {})
        AS_EXPRESSION(
            mat<T>{ FWD(u) | make<T> }
        )

        template<class U, class Self> requires (mat_shape<T> == mat_shape<U>)
        explicit constexpr operator mat<U>(this Self&& self)
        {
            return mat<U>{ FWD(self, base) | make<U> };
        }

        template<mat_wrapped U> requires (mat_shape<T> == mat_shape<U>)
        constexpr mat& operator=(U&& src)&
        {
            [&]<size_t...I>(std::index_sequence<I...>)
            {
                (..., (this->r<I>() = FWD(src).template r<I>()));
            }(std::make_index_sequence<size<T>>{});
            return *this;
        }

        template<size_t R, class Self> requires (R < nr)
        constexpr decltype(auto) r(this Self&& self)
        {
            return FWD(self, base) | component<R, 1uz> | as_vec;
        }

        template<size_t C, class Self> requires (C < nc)
        constexpr decltype(auto) c(this Self&& self)
        {
            return senluo::tree_get<C>(FWD(self, base)) | take<nr> | as_vec;
        }

        template<size_t R, size_t C, class Self> requires (R < nr) && (C < nc)
        constexpr decltype(auto) rc(this Self&& self)
        {
            return FWD(self, base) | subtree<C, R>;
        }

        template<size_t C, size_t R, class Self> requires (R < nr) && (C < nc)
        constexpr decltype(auto) cr(this Self&& self)
        {
            return FWD(self, base) | subtree<C, R>;
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
}

namespace senluo::detail
{
    template<class T, class Lazy>
    constexpr auto to_mat(Lazy&& lazy_val)
    {
        using base_t = std::remove_cvref_t<typename std::remove_cvref_t<T>::base_type>;
        if constexpr(requires{ FWD(lazy_val) | make<base_t>; })
        {
            return mat<base_t>{ FWD(lazy_val) | make<base_t> };
        }
        else
        {
            using container_t = default_vec_container_t<Lazy>;
            return mat<container_t>{ FWD(lazy_val) | make<container_t> };
        }
    }

    template<class V, class M>
    struct vec_mul_mat_tree
    {
        V vec;
        M mat;
        
        template<size_t I, class Self>
        constexpr decltype(auto) tree_get(this Self&& self, custom_t = {})
        {
            return dot(FWD(self, vec), senluo::tree_get<I>(FWD(self, mat)));
        }

        static consteval size_t get_size(custom_t = {}) noexcept
        {
            return size<M>;
        }
    };

    template<class L, class R>
    struct mat_mul_tree
    {
        L lhs;
        R rhs;

        template<size_t I, class Self>
        constexpr decltype(auto) tree_get(this Self&& self, custom_t = {})
        {
            return vec_mul_mat_tree<decltype(FWD(self, rhs) | subtree<I>), decltype(FWD(self, lhs) | transpose<>)>{
                senluo::tree_get<I>(FWD(self, rhs)),
                FWD(self, lhs) | transpose<>
            };
        }

        static consteval size_t get_size(custom_t = {}) noexcept
        {
            return size<L>;
        }
    };
    
    struct lazy_vec_mul_mat_fn : adaptor<lazy_vec_mul_mat_fn>
    {
        template<class M, class V>
        static constexpr decltype(auto) adapt(M&& mat, V&& vec)
        {
            return vec_mul_mat_tree<pass_t<M>, decltype(detail::astrict_unchecked<stricture_t::readonly>(FWD(vec)))>
            {
                FWD(mat),
                detail::astrict_unchecked<stricture_t::readonly>(FWD(vec))
            };
        }
    };

    struct lazy_mat_mul_fn : adaptor<lazy_mat_mul_fn>
    {
        template<class L, class R>
        static constexpr auto adapt(L&& l, R&& r)
        {
            return mat_mul_tree<decltype(detail::astrict_unchecked<stricture_t::readonly>(FWD(l))),
                                decltype(detail::astrict_unchecked<stricture_t::readonly>(FWD(r)))>
            {
                detail::astrict_unchecked<stricture_t::readonly>(FWD(l)),
                detail::astrict_unchecked<stricture_t::readonly>(FWD(r))
            };
        }
    };
}

namespace senluo
{   
    inline namespace functors
    {
        inline constexpr detail::lazy_vec_mul_mat_fn lazy_vec_mul_mat{};
        inline constexpr detail::lazy_mat_mul_fn lazy_mat_mul{};
    }
}

namespace senluo::mat_ns
{    
    template<class LHS, class RHS>
    constexpr decltype(auto) operator+(LHS&& lhs, RHS&& rhs)
    {
        return detail::to_mat<LHS>(mat_zip_transform(std::plus<>{}, FWD(lhs), FWD(rhs)));
    }

    template<class LHS, class RHS>
    constexpr decltype(auto) operator-(LHS&& lhs, RHS&& rhs)
    {
        return detail::to_mat<LHS>(mat_zip_transform(std::minus<>{}, FWD(lhs), FWD(rhs)));
    }

    template<mat_wrapped LHS, mat_wrapped RHS>
    constexpr decltype(auto) operator*(LHS&& lhs, RHS&& rhs)
    {
        return detail::to_mat<LHS>(lazy_mat_mul(FWD(lhs), FWD(rhs)));
    }    

    template<class M>
    constexpr decltype(auto) operator-(M&& m)
    {
        return detail::to_mat<M>(mat_transform(FWD(m), std::negate<>{}));
    }

    template<mat_wrapped M, vec_wrapped V>
    constexpr decltype(auto) operator*(M&& m, V&& v)
    {
        return detail::to_vec<V>(lazy_vec_mul_mat(FWD(v), FWD(m) | transpose<>));
    }

    template<vec_wrapped V, mat_wrapped M>
    constexpr decltype(auto) operator*(V&& v, M&& m)
    {
        return detail::to_vec<V>(lazy_vec_mul_mat(FWD(v), FWD(m)));
    }

    template<mat_wrapped M, class T>
    constexpr decltype(auto) operator*(M&& m, T&& t)
    {
        return detail::to_mat<M>(mat_transform(FWD(m), [t = pass(FWD(t))](auto&& e){ return e * t; }));
    }

    template<class T, mat_wrapped M>
    constexpr decltype(auto) operator*(T&& t, M&& m)
    {
        return detail::to_mat<M>(mat_transform(FWD(m), [t = pass(FWD(t))](auto&& e){ return t * e; }));
    }

    template<mat_wrapped M, class T>
    constexpr decltype(auto) operator/(M&& m, T&& t)
    {
        return detail::to_mat<M>(mat_transform(FWD(m), [t = pass(FWD(t))](auto&& e){ return e / t; }));
    }
}

#include "tools/macro_undef.hpp"
#endif // SENLUO_MAT_HPP
