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
}

namespace senluo
{
    namespace detail::principle_t_ns 
    {
        template<auto UsageTree>
        struct principle_fn;

        template<auto UsageTree>
        void principle();
    }

    inline namespace functors
    {
        template<auto UsageTree>
        inline constexpr detail::principle_t_ns::principle_fn<UsageTree> principle{};
    }

    template<typename T, auto UsageTree>
    using principle_t = decltype(std::declval<T>() | principle<UsageTree>);

    template<class Pretreater>
    struct pretreater_interface : adaptor_closure<Pretreater>
    {
        constexpr decltype(auto) operator()(auto&& tree) const
        {
            auto p = FWD(tree) | principle<Pretreater::usage_tree()>;
            
            using data_t = decltype(FWD(p).data());
            
            constexpr auto layout = p.layout();
            constexpr auto raw_stricture_tree = p.stricture_tree();
            constexpr auto independence_tree = p.independence_tree();
            constexpr auto stricture_tree = Pretreater::template pretreat_stricture<raw_stricture_tree, independence_tree, layout>(shape<data_t>);
            constexpr auto operation_tree = p.operation_tree();

            if constexpr(std::is_rvalue_reference_v<data_t>)
            {
                return decltype(detail::operate_unchecked<operation_tree>(
                    detail::astrict_unchecked<stricture_tree>(
                        detail::relayout_unchecked<layout>(wrapper<data_t>{ std::move(p).data() })
                    )
                ))
                {
                    std::move(p).data()
                };
            }
            else
            {
                return decltype(detail::operate_unchecked<operation_tree>(
                    detail::astrict_unchecked<stricture_tree>(
                        detail::relayout_unchecked<layout>(std::move(p).data())
                    )
                ))
                {
                    std::move(p).data()
                };
            }
        }
    };

    namespace detail
    {
        template<auto UsageTree>
        struct sequence_by_usage_t : pretreater_interface<sequence_by_usage_t<UsageTree>>
        {
            static consteval auto usage_tree()
            {
                return UsageTree;
            }

            template<auto RawStrictureTree, auto IndependenceTree, auto Layout, class S>
            static consteval auto pretreat_stricture(S data_shape)
            {
                if constexpr(requires{ requires IndependenceTree >= independence_t::safe; })
                {
                    return RawStrictureTree;
                }
                else
                {
                    auto cur_data_stricture_tree = detail::make_tree_of_same_value(stricture_t::none, data_shape);
                    return detail::get_sequence_stricture_tree_impl<RawStrictureTree, IndependenceTree, Layout, UsageTree>(cur_data_stricture_tree);
                }
            }
        };

        template<auto UsageTree>
        struct inverse_sequence_by_usage_t : pretreater_interface<inverse_sequence_by_usage_t<UsageTree>>
        {
            static consteval auto usage_tree()
            {
                return UsageTree;
            }

            template<auto RawStrictureTree, auto IndependenceTree, auto Layout, class S>
            static consteval auto pretreat_stricture(S data_shape)
            {
                auto cur_data_stricture_tree = detail::make_tree_of_same_value(stricture_t::none, data_shape);
                return detail::get_inverse_sequence_stricture_tree_impl<RawStrictureTree, IndependenceTree, Layout, UsageTree>(data_shape);
            }
        };

        template<auto UsageTree>
        struct seperate_by_usage_t : pretreater_interface<seperate_by_usage_t<UsageTree>>
        {
            static consteval auto usage_tree()
            {
                return UsageTree;
            }

            template<auto RawStrictureTree, auto IndependenceTree, auto Layout, class S>
            static consteval auto pretreat_stricture(S data_shape)
            {
                constexpr auto seq_stricture_tree = sequence_by_usage_t<UsageTree>::template pretreat_stricture<RawStrictureTree, IndependenceTree, Layout>(S{});
                constexpr auto inv_stricture_tree = inverse_sequence_by_usage_t<UsageTree>::template pretreat_stricture<RawStrictureTree, IndependenceTree, Layout>(S{});

                return detail::merge_stricture_tree<seq_stricture_tree, inv_stricture_tree>();
            }
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
                FWD(tree) | sequence_by_usage<detail::make_tree_of_same_value(usage_t::once, shape<array<int, size<T>>>)>
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
                FWD(tree) | inverse_sequence_by_usage<detail::make_tree_of_same_value(usage_t::once, shape<array<int, size<T>>>)>
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
                FWD(tree) | seperate_by_usage<detail::make_tree_of_same_value(usage_t::once, shape<array<int, size<T>>>)>
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