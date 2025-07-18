#ifndef SENLUO_ADAPTOR_CLOSURE_HPP
#define SENLUO_ADAPTOR_CLOSURE_HPP

#include "../tools/general.hpp"
#include "../tools/wrapper.hpp"
#include "../tools/tuple.hpp"
#include "../tools/array.hpp"

#include "../tools/macro_define.hpp"

namespace senluo
{
    using std::size_t;

    // template<class T>
    // struct materialize : std::type_identity<ideal_store_t<T>> {};

    // template<class T>
    // using materialize_t = materialize<T>::type;

    namespace detail::materialize_ns
    {
        void materialize();

        struct materialize_fn
        {
            template<class T>
            static constexpr decltype(auto) operator()(T&& t)
            {
                if constexpr(requires{ std::forward<T>(t).materialize(custom_t{}); })
                {
                    return std::forward<T>(t).materialize(custom_t{});
                }
                else if constexpr(requires{ materialize(std::forward<T>(t), custom_t{}); })
                {
                    return materialize(std::forward<T>(t), custom_t{});
                }
                else if constexpr(std::is_rvalue_reference_v<T&&>)
                {
                    return std::remove_cvref_t<T>{ std::forward<T>(t) };
                }
                else
                {
                    return std::forward<T>(t);
                }
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::materialize_ns::materialize_fn materialize{};
    }

    template<class T>
    using materialize_t = decltype(materialize(std::declval<T>()));

    template<class F>
    struct tree_adaptor_closure
    {
        // template<class T, class Self>
        // constexpr decltype(auto) unwrap_adapt(this Self&& self, T&& t)
        // AS_EXPRESSION(
        //     std::forward<Self>(self).adapt(std::forward<T>(t))
        // )

        template<class T, class Self>
        constexpr auto operator()(this Self&& self, T&& t) 
            noexcept(noexcept(materialize(FWD(self).adapt(FWD(t)))))
            -> materialize_t<decltype(FWD(self).adapt(FWD(t)))>
        {
            using rtype = decltype(FWD(self).adapt(FWD(t)));
            if constexpr(std::same_as<rtype, materialize_t<rtype>>)
            {
                return FWD(self).adapt(FWD(t));
            }
            else
            {
                return materialize(FWD(self).adapt(FWD(t)));
            }
        }

        template<array Indexes>
        static consteval tuple<array<size_t, 0uz>> dependencies()
        {
            return {};
        }
    };

    template<class T>
    concept adaptor_closuroid = requires(std::remove_cvref_t<T>& t) 
    {
        { []<class F>(tree_adaptor_closure<F>&)->F*{}(t) } -> std::same_as<std::remove_cvref_t<T>*>;
    };

    namespace detail
    {
        template<class Adaptor, class...Args>
        struct closure : tuple<Args...>, tree_adaptor_closure<closure<Adaptor, Args...>>
        {
            // clang crashed with [[no_unique_address]]
            // https://github.com/llvm/llvm-project/issues/104227
            // SENLUO(no_unique_address) tuple<Args...> captures;

        private:
            template<typename T, size_t...I, typename Self>
            constexpr decltype(auto) impl(this Self&& self, T&& t, std::index_sequence<I...>)
            AS_EXPRESSION(
                Adaptor::adapt(FWD(t), get<I>(FWD(self))...)
            )

        public:
            template<typename T, typename Self>
            constexpr decltype(auto) adapt(this Self&& self, T&& t)
            AS_EXPRESSION(
                FWD(self).impl(FWD(t), std::index_sequence_for<Args...>{})
            )
        };
    }

    template<class F> 
    struct adaptor
    {
        template<typename...Args>
        static constexpr auto operator()(Args&&...args) 
            noexcept(noexcept(materialize(F::adapt(FWD(args)...))))
            -> materialize_t<decltype(F::adapt(FWD(args)...))>
        {
            using rtype = decltype(F::adapt(FWD(args)...));
            if constexpr(std::same_as<rtype, materialize_t<rtype>>)
            {
                return F::adapt(FWD(args)...);
            }
            else 
            {
                return materialize(F::adapt(FWD(args)...));
            }
        }

        template<typename...Args> 
            requires (not requires{ F{}.adapt(std::declval<Args>()...); })
        static constexpr detail::closure<F, Args...> operator()(Args&&...args)
        AS_EXPRESSION(
            detail::closure<F, Args...>{ FWD(args)... }
        )
    };
}

namespace senluo::detail
{        
    template<class ClosureLeft, class ClosureRight>
    struct pipeline : tree_adaptor_closure<pipeline<ClosureLeft, ClosureRight>>
    {
        SENLUO(no_unique_address) ClosureLeft  left;
        SENLUO(no_unique_address) ClosureRight right;

