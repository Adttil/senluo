#ifndef SENLUO_PRETREAT_HPP
#define SENLUO_PRETREAT_HPP

#include "../tools/general.hpp"
#include "subtree.hpp"
#include "principle.hpp"
//#include "tag.hpp"

#include "../tools/macro_define.hpp"

namespace senluo
{
    namespace detail
    {
        template<class Shape>
        struct replicate_fn;
    }

    inline namespace functors
    {
        template<class Shape>
        inline constexpr detail::replicate_fn<Shape> replicate{};
    }
}

namespace senluo::detail 
{
    // template<auto FoldedLayout, class T>
    // constexpr decltype(auto) relayout_unchecked(T&& t);
    
    // template<auto FoldedStrictureTree, class T>
    // constexpr decltype(auto) astrict_unchecked(T&& t);

    // template<auto OperationTree, class T>
    // constexpr decltype(auto) operate_unchecked(T&& t);

    // template<auto FoldedOperationTree>
    // struct operate_unchecked_fn : adaptor_closure<operate_unchecked_fn<FoldedOperationTree>>
    // {
    //     template<class T>
    //     constexpr decltype(auto) operator()(T&& t) const
    //     {
    //         return detail::operate_unchecked<FoldedOperationTree>(FWD(t));
    //     }
    // };

    // template<auto FoldedOperationTree>
    // inline constexpr operate_unchecked_fn<FoldedOperationTree> operate_unchecked_closure{};

    // template<class P>
    // constexpr decltype(auto) principle_data(P&& p)
    // {
    //     using data_t = decltype(std::move(p).data());
    //     if constexpr(std::is_rvalue_reference_v<data_t>)
    //     {
    //         return wrapper<data_t>{ std::move(p).data() };
    //     }
    //     else
    //     {
    //         return std::move(p).data();
    //     }
    // }

    // template<class P, size_t I>
    // constexpr auto principle_operation_tree()
    // {
    //     if constexpr(I <= P::operation_tree_count)
    //     {
    //         return P::template operation_tree<I>;
    //     }
    //     else
    //     {
    //         return operation_t::none;
    //     }
    // }

    using stricture_t = unsigned char;
    inline constexpr stricture_t strictrue_none = 0;
    inline constexpr stricture_t strictrue_readonly = 1;

    using usage_t = unsigned char;
    inline constexpr usage_t usage_none = 0;
    inline constexpr usage_t usage_once = 1;

    template<class T>
    consteval bool only_input()
    {
        if constexpr(not (std::is_const_v<std::remove_reference_t<T>> || std::is_object_v<T>))
        {
            return false;
        }
        else if constexpr(terminal<T>)
        {
            return true;
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            return (... && detail::only_input<tree_get_t<I, T>>());
        }(std::make_index_sequence<size<T>>{});
    }

    template<class T>
    constexpr auto intrinsic_stricture_tree()
    {
        if constexpr(only_input<T>())
        {
            return strictrue_readonly;
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(
                detail::intrinsic_stricture_tree<tree_get_t<I, T>>()...
            );
        }(std::make_index_sequence<size<T>>{});
    }

    template<auto StrictureTree, class T>
    constexpr auto stricture_operation_tree(stricture_t value = strictrue_readonly)
    {
        if constexpr(std::same_as<decltype(StrictureTree), stricture_t>)
        {
            if constexpr(StrictureTree == value)
            {
                return as_input;
            }
            else
            {
                return pass;
            }
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto subresults = make_tuple(
                detail::stricture_operation_tree<StrictureTree.template get<I>(), tree_get_t<I, T>>()...
            );
            constexpr auto first = subresults.template get<0>();
            if constexpr(not adaptor_closuroid<decltype(first)>)
            {
                return subresults;
            }
            else if constexpr((... && detail::equal(subresults.template get<I>(), first)))
            {
                if constexpr(detail::equal(first, pass))
                {
                    return pass;
                }
                else if constexpr(detail::equal(first, as_input) && detail::only_input<decltype(as_input(std::declval<T>()))>())
                {
                    return as_input;
                }
                else
                {
                    return subresults;
                }
            }
            else
            {
                return subresults;
            }
        }(std::make_index_sequence<StrictureTree.size()>{});
    }

