#ifndef SENLUO_PRINCIPLE_HPP
#define SENLUO_PRINCIPLE_HPP

#include "../tools/general.hpp"
#include "tree.hpp"


#include "../tools/macro_define.hpp"

namespace senluo
{
    template<class T>
    struct plain_principle;
    
    template<class T>
    struct trivial_principle;

    namespace detail
    {
        template<auto OperationTree>
        struct operate_fn;
    }

    inline namespace functors
    {
        template<auto OperationTree>
        inline constexpr detail::operate_fn<OperationTree> operate{};
    }

    namespace detail::principle_ns
    {
        void principle() = delete;

        struct principle_fn
        {
            template<class T>
            static consteval bool is_plain()
            {
                if constexpr(terminal<T> 
                    && (not requires{ std::declval<T>().template principle<T>(custom_t{}); })
                    && (not requires{ principle<T>(std::declval<T>(), custom_t{}); })
                )
                {
                    return true;
                }
                else return []<size_t...I>(std::index_sequence<I...>)
                {
                    return (... && is_plain<tree_get_t<I, T>>());
                }(std::make_index_sequence<size<T>>{});
            }

            template<class T>
            static constexpr decltype(auto) operator()(T&& t)
            {
                using utype = ideal_unwrap_t<T>;
                using type = std::remove_cvref_t<utype>;
                if constexpr(requires{ unwrap_fwd(FWD(t)).template principle<utype>(custom_t{}); })
                {
                    return unwrap_fwd(FWD(t)).template principle<utype>(custom_t{});
                }
                else if constexpr(requires{ principle<utype>(unwrap_fwd(FWD(t)), custom_t{}); })
                {
                    return principle<utype>(unwrap_fwd(FWD(t)), custom_t{});
                }
                else if constexpr(is_plain<T>())
                {
                    return plain_principle<utype>{ unwrap_fwd(FWD(t)) };
                }
                else
                {
                    return trivial_principle<utype>{ unwrap_fwd(FWD(t)) };
                }
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::principle_ns::principle_fn principle{};
    }

    template<class T>
    using principle_t = decltype(principle(std::declval<T>()));

    template<class T>
    concept plain = std::same_as<principle_t<T>, plain_principle<ideal_unwrap_t<T>>>;

    struct null_principle
    {
        constexpr std::in_place_t data()&& noexcept
        {
            return std::in_place_t{};
        }
        
        static consteval detail::pass_fn adaptor_closure()
        {
            return {};
        }
    };

    template<class T>
    struct plain_principle
    {
        T&& value;

        constexpr T data()&& noexcept
        {
            return (T&&)value;
        }
        
        static consteval detail::pass_fn adaptor_closure()
        {
            return {};
        }
    };

    template<class T>
    struct trivial_principle
    {        
        T&& value_;

        constexpr decltype(auto) value()
        {
            if constexpr(std::is_rvalue_reference_v<T>)
            {
                return wrapper<T>{ (T)value_ };
            }
            else
            {
                return (T&&)value_;
            }
        }

        constexpr auto data()&&
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                return tuple<decltype(principle(value() || tree_get<I>).data())...>
                {
                    principle(value() || tree_get<I>).data()...
                };
            }(std::make_index_sequence<size<T>>{});
        }

        static consteval auto adaptor_closure()
        {
            return []<size_t...I>(std::index_sequence<I...>)
            {
                return operate<tuple{
                    decltype(principle(std::declval<trivial_principle>().value() || tree_get<I>))::adaptor_closure()...
                }>;
            }(std::make_index_sequence<size<T>>{});
        }
    };
}

#include "../tools/macro_undef.hpp"
#endif