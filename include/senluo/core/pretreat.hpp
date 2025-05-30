#ifndef SENLUO_PRETREAT_HPP
#define SENLUO_PRETREAT_HPP

#include "../tools/general.hpp"
#include "tree.hpp"
#include "wrap.hpp"
#include "tag.hpp"

#include "../tools/macro_define.hpp"

// namespace senluo
// {
//     namespace detail
//     {
//         template<auto Layout>
//         struct relayout_fn;

//         template<auto OperationTree>
//         struct operate_fn;
//     }

//     inline namespace functors
//     {
//         template<auto Layout>
//         inline constexpr detail::relayout_fn<Layout> relayout{};

//         template<auto OperationTree>
//         inline constexpr detail::operate_fn<detail::fold_operation_tree<OperationTree>()> operate{};
//     }
// }

namespace senluo::detail 
{
    template<auto FoldedLayout, class T>
    constexpr decltype(auto) relayout_unchecked(T&& t);
    
    template<auto FoldedStrictureTree, class T>
    constexpr decltype(auto) astrict_unchecked(T&& t);

    template<auto OperationTree, class T>
    constexpr decltype(auto) operate_unchecked(T&& t);

    template<auto FoldedOperationTree>
    struct operate_unchecked_fn : adaptor_closure<operate_unchecked_fn<FoldedOperationTree>>
    {
        template<class T>
        constexpr decltype(auto) operator()(T&& t) const
        {
            return detail::operate_unchecked<FoldedOperationTree>(FWD(t));
        }
    };

    template<auto FoldedOperationTree>
    inline constexpr operate_unchecked_fn<FoldedOperationTree> operate_unchecked_closure{};

    template<class P>
    constexpr decltype(auto) principle_data(P&& p)
    {
        using data_t = decltype(std::move(p).data());
        if constexpr(std::is_rvalue_reference_v<data_t>)
        {
            return wrapper<data_t>{ std::move(p).data() };
        }
        else
        {
            return std::move(p).data();
        }
    }

    template<class P, size_t I>
    constexpr auto principle_operation_tree()
    {
        if constexpr(I <= P::operation_tree_count)
        {
            return P::template operation_tree<I>;
        }
        else
        {
            return operation_t::none;
        }
    }
}

namespace senluo
{
    template<class T>
    struct principle_layout;

    template<class T>
    inline constexpr auto principle_layout_v = principle_layout<T>::value;

    template<class T>
    struct principle_operation_tree_count;

    template<class T>
    inline constexpr auto principle_operation_tree_count_v = principle_operation_tree_count<T>::value;

    template<class T, size_t I>
    struct principle_operation_tree;

    template<class T, size_t I>
    inline constexpr auto principle_operation_tree_v = principle_operation_tree<T, I>::value;

    namespace detail::principle_fn_ns 
    {
        template<auto UsageTree>
        struct principle_fn;

        template<auto UsageTree>
        void principle();
    }

    inline namespace functors
    {
        template<auto UsageTree>
        inline constexpr detail::principle_fn_ns::principle_fn<UsageTree> principle{};
    }

    template<typename T, auto UsageTree>
    using principle_t = decltype(std::declval<T>() | principle<UsageTree>);

    template<typename T, auto UsageTree>
    using principle_data_t = decltype((principle<UsageTree>(std::declval<T>())).data());

    template<class Pretreater>
    struct pretreater_interface : adaptor_closure<Pretreater>
    {
        template<class T>
        constexpr decltype(auto) operator()(T&& tree) const
        {
            using principle_t = senluo::principle_t<T, Pretreater::usage_tree>;
            auto p = principle<Pretreater::usage_tree>((T&&)tree);
            
            using data_t = principle_data_t<T, Pretreater::usage_tree>;
            
            return [&]<size_t...I>(std::index_sequence<I...>) -> decltype(auto)
            {
                constexpr auto layout = p.layout;
                //constexpr auto independence_tree = p.independence_tree();
                constexpr auto pretreat_stricture_tree = Pretreater::template stricture_tree<layout, data_t>;
                constexpr auto stricture_tree = detail::merge_stricture_tree<p.stricture_tree, pretreat_stricture_tree>();
                
                return decltype((detail::astrict_unchecked<stricture_tree>(detail::relayout_unchecked<layout>(detail::principle_data(p))) 
                    | ... | detail::operate_unchecked_closure<detail::principle_operation_tree<principle_t, I>()>)
                )
                {
                    detail::principle_data(p)
                };
            }(std::make_index_sequence<p.operation_tree_count>{});
        }
    };

