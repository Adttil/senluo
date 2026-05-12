#ifndef SENLUO_SHARED_STATE_FREEZE_HPP
#define SENLUO_SHARED_STATE_FREEZE_HPP

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

#include "../tools/general.hpp"

namespace senluo
{
    enum class mutable_state_sharing_mode{
        no_mutable_state, // can't arrive any mutable state. e.g. nullptr_t, std::monostate, std::tuple<const int, const int&>
        exclusive, // new object only have exclusive mutable state. e.g. int, std::string, const int*, std::span<const int>
        shared // new object maybe have shared mutable state. e.g. int*, std::span<int>
    };

    constexpr mutable_state_sharing_mode operator&(mutable_state_sharing_mode l, mutable_state_sharing_mode r) noexcept
    {
        using enum mutable_state_sharing_mode;

        if (l == shared || r == shared)
        {
            return shared;
        }
        else if (l == exclusive || r == exclusive)
        {
            return exclusive;
        }
        else
        {
            return no_mutable_state;
        }
    }

    template<class T>
    inline constexpr auto mutable_state_sharing_mode_of = []()
    {
        if constexpr (std::is_same_v<T, std::nullptr_t> || (std::is_aggregate_v<T> && std::is_empty_v<T>)) 
        {
            return mutable_state_sharing_mode::no_mutable_state;
        }
        else if constexpr (std::is_arithmetic_v<T> || std::is_enum_v<T>)
        {
            return mutable_state_sharing_mode::exclusive;
        }
        else if constexpr(std::is_bounded_array_v<T>)
        {
            return mutable_state_sharing_mode_of<std::remove_const_t<std::remove_extent_t<T>>>;
        }
        else if constexpr (std::is_pointer_v<T>)
        {
            using element_type = std::remove_pointer_t<T>;
            using type = std::remove_cvref_t<element_type>;
        
            constexpr auto mode = mutable_state_sharing_mode_of<type>;
        
            if constexpr ((mode == mutable_state_sharing_mode::exclusive && std::is_const_v<element_type>) ||
                           mode == mutable_state_sharing_mode::no_mutable_state)
            {
                return mutable_state_sharing_mode::exclusive;
            }
            else
            {
                return mutable_state_sharing_mode::shared;
            }
        }
        else 
        {
            return mutable_state_sharing_mode::shared;
        }
    }();

    template<class... T>
    inline constexpr mutable_state_sharing_mode mutable_state_sharing_mode_of_aggregate =
        (mutable_state_sharing_mode::no_mutable_state & ... & mutable_state_sharing_mode_of_aggregate<T>);

    template<class T>
    inline constexpr mutable_state_sharing_mode mutable_state_sharing_mode_of_aggregate<T> = []()
    {
        using type = std::remove_cvref_t<T>;
        constexpr auto mode = mutable_state_sharing_mode_of<type>;
        if constexpr (std::is_const_v<std::remove_reference_t<T>>)
        {
            if constexpr (mode == mutable_state_sharing_mode::shared)
            {
                return mutable_state_sharing_mode::shared;
            }
            else
            {
                return mutable_state_sharing_mode::no_mutable_state;
            }
        }
        else if constexpr (std::is_object_v<T>) 
        {
            return mode;
        }
        else if constexpr (mode == mutable_state_sharing_mode::no_mutable_state) 
        {
            return mutable_state_sharing_mode::no_mutable_state;
        }
        else
        {
            return mutable_state_sharing_mode::shared;
        }
    }();

    namespace detail
    {
        template<class T>
        consteval bool shared_state_frozen_impl()
        {
            using type = std::remove_cvref_t<T>;
            constexpr auto mode = mutable_state_sharing_mode_of<type>;
            if constexpr (
                std::is_reference_v<T>
                &&
                not std::is_const_v<std::remove_reference_t<T>>) 
            {
                return mode == mutable_state_sharing_mode::no_mutable_state;
            }
            else
            {
                return mode != mutable_state_sharing_mode::shared;
            }
        }
    }

    template<class T>
    concept shared_state_frozen = detail::shared_state_frozen_impl<T>();
}

namespace senluo
{
    // You've got this because the lib don't know how to convert T to a shared_state_frozen type.
    // Maybe you should customize mutable_state_sharing_mode or shared_state_freeze.
    template<class T>
    struct unknown_shared_state_frozen_type_of
    {
        using source_type = T;
    };

    template<class T>
    inline constexpr bool is_unknown_shared_state_frozen_type = false;

    template<class T>
    inline constexpr bool is_unknown_shared_state_frozen_type<unknown_shared_state_frozen_type_of<T>> = true;

    namespace detail::shared_state_freeze_ns
    {
        void shared_state_freeze();

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
            { shared_state_freeze((T&&)value) } -> senluo::shared_state_frozen;
        };

        template<class T>
        struct shared_state_freeze_fn
        {
        private:
            static constexpr choice_t<strategy_t> choose() noexcept
            {
                if constexpr (std::is_bounded_array_v<T>) 
                {
                    return { strategy_t::none };
                }
                else if constexpr (shared_state_frozen<T>) 
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
                else if constexpr (std::is_lvalue_reference_v<T> && shared_state_frozen<const std::remove_reference_t<T>&>) 
                {
                    return { strategy_t::as_const, true };
                }
                else if constexpr (has_adl_freeze<T>) 
                {
                    return { strategy_t::adl, noexcept(shared_state_freeze(std::declval<T>())) };
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
                    return shared_state_freeze((T&&)value);
                }
                else
                {
                    return unknown_shared_state_frozen_type_of<T>{};
                }
            }
        };

        template<class T> requires (not shared_state_frozen<T&&>)
        struct shared_state_freeze_fn<T&&>
        {
            constexpr decltype(auto) operator()(T&& value) const
            noexcept(noexcept(shared_state_freeze_fn<T&>{}(value)))
            {
                using type = decltype(shared_state_freeze_fn<T&>{}(value));
                if constexpr (is_unknown_shared_state_frozen_type<type>) 
                {
                    return unknown_shared_state_frozen_type_of<T&&>{};
                }
                else if constexpr (std::same_as<type, const T&>) 
                {
                    return (const T&&)shared_state_freeze_fn<T&>{}(value);
                }
                else
                {
                    return shared_state_freeze_fn<T&>{}(value);
                }
            }
        };
    }

    inline namespace functors
    {
        template<class T>
        inline constexpr detail::shared_state_freeze_ns::shared_state_freeze_fn<T> shared_state_freeze{};
    }

    template<class T>
    using shared_state_freeze_t = decltype(shared_state_freeze<T>(std::declval<T>()));

    template<class T>
    concept shared_state_freezable = not is_unknown_shared_state_frozen_type<shared_state_freeze_t<T>>;
}

namespace senluo
{
    namespace detail
    {
        struct invoke_and_shared_state_freeze_fn
        {
            template<class F, class...Args>
            constexpr decltype(auto) operator()(F&& fn, Args&&...args) const
            {
                using result_t = std::invoke_result_t<F, Args...>;

                if constexpr(shared_state_frozen<result_t>)
                {
                    return std::invoke(static_cast<F&&>(fn), static_cast<Args&&>(args)...);
                }
                else
                {
                    return shared_state_freeze<result_t>(
                        std::invoke(static_cast<F&&>(fn), static_cast<Args&&>(args)...)
                    );
                }
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::invoke_and_shared_state_freeze_fn invoke_and_alias_freeze{};
    }
}

#endif