        template<class T, class Self>
        constexpr decltype(auto) adapt(this Self&& self, T&& val)
        AS_EXPRESSION(
            FWD(self, right).adapt(FWD(self, left).adapt(FWD(val)))
        )

        template<array Indexes>
        static consteval auto dependencies()
        {
            const auto local_dependencies = std::remove_cvref_t<ClosureRight>::template dependencies<Indexes>();
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                return tuple_cat(std::remove_cvref_t<ClosureLeft>::template data_dependencies<local_dependencies.template get<I>()>()...);
            }(std::make_index_sequence<local_dependencies.size()>{});
        }

        friend constexpr bool operator==(const pipeline&, const pipeline&) = default; 
    };
}

namespace senluo
{
    template<adaptor_closuroid L, adaptor_closuroid R>
    constexpr detail::pipeline<L, R> operator|(L&& l, R&& r)
    AS_EXPRESSION(
        detail::pipeline<L, R>{ {}, FWD(l), FWD(r) }
    )

    template<class L, adaptor_closuroid R>
    requires (not adaptor_closuroid<L>)
    constexpr decltype(auto) operator|(L&& l, R&& r)
    AS_EXPRESSION(
        FWD(r)(FWD(l))
    )

    template<class L, adaptor_closuroid R>
    requires (not adaptor_closuroid<L>)
    constexpr decltype(auto) operator||(L&& l, R&& r)
    AS_EXPRESSION(
        FWD(r).adapt(FWD(l))
    )
}

namespace senluo 
{
    namespace detail
    {
        struct pass_fn : tree_adaptor_closure<pass_fn>
        {
            template<class T>
            static constexpr decltype(auto) adapt(T&& t) noexcept
            {
                return FWD(t);
            }

            template<array Indexes>
            static consteval auto dependencies()
            {
                return tuple{ Indexes };
            }
        };

        struct refer_fn : tree_adaptor_closure<refer_fn>
        {
            template<class T>
            static constexpr decltype(auto) adapt(T&& t) noexcept
            {
                if constexpr(std::is_lvalue_reference_v<T>)
                {
                    return FWD(t);
                }
                else
                {
                    return wrapper<T&&>{ FWD(t) };
                }
            }

            template<array Indexes>
            static consteval auto dependencies()
            {
                return tuple{ Indexes };
            }

            template<class T>
            static constexpr decltype(auto) operator()(T&& t) noexcept
            {
                return adapt(FWD(t));
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::pass_fn pass{};
        inline constexpr detail::refer_fn refer{};
    }

    namespace detail
    {
        struct as_input_fn : tree_adaptor_closure<as_input_fn>
        {
            template<class T>
            static constexpr decltype(auto) adapt(T&& t) noexcept
            {
                using utype = ideal_unwrap_t<T>;
                if constexpr(std::is_object_v<utype>)
                {
                    auto&& ref = unwrap_fwd(FWD(t));
                    return std::move(std::as_const(ref));
                }
                else if constexpr(std::is_rvalue_reference_v<utype>)
                {
                    auto&& ref = unwrap_fwd(FWD(t));
                    return std::move(std::as_const(ref)) | refer;
                }
                else
                {
                    return std::as_const(unwrap_fwd(FWD(t)));
                }
            }

            template<array Indexes>
            static consteval auto dependencies()
            {
                return tuple{ Indexes };
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::as_input_fn as_input{};
    }

    namespace detail
    {
        template<class T>
        struct unwrap_opt_refer_fn
        {
            template<class U>
            static constexpr decltype(auto) operator()(wrapper<U>& t) noexcept
            {
                if constexpr(std::is_rvalue_reference_v<decltype(std::forward_like<T>(t).get())>
                    && (std::is_reference_v<T> || std::is_reference_v<U>)
                )
                {
                    return refer(std::forward_like<T>(t).get());
                }
                else
                {
                    return std::forward_like<T>(t).get();
                }
            }

            template<class U>
            static constexpr decltype(auto) operator()(const wrapper<U>& t) noexcept
            {
                if constexpr(std::is_rvalue_reference_v<decltype(std::forward_like<T>(t).get())>
                    && (std::is_reference_v<T> || std::is_reference_v<U>)
                )
                {
                    return refer(std::forward_like<T>(t).get());
                }
                else
                {
                    return std::forward_like<T>(t).get();
                }
            }
        };
    }

    inline namespace functors
    {
        template<class T>
        inline constexpr detail::unwrap_opt_refer_fn<T> unwrap_opt_refer{};
    }
}

#include "../tools/macro_undef.hpp"
#endif