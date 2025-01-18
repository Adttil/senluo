#ifndef SENLUO_WRAP_HPP
#define SENLUO_WRAP_HPP

#include "../general.hpp"
#include "../tree.hpp"

#include "../macro_define.hpp"

namespace senluo 
{
    template<class T>
    struct transition_wrapper;

    template<class T>
    struct standard_interface
    {
        template<class Self>
        constexpr transition_wrapper<Self&&> operator+(this Self&& self)
        {
            return { FWD(self) };
        }
    };

    template<class T>
    struct based_on
    {
        SENLUO(no_unique_address) T base;
    };

    template<class T>
    struct wrapper : based_on<T>, standard_interface<wrapper<T>>
    {
        template<size_t I, class Self>
        constexpr decltype(auto) get(this Self&& self)
        {
            if constexpr(I < size<T>)
            {
                return subtree<I>(FWD(self, base));
            }
            else
            {
                return end();
            } 
        }

        template<class Self>
        constexpr explicit operator std::decay_t<T>(this Self&& self)
        {
            return FWD(self, base);
        }

        template<typename To, class Self> requires std::same_as<decltype(FWD(std::declval<Self>(), base)), To&>
        constexpr explicit operator To&(this Self&& self)
        {
            return FWD(self, base);
        }

        template<typename To, class Self> requires std::same_as<decltype(FWD(std::declval<Self>(), base)), To&&>
        constexpr explicit operator To&&(this Self&& self)
        {
            return FWD(self, base);
        }
    };

    template<class T>
    wrapper(T) -> wrapper<T>;

    template<class T>
    concept wrapped = requires(std::remove_cvref_t<T>& t)
    {
        { []<class V>(wrapper<V>&)->wrapper<V>*{ return nullptr; }(t) } -> std::same_as<std::remove_cvref_t<T>*>;
    };

    namespace detail 
    {
        struct unwrap_fn : adaptor_closure<unwrap_fn>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& tree)const
            {
                if constexpr(not wrapped<T>)
                {
                    //gcc bug.
                    //return T{ FWD(tree) }
                    return (T)FWD(tree);
                }
                else if constexpr(std::is_object_v<T> && requires{ requires std::is_object_v<decltype(tree.base)>; })
                {
                    return decltype(tree.base){ FWD(tree, base) };
                }
                else
                {
                    return FWD(tree, base);
                }
            }
        };

        struct unwrap_fwd_fn : adaptor_closure<unwrap_fwd_fn>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& tree)const
            {
                if constexpr(not wrapped<T>)
                {
                    return FWD(tree);
                }
                else
                {
                    return FWD(tree, base);
                }
            }
        };
    }

    inline constexpr detail::unwrap_fn unwrap{};

    template<class T>
    using unwrap_t = decltype(unwrap(std::declval<T>()));

    inline constexpr detail::unwrap_fwd_fn unwrap_fwd{};

    namespace detail 
    {
        struct wrap_t : adaptor_closure<wrap_t>
        {
            template<class T>
            constexpr wrapper<senluo::unwrap_t<T>> operator()(T&& t) const
            {
                return { unwrap_fwd(FWD(t)) };
            }
        };

        struct refer_t : adaptor_closure<refer_t>
        {
            template<class T>
            constexpr wrapper<senluo::unwrap_t<T>&&> operator()(T&& t) const
            {
                return { unwrap_fwd(FWD(t)) };
            }
        };
    }

    inline constexpr detail::wrap_t wrap{};

    inline constexpr detail::refer_t refer{};

    namespace detail 
    {
        struct base_fn : adaptor_closure<base_fn>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& t) const
            {
                if constexpr(std::is_object_v<unwrap_t<T>> && std::is_object_v<decltype(unwrap_fwd(FWD(t)).base)>)
                {
                    return FWD(unwrap_fwd(FWD(t)), base);
                }
                else
                {
                    return FWD(unwrap_fwd(FWD(t)), base) | refer;
                }
            }
        };

        inline constexpr base_fn base{};
    }

    template<class T, class U>
    concept unwarp_derived_from = derived_from<unwrap_t<T>, U>;
}

#include "../macro_undef.hpp"
#endif