    template<auto UsedTree, class IntrinsicStrictureTree>
    constexpr auto init_edit_trees()
    {
        if constexpr(std::same_as<decltype(UsedTree), usage_t>)
        {
            if constexpr(UsedTree == usage_none)
            {
                return 0uz;
            }
            else
            {
                return IntrinsicStrictureTree{};
            }
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(
                detail::init_edit_trees<UsedTree.template get<I>(), IntrinsicStrictureTree>()...
            );
        }(std::make_index_sequence<UsedTree.size()>{});
    }

    template<class EditTrees>
    constexpr void set_edit_tree(EditTrees& edit_trees, bool& has_edit, const EditTrees& base_stricture_tree)
    {
        if constexpr(std::same_as<EditTrees, usage_t>)
        {
            if(base_stricture_tree != strictrue_readonly)
            {
                edit_trees = usage_once;
                has_edit = true;
            }
        }
        else [&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., detail::set_edit_tree(edit_trees.template get<I>(), base_stricture_tree.template get<I>()));
        }(std::make_index_sequence<size<EditTrees>>{});
    }

    template<array Index, class EditTrees>
    constexpr void set_edit_tree_at(EditTrees& edit_trees, bool& has_edit, const EditTrees& base_stricture_tree)
    {
        if constexpr(std::same_as<EditTrees, usage_t>)
        {
            if(base_stricture_tree != strictrue_readonly)
            {
                edit_trees = usage_once;
                has_edit = true;
            }
        }
        else if constexpr(Index.size() == 0uz)
        {
            detail::set_edit_tree(edit_trees, has_edit, base_stricture_tree);
        }
        else
        {
            detail::set_edit_tree_at<detail::array_drop<1uz>(Index)>(
                edit_trees.template get<Index[0]>(), base_stricture_tree.template get<Index[0]>()
            );
        }
    }

    template<tuple Indexes, class EditTrees>
    constexpr void set_edit_tree_of(EditTrees& edit_trees, bool& has_edit, const EditTrees& base_stricture_tree)
    {
        [&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., detail::set_edit_tree_at<Indexes.template get<I>()>(edit_trees, has_edit, base_stricture_tree));
        }(std::make_index_sequence<Indexes.size()>{});
    }
    
    template<class AC, array SubIndex = indexes_of_whole, class EditTrees, class BaseStrictureTree>
    constexpr void set_edit_trees(EditTrees& edit_trees, bool& has_edit, const BaseStrictureTree& base_stricture_tree)
    {
        if constexpr(std::same_as<EditTrees, BaseStrictureTree>)
        {
            detail::set_edit_tree_of<AC::template dependencies<SubIndex>()>(edit_trees, has_edit, base_stricture_tree);
        }
        else if constexpr(std::same_as<EditTrees, usage_t>)
        {
            return;
        }
        else [&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., detail::set_edit_trees<AC, array_cat(SubIndex, array{I})>(edit_trees. template get<I>(), has_edit, base_stricture_tree));
        }(std::make_index_sequence<EditTrees::size()>{});
    }

    template<class EditTree>
    void set_strictrue(EditTree& edit_tree, bool& has_edit, EditTree& current_stricture)
    {
        if constexpr(std::same_as<EditTree, usage_t>)
        {
            if(edit_tree == usage_once)
            {
                edit_tree = current_stricture;
                current_stricture = strictrue_readonly;
                has_edit = true;
            }
        }
        else [&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., detail::set_strictrue(edit_tree.template get<I>(), has_edit, current_stricture.template get<I>()));
        }(std::make_index_sequence<EditTree::size()>{});
    }

    template<class EditTrees, class CurrentStrictureTree>
    void set_seq_strictrues(EditTrees& edit_trees, bool* edit_stack, CurrentStrictureTree& current_stricture)
    {
        if constexpr(std::same_as<EditTrees, CurrentStrictureTree>)
        {
            edit_stack[0] = edit_stack[1];
            edit_stack[1] = false;
            detail::set_strictrue(edit_trees, edit_trees[1], current_stricture);
        }
        else if constexpr(std::same_as<EditTrees, usage_t>)
        {
            return;
        }
        else [&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., detail::set_seq_strictrues(edit_trees.template get<I>(), edit_stack, current_stricture));
        }(std::make_index_sequence<EditTrees::size()>{});
    }

    template<class EditTrees, class CurrentStrictureTree>
    void set_inverse_seq_strictrues(EditTrees& edit_trees, bool* edit_stack, CurrentStrictureTree& current_stricture)
    {
        if constexpr(std::same_as<EditTrees, CurrentStrictureTree>)
        {
            edit_stack[0] = edit_stack[1];
            edit_stack[1] = false;
            detail::set_strictrue(edit_trees, edit_stack[1], current_stricture);
        }
        else if constexpr(std::same_as<EditTrees, usage_t>)
        {
            return;
        }
        else [&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., detail::set_seq_strictrues(edit_trees.template get<EditTrees::size() - 1uz - I>(), edit_stack, current_stricture));
        }(std::make_index_sequence<EditTrees::size()>{});
    }

    template<class EditTree>
    void set_sep_strictrue(const EditTree& edit_tree, EditTree& current_stricture)
    {
        if constexpr(std::same_as<EditTree, usage_t>)
        {
            if(edit_tree == usage_once && current_stricture <= 2)
            {
                ++current_stricture;
            }
        }
        else [&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., detail::set_sep_strictrue(edit_tree.template get<I>(), current_stricture.template get<I>()));
        }(std::make_index_sequence<EditTree::size()>{});
    }

    template<class EditTrees, class CurrentStrictureTree>
    void set_sep_strictrues(const EditTrees& edit_trees, CurrentStrictureTree& current_stricture)
    {
        if constexpr(std::same_as<EditTrees, CurrentStrictureTree>)
        {
            detail::set_sep_strictrue(edit_trees, current_stricture);
        }
        else if constexpr(std::same_as<EditTrees, usage_t>)
        {
            return;
        }
        else [&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., detail::set_sep_strictrues(edit_trees.template get<I>(), current_stricture));
        }(std::make_index_sequence<EditTrees::size()>{});
    }

    template<class AC, auto UsedTree, auto BaseStrictureTree>
    constexpr auto get_seq_stricture()
    {
        auto edit_trees = detail::init_edit_trees<UsedTree, BaseStrictureTree>();
        struct result_t
        {
            bool has_edit;
            decltype(edit_trees) edit_trees;
        };

        bool has_edit = false;
        detail::set_edit_trees<AC>(edit_trees, has_edit, BaseStrictureTree);

        if(not has_edit)
        {
            return result_t{ false, {} };
        }

        decltype(BaseStrictureTree) current_stricture = {};
        bool edit_stack[2] = { false, false };
        detail::set_seq_strictrues(edit_trees, edit_stack, current_stricture);
        
        if(not edit_stack[0])
        {
            return result_t{ false, {} };
        }
        
        return result_t{ true, edit_trees };
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

    template<typename T>
    using principle_data_t = decltype((principle(std::declval<T>())).data());

    template<class Pretreater>
    struct pretreater_interface : adaptor_closure<Pretreater>
    {
        template<class T>
        static constexpr decltype(auto) adapt(T&& tree)
        {
            return Pretreater::pretreat(principle((T&&)tree));
        }
    };

    namespace detail
    {
        template<auto UsageTree>
        struct sequence_by_usage_t : pretreater_interface<sequence_by_usage_t<UsageTree>>
        {
            static constexpr auto usage_tree = UsageTree;

            template<class P>
            static constexpr decltype(auto) pretreat(P&& principle)
            {
                using data_t = decltype(std::declval<P>().data());
                constexpr auto intrinsic_stricture_tree = detail::intrinsic_stricture_tree<data_t>();
                constexpr auto edit_st = detail::get_seq_stricture<decltype(P::adaptor_closure()), UsageTree, intrinsic_stricture_tree>();
                constexpr bool has_edit = edit_st.has_edit;
                constexpr auto edit_trees = edit_st.edit_trees;

                if constexpr(not has_edit)
                {
                    return std::move(principle).data() || P::adaptor_closure();
                }
                else
                {
                    constexpr auto op = detail::stricture_operation_tree<>();
                    return
                }
            }
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