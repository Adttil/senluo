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

    template<class F>
    struct tree_adaptor_closure
    {
        // template<class T, class Self>
        // constexpr decltype(auto) unwrap_adapt(this Self&& self, T&& t)
        // AS_EXPRESSION(
        //     std::forward<Self>(self).adapt(std::forward<T>(t))
        // )

        // template<class T, class Self>
        // constexpr auto operator()(this Self&& self, T&& t) 
        //     noexcept(noexcept(materialize(FWD(self).adapt(FWD(t)))))
        //     -> materialize_t<decltype(FWD(self).adapt(FWD(t)))>
        // {
        //     using rtype = decltype(FWD(self).adapt(FWD(t)));
        //     if constexpr(std::same_as<rtype, materialize_t<rtype>>)
        //     {
        //         return FWD(self).adapt(FWD(t));
        //     }
        //     else
        //     {
        //         return materialize(FWD(self).adapt(FWD(t)));
        //     }
        // }

        // template<array Indexes>
        // static consteval tuple<array<size_t, 0uz>> dependencies()
        // {
        //     return {};
        // }
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
            noexcept(noexcept(F::adapt(FWD(args)...)))
            -> decltype(F::adapt(FWD(args)...))
        {
            return F::adapt(FWD(args)...);
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
        constexpr decltype(auto) operator()(this Self&& self, T&& val)
        AS_EXPRESSION(
            FWD(self, right)(FWD(self, left)(FWD(val)))
        )

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
}

namespace senluo 
{
    namespace detail
    {
        struct pass_fn : tree_adaptor_closure<pass_fn>
        {
            template<class T>
            static constexpr decltype(auto) operator()(T&& t) noexcept
            {
                return (T)FWD(t);
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::pass_fn pass{};
    }
}

#include "../tools/macro_undef.hpp"
#endif