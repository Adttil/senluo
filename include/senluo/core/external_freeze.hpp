#ifndef SENLUO_EXTERNAL_FREEZE_HPP
#define SENLUO_EXTERNAL_FREEZE_HPP

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

#include "../tools/general.hpp"

namespace senluo
{
    enum class writability{
        none, // can't arrive any writable state. e.g. nullptr_t, std::monostate, std::tuple<const int, const int&>
        self, // can only arrive self contained writable state. e.g. int, std::string, const int*, std::span<const int>
        external // maybe can arrive to external writable state. e.g. int*, std::span<int>, or unknown type
    };

    constexpr writability operator&(writability l, writability r) noexcept
    {
        using enum writability;

        if (l == external || r == external)
        {
            return external;
        }
        else if (l == self || r == self)
        {
            return self;
        }
        else
        {
            return none;
        }
    }

    template<class T>
    inline constexpr auto writability_through_object = []()
    {
        if constexpr (std::is_same_v<T, std::nullptr_t> || (std::is_aggregate_v<T> && std::is_empty_v<T>)) 
        {
            return writability::none;
        }
        else if constexpr (std::is_arithmetic_v<T> || std::is_enum_v<T>)
        {
            return writability::self;
        }
        else if constexpr(std::is_bounded_array_v<T>)
        {
            return writability_through_object<std::remove_const_t<std::remove_extent_t<T>>>;
        }
        else if constexpr (std::is_pointer_v<T>)
        {
            using element_type = std::remove_pointer_t<T>;
            using type = std::remove_cvref_t<element_type>;
        
            constexpr auto mode = writability_through_object<type>;
        
            if constexpr ((mode == writability::self && std::is_const_v<element_type>) ||
                           mode == writability::none)
            {
                return writability::self;
            }
            else
            {
                return writability::external;
            }
        }
        else 
        {
            return writability::external;
        }
    }();

    template<class... T>
    inline constexpr writability writability_through =
        (writability::none & ... & writability_through<T>);

    template<class T>
    inline constexpr writability writability_through<T> = []()
    {
        using type = std::remove_cvref_t<T>;
        constexpr auto mode = writability_through_object<type>;
        if constexpr (std::is_const_v<std::remove_reference_t<T>>)
        {
            if constexpr (mode == writability::external)
            {
                return writability::external;
            }
            else
            {
                return writability::none;
            }
        }
        else if constexpr (std::is_object_v<T>) 
        {
            return mode;
        }
        else if constexpr (mode == writability::none) 
        {
            return writability::none;
        }
        else
        {
            return writability::external;
        }
    }();

    template<class T>
    concept external_frozen = (writability_through<T> != writability::external);
}

namespace senluo
{
    // You've got this because the lib don't know how to convert T to a external_frozen type.
    // Maybe you should customize writability_through_object or external_freeze.
    template<class T>
    struct unknown_external_frozen_type_of
    {
        using source_type = T;
    };

    template<class T>
    inline constexpr bool is_unknown_external_frozen_type = false;

    template<class T>
    inline constexpr bool is_unknown_external_frozen_type<unknown_external_frozen_type_of<T>> = true;

    namespace detail::external_freeze_ns
    {
        void external_freeze();

        enum strategy_t
        {
            none,
            pass,
            as_const,
            adl,
        };

        template<class T>
        inline constexpr bool has_adl_freeze = requires(T&& value)
        {
            { external_freeze((T&&)value) } -> external_frozen;
        };

        template<class T>
        struct external_freeze_fn
        {
        private:
            static constexpr choice_t<strategy_t> choose() noexcept
            {
                if constexpr (std::is_bounded_array_v<T>) 
                {
                    return { strategy_t::none };
                }
                else if constexpr (external_frozen<T>) 
                {
                    if constexpr (std::is_convertible_v<T&&, T>) 
                    {
                        return { pass, noexcept((T)std::declval<T>()) };
                    }
                    else
                    {
                        return { strategy_t::none };
                    }
                }
                else if constexpr (std::is_lvalue_reference_v<T> && external_frozen<const std::remove_reference_t<T>&>) 
                {
                    return { strategy_t::as_const, true };
                }
                else if constexpr (has_adl_freeze<T>) 
                {
                    return { strategy_t::adl, noexcept(external_freeze(std::declval<T>())) };
                }
                else
                {
                    return { strategy_t::none };
                }
            }

            static constexpr choice_t<strategy_t> choice = choose();

        public:
            constexpr decltype(auto) operator()(T&& value) const
            noexcept(choice.nothrow)
            {
                constexpr strategy_t strategy = choice.strategy;
                if constexpr(strategy == strategy_t::pass)
                {
                    return (T)(T&&)value;
                }
                else if constexpr (strategy == strategy_t::as_const) 
                {
                    return std::as_const(value);
                }
                else if constexpr (strategy == strategy_t::adl) 
                {
                    return external_freeze((T&&)value);
                }
                else
                {
                    return unknown_external_frozen_type_of<T>{};
                }
            }
        };

        template<class T> requires (not external_frozen<T&&>)
        struct external_freeze_fn<T&&>
        {
            constexpr decltype(auto) operator()(T&& value) const
            noexcept(noexcept(external_freeze_fn<T&>{}(value)))
            {
                using type = decltype(external_freeze_fn<T&>{}(value));
                if constexpr (is_unknown_external_frozen_type<type>) 
                {
                    return unknown_external_frozen_type_of<T&&>{};
                }
                else if constexpr (std::same_as<type, const T&>) 
                {
                    return (const T&&)external_freeze_fn<T&>{}(value);
                }
                else
                {
                    return external_freeze_fn<T&>{}(value);
                }
            }
        };
    }

    inline namespace functors
    {
        template<class T>
        inline constexpr detail::external_freeze_ns::external_freeze_fn<T> external_freeze{};
    }

    template<class T>
    using external_freeze_t = decltype(external_freeze<T>(std::declval<T>()));

    template<class T>
    concept external_freezable = not is_unknown_external_frozen_type<external_freeze_t<T>>;
}

namespace senluo
{
    namespace detail
    {
        struct invoke_and_external_freeze_fn
        {
            template<class F, class...Args>
            constexpr decltype(auto) operator()(F&& fn, Args&&...args) const
            {
                using result_t = std::invoke_result_t<F, Args...>;

                if constexpr(external_frozen<result_t>)
                {
                    return std::invoke(static_cast<F&&>(fn), static_cast<Args&&>(args)...);
                }
                else
                {
                    return external_freeze<result_t>(
                        std::invoke(static_cast<F&&>(fn), static_cast<Args&&>(args)...)
                    );
                }
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::invoke_and_external_freeze_fn invoke_and_external_freeze{};
    }
}

#endif