#ifndef SENLUO_WRAP_HPP
#define SENLUO_WRAP_HPP

#include "../tools/adaptor.hpp"

#include "../tools/macro_define.hpp"

namespace senluo
{
    template<class T>
    struct wrapper;

    template<class T>
    struct based_on
    {
        T base_;
        
        constexpr auto&& raw_base(this auto&& self) noexcept
        {
            return FWD(self, base_);
        }

        template<class Self>
        constexpr decltype(auto) base(this Self&& self) noexcept
        {
            if constexpr(std::is_rvalue_reference_v<Self&&> && std::is_rvalue_reference_v<T>)
            {
                return wrapper<decltype(FWD(self).raw_base())>{ FWD(self).raw_base() };
            }
            else
            {
                return FWD(self).raw_base();
            }
        }

        template<class Self>
        constexpr decltype(auto) unwrap_base(this Self&& self) noexcept
        {
            return FWD(self).base();
        }
    };

    template<class T>
    struct wrapper : based_on<T>
    {
        template<class Self>
        constexpr decltype(auto) unwrap_base(this Self&& self) noexcept
        {
            if constexpr(std::is_rvalue_reference_v<decltype(FWD(self).raw_base().raw_base())>
                && (std::is_rvalue_reference_v<T> || std::is_rvalue_reference_v<decltype(FWD(self).raw_base().base_)>)
            )
            {
                return wrapper<decltype(FWD(self).raw_base().raw_base())>{ FWD(self).raw_base().raw_base() };
            }
            else{
                return FWD(self).raw_base().raw_base();
            }
        }
    };

    template<class T>
    wrapper(T) -> wrapper<T>;

    namespace detail 
    {
        struct wrap_fn : adaptor_closure<wrap_fn>
        {
            template<class T>
            constexpr wrapper<T> operator()(T&& t) const
            {
                return { FWD(t) };
            }
        };

        struct refer_fn : adaptor_closure<refer_fn>
        {
            template<class T>
            constexpr T& operator()(T& t) const
            {
                return t;
            }

            template<class T>
            constexpr wrapper<T&&> operator()(T&& t) const
            {
                return { FWD(t) };
            }
        };

        struct unwrap_fn : adaptor_closure<unwrap_fn>
        {
            template<class T>
            constexpr T& operator()(wrapper<T>& t) const
            {
                return t.raw_base();
            }

            template<class T>
            constexpr const T& operator()(const wrapper<T>& t) const
            {
                return t.raw_base();
            }

            template<class T>
            constexpr decltype(auto) operator()(wrapper<T>&& t) const
            {
                return (T)FWD(t).raw_base();
            }

            template<class T>
            constexpr decltype(auto) operator()(const wrapper<T>&& t) const
            {
                return (const T)FWD(t).raw_base();
            }

            template<class T>
            constexpr decltype(auto) operator()(T&& t) const
            {
                return (T)FWD(t);
            }
        };

        struct unwrap_fwd_fn : adaptor_closure<unwrap_fwd_fn>
        {
            template<class T>
            constexpr T& operator()(wrapper<T>& t) const
            {
                return t.raw_base();
            }

            template<class T>
            constexpr const T& operator()(const wrapper<T>& t) const
            {
                return t.raw_base();
            }

            template<class T>
            constexpr T&& operator()(wrapper<T>&& t) const
            {
                return FWD(t).raw_base();
            }

            template<class T>
            constexpr const T&& operator()(const wrapper<T>&& t) const
            {
                return FWD(t).raw_base();
            }

            template<class T>
            constexpr T&& operator()(T&& t) const
            {
                return FWD(t);
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::wrap_fn wrap{};

        inline constexpr detail::refer_fn refer{};

        inline constexpr detail::unwrap_fn unwrap{};
        
        inline constexpr detail::unwrap_fwd_fn unwrap_fwd{};
    }

    template<class T>
    using unwrap_t = decltype(unwrap(std::declval<T>()));

    template<class T>
    using unwrap_fwd_t = decltype(unwrap_fwd(std::declval<T>()));

    template<class T>
    concept wrapped = requires(std::remove_cvref_t<T>& t)
    {
        { []<class V>(wrapper<V>&)->wrapper<V>*{ return nullptr; }(t) } -> std::same_as<std::remove_cvref_t<T>*>;
    };

    namespace detail 
    {
        template<class T>
        constexpr auto ideal_unwrap_t_tag()
        {
            if constexpr(not wrapped<T>)
            {
                return type_tag<std::remove_const_t<T>>{};
            }
            else if constexpr(std::is_object_v<T> && requires{ requires std::is_object_v<decltype(std::declval<T>().base_)>; })
            {
                return type_tag<std::remove_const_t<decltype(std::declval<T>().base_)>>{};
            }
            else
            {
                return type_tag<decltype(FWD(std::declval<T>(), base_))>{};
            }
        }
    }
    
    template<class T>
    using ideal_unwrap_t = decltype(detail::ideal_unwrap_t_tag<T>())::type;

    // namespace detail 
    // {
    //     struct unwrap_base_fn : adaptor_closure<unwrap_base_fn>
    //     {
    //         template<class T>
    //         constexpr decltype(auto) operator()(T&& t) const noexcept
    //         {
    //             if constexpr(std::is_object_v<unwrap_t<T>> && std::is_object_v<decltype(unwrap_fwd(FWD(t)).base_)>)
    //             {
    //                 return unwrap_fwd(FWD(t)).raw_base();
    //             }
    //             else
    //             {
    //                 return unwrap_fwd(FWD(t)).raw_base() | refer;
    //             }
    //         }
    //     };
    // }

    // inline namespace functors
    // {
    //     inline constexpr detail::unwrap_base_fn unwrap_base{};
    // }

    template<class T, class U>
    concept unwarp_derived_from = derived_from<ideal_unwrap_t<T>, U>;
}

#include "../tools/macro_undef.hpp"
#endif // SENLUO_SUBTREE_HPP