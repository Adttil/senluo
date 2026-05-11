#ifndef SENLUO_PIPELINE_HPP
#define SENLUO_PIPELINE_HPP

#include "general.hpp"
#include "tuple.hpp"

#include "macro_define.hpp"

namespace senluo
{
    template<class F>
    struct pipe_closure{};

    template<class T>
    concept pipe_closuroid = requires(std::remove_cvref_t<T>& t) 
    {
        { []<class F>(pipe_closure<F>&)->F*{}(t) } -> std::same_as<std::remove_cvref_t<T>*>;
    };

    namespace detail
    {
        template<class Pipe, class...Args>
        struct closure : tuple<Args...>, pipe_closure<closure<Pipe, Args...>>
        {
            // clang crashed with [[no_unique_address]]
            // https://github.com/llvm/llvm-project/issues/104227
            // SENLUO(no_unique_address) tuple<Args...> captures;

        private:
            template<typename T, size_t...I, typename Self>
            constexpr decltype(auto) impl(this Self&& self, T&& t, std::index_sequence<I...>)
            AS_EXPRESSION(
                Pipe{}(FWD(t), get<I>(FWD(self))...)
            )

        public:
            template<typename T, typename Self>
            constexpr decltype(auto) operator()(this Self&& self, T&& t)
            AS_EXPRESSION(
                FWD(self).impl(FWD(t), std::index_sequence_for<Args...>{})
            )
        };
    }

    template<class F> 
    struct pipe
    {
        template<typename...Args>
        constexpr decltype(auto) operator()(Args&&...args) const
        AS_EXPRESSION(
            F{}.apply(FWD(args)...)
        )

        template<typename...Args> 
            requires (not requires{ F{}.apply(std::declval<Args>()...); })
        constexpr detail::closure<F, Args...> operator()(Args&&...args) const
        AS_EXPRESSION(
            detail::closure<F, Args...>{ FWD(args)... }
        )
    };
}

namespace senluo::detail
{        
    template<class ClosureLeft, class ClosureRight>
    struct pipeline : pipe_closure<pipeline<ClosureLeft, ClosureRight>>
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
    template<pipe_closuroid L, pipe_closuroid R>
    constexpr detail::pipeline<L, R> operator|(L&& l, R&& r)
    AS_EXPRESSION(
        detail::pipeline<L, R>{ {}, FWD(l), FWD(r) }
    )

    template<class L, pipe_closuroid R>
    requires (not pipe_closuroid<L>)
    constexpr decltype(auto) operator|(L&& l, R&& r)
    AS_EXPRESSION(
        FWD(r)(FWD(l))
    )
}

#include "macro_undef.hpp"
#endif