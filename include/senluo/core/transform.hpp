#ifndef SENLUO_TRANSFORM_HPP
#define SENLUO_TRANSFORM_HPP

#include "tree.hpp"
#include "relayout.hpp"

#include "../tools/macro_define.hpp"

namespace senluo
{
    namespace detail::transform_ns
    {
        template<class T, class F>
        struct transform_tree
        {
            T base;
            F fn;

            template<size_t I, class Self>
            constexpr decltype(auto) tree_get(this Self&& self, custom_t = {})
            {
                return self.fn(senluo::tree_get<I>(FWD(self, base)));
            }

            static consteval size_t get_size(custom_t = {})
            {
                return size<T>;
            }
        };
    }

    namespace detail
    {
        struct transform_fn : adaptor<transform_fn>
        {
            template<class T, class F>
            static constexpr decltype(auto) adapt(T&& t, F&& fn)
            {
                return detail::transform_ns::transform_tree<pass_t<T>, pass_t<F>>{
                    FWD(t), FWD(fn)
                };
            }
        };

        struct zip_transform_fn
        {
            template<class F, class...T>
            static constexpr decltype(auto) operator()(F&& fn, T&&...t)
            {
                return detail::transform_ns::transform_tree<zip_t<T...>, decltype(apply(FWD(fn)))>{
                    zip(FWD(t)...), apply(FWD(fn))
                };
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::transform_fn transform{};
        inline constexpr detail::zip_transform_fn zip_transform{};
    }

    namespace detail::mat_transform_ns
    {
        template<class T, class F>
        struct mat_transform_tree
        {
            T base;
            F fn;

            template<size_t I, class Self>
            constexpr decltype(auto) tree_get(this Self&& self, custom_t = {})
            {
                return senluo::tree_get<I>(FWD(self, base)) | transform(FWD(self, fn));
            }

            static consteval size_t get_size(custom_t = {})
            {
                return size<T>;
            }
        };
    }

    namespace detail
    {
        struct mat_transform_fn : adaptor<mat_transform_fn>
        {
            template<class T, class F>
            static constexpr decltype(auto) adapt(T&& t, F&& fn)
            {
                return detail::mat_transform_ns::mat_transform_tree<pass_t<T>, pass_t<F>>{
                    FWD(t), FWD(fn)
                };
            }
        };

        struct mat_zip_transform_fn
        {
            template<class F, class...T>
            static constexpr decltype(auto) operator()(F&& fn, T&&...t)
            {
                return detail::mat_transform_ns::mat_transform_tree<mat_zip_t<T...>, decltype(apply(FWD(fn)))>{
                    mat_zip(FWD(t)...), apply(FWD(fn))
                };
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::mat_transform_fn mat_transform{};
        inline constexpr detail::mat_zip_transform_fn mat_zip_transform{};
    }
}

#include "../tools/macro_undef.hpp"
#endif