    namespace detail
    {
        template<auto UsageTree>
        struct sequence_by_usage_t : pretreater_interface<sequence_by_usage_t<UsageTree>>
        {
            static constexpr auto usage_tree = UsageTree;

            template<auto Layout, class Base>
            static constexpr auto stricture_tree = 
                detail::sequence_stricture_v<Layout, UsageTree, intrinsic_stricture_tree_v<Base>, shape_t<Base>>;
        };

        template<auto UsageTree>
        struct inverse_sequence_by_usage_t : pretreater_interface<inverse_sequence_by_usage_t<UsageTree>>
        {
            static constexpr auto usage_tree = UsageTree;

            template<auto Layout, class Base>
            static constexpr auto stricture_tree = 
                detail::inverse_sequence_stricture_v<Layout, UsageTree, intrinsic_stricture_tree_v<Base>, shape_t<Base>>;
        };

        template<auto UsageTree>
        struct seperate_by_usage_t : pretreater_interface<seperate_by_usage_t<UsageTree>>
        {
            static constexpr auto usage_tree = UsageTree;

            template<auto Layout, class Base>
            static constexpr auto stricture_tree = 
                detail::seperate_stricture_v<Layout, UsageTree, intrinsic_stricture_tree_v<Base>, shape_t<Base>>;
        };
    }

    inline namespace functors
    {
        template<auto UsageTree>
        inline constexpr detail::sequence_by_usage_t<UsageTree> sequence_by_usage{};

        template<auto UsageTree>
        inline constexpr detail::inverse_sequence_by_usage_t<UsageTree> inverse_sequence_by_usage{};

        template<auto UsageTree>
        inline constexpr detail::seperate_by_usage_t<UsageTree> seperate_by_usage{};
    }

    namespace detail
    {
        struct sequence_t : adaptor_closure<sequence_t>
        {
            template<branched T>
            constexpr decltype(auto) operator()(T&& tree)const
            AS_EXPRESSION(
                FWD(tree) | sequence_by_usage<detail::replicate(usage_t::once, shape<array<int, size<T>>>)>
            )

            template<terminal T>
            constexpr tuple<> operator()(T&& t)const noexcept
            {
                return {};
            }
        };

        struct inverse_sequence_t : adaptor_closure<inverse_sequence_t>
        {
            template<branched T>
            constexpr decltype(auto) operator()(T&& tree)const
            AS_EXPRESSION(
                FWD(tree) | inverse_sequence_by_usage<detail::replicate(usage_t::once, shape<array<int, size<T>>>)>
            )

            template<terminal T>
            constexpr tuple<> operator()(T&& t)const noexcept
            {
                return {};
            }
        };

        struct seperate_t : adaptor_closure<seperate_t>
        {
            template<branched T>
            constexpr decltype(auto) operator()(T&& tree)const
            AS_EXPRESSION(
                FWD(tree) | seperate_by_usage<detail::replicate(usage_t::once, shape<array<int, size<T>>>)>
            )

            template<terminal T>
            constexpr tuple<> operator()(T&& t)const noexcept
            {
                return {};
            }
        };
    }

    inline namespace functors
    {
        inline constexpr detail::sequence_t sequence{};
        
        inline constexpr detail::inverse_sequence_t inverse_sequence{};
        
        inline constexpr detail::seperate_t seperate{};
    }
}

#include "../tools/macro_undef.hpp"
#endif