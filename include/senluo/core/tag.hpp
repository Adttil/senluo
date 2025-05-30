#ifndef SENLUO_TAG_HPP
#define SENLUO_TAG_HPP

#include "../tools/general.hpp"
#include "subtree.hpp"

#include "../tools/macro_define.hpp"

namespace senluo
{
    enum class usage_t
    {
        none,
        once,
        repeatedly
    };

    constexpr usage_t operator&(usage_t l, usage_t r)noexcept
    {
        auto result = std::to_underlying(l) + std::to_underlying(r);
        return result < std::to_underlying(usage_t::repeatedly) ? usage_t{ result } : usage_t::repeatedly ;
    }

    enum class stricture_t
    {
        none,
        readonly
    };

    constexpr stricture_t operator&(stricture_t l, stricture_t r)noexcept
    {
        if(l == stricture_t::none && r == stricture_t::none)
        {
            return stricture_t::none;
        }
        else
        {
            return stricture_t::readonly;
        }
    }

    enum class operation_t
    {
        none,
        apply_invoke
    };

    enum class independence_t
    {
        none,
        safe//,
        //isolated
    };

    enum class layout_mapping_type_t
    {
        multi_shot,
        injective,
        bijective
    };

    template<class T>
    struct is_only_input
    {
        static consteval bool get_value()
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
                return (... && is_only_input<subtree_t<T, I>>::value);
            }(std::make_index_sequence<size<T>>{});
        }

        static constexpr bool value = get_value();
    };

    template<class T>
    inline constexpr bool is_only_input_v = is_only_input<T>::value;
}

namespace senluo::detail
{
    //for msvc adl bug: https://gcc.godbolt.org/z/GYvdrbooW
    template<size_t>
    void get();
}

namespace senluo::detail
{
    template<size_t I, class T>
    constexpr auto tag_tree_get(const T& tag_tree)
    {
        if constexpr(terminal<T>)
        {
            return tag_tree;
        }
        else
        {
            return get<I>(tag_tree);
        }
    }

    // template<size_t I, auto TagTree>
    // inline constexpr auto tag_tree_get_Cache = tag_tree_get<I>(TagTree);

    template<auto indexes, class T>
    constexpr auto tag_subtree(const T& tag_tree)
    {
        if constexpr(detail::equal(indexes, indexes_of_whole))
        {
            return tag_tree;
        }
        else
        {
            return detail::tag_subtree<detail::array_drop<1uz>(indexes)>(detail::tag_tree_get<indexes[0]>(tag_tree));
        }
    }

    template<size_t I, class L>
    constexpr auto layout_get(const L& layout)
    {
        if constexpr(indexical<L>)
        {
            return detail::array_cat(layout, array{ I });
        }
        else if constexpr(I < size<L>)
        {
            return get<I>(layout);
        }
        // else
        // {
        //     return invalid_index;
        // }
    }

    template<auto Indexes, class L>
    constexpr auto sublayout(const L& layout)
    {
        if constexpr(detail::equal(Indexes, indexes_of_whole))
        {
            return layout;
        }
        else if constexpr(indexical<L>)
        {
            return detail::array_cat(layout, Indexes);
        }
        else
        {
            return detail::sublayout<detail::array_drop<1uz>(Indexes)>(detail::layout_get<Indexes[0]>(layout));
        }
    }
    
    template<typename TLayout, size_t N>
    constexpr auto layout_add_prefix(const TLayout& layout, const array<size_t, N>& prefix)
    {
        if constexpr(detail::indexical_array<TLayout>)
        {
            return detail::array_cat(prefix, layout);
        }
        else return[&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(detail::layout_add_prefix(get<I>(layout), prefix)...);
        }(std::make_index_sequence<size<TLayout>>{});
    }
    
    template<indexical auto Indices, class Shape>
    struct normalize_indices
    {
        static consteval auto get_value()
        {
            if constexpr(std::integral<decltype(Indices)>)
            {
                return array{ detail::normalize_index(Indices, size<Shape>) };
            }
            else return []<size_t...I>(std::index_sequence<I...>)
            {
                return array<size_t, Indices.size()>
                {
                    detail::normalize_index(Indices[I], size<subtree_t<Shape, detail::array_take<I>(Indices)>>)...
                };
            }(std::make_index_sequence<Indices.size()>{});
        }

        static constexpr auto value = get_value();
    };

    template<indexical auto Indices, class Shape>
    inline constexpr auto normalize_indices_v = normalize_indices<Indices, Shape>::value;

    // template<indexical auto Indices, class Shape>
    // constexpr auto normalize_indices(Shape shape = {})
    // {
    //     if constexpr(std::integral<decltype(Indices)>)
    //     {
    //         return array{ detail::normalize_index(Indices, size<Shape>) };
    //     }
    //     else return []<size_t...I>(std::index_sequence<I...>)
    //     {
    //         return array<size_t, Indices.size()>
    //         {
    //             detail::normalize_index(Indices[I], size<subtree_t<Shape, detail::array_take<I>(Indices)>>)...
    //         };
    //     }(std::make_index_sequence<Indices.size()>{});
    // }

    template<typename T>
    constexpr auto default_unfolded_layout()
    {
        if constexpr (terminal<T>)
        {
            return indexes_of_whole;
        }
        else return[]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(detail::layout_add_prefix(detail::default_unfolded_layout<subtree_t<T&, I>>(), array{I})...);
        }(std::make_index_sequence<size<T>>{});    
    };

    template<auto Layout, class Shape>
    struct fold_layout
    {
        static consteval auto get_value()
        {
            if constexpr(indexical<decltype(Layout)>)
            {
                return detail::normalize_indices_v<Layout, Shape>;
            }
            else return []<size_t...I>(std::index_sequence<I...>)
            {
                constexpr auto child_relayout = make_tuple(detail::fold_layout<subtree<I>(Layout), Shape>::value...);
                constexpr size_t n = size<decltype(get<0uz>(child_relayout))>;

                if constexpr(n > 0uz
                    && (... && detail::indexical_array<decltype(get<I>(child_relayout))>)
                    && (... && (n == size<decltype(get<I>(child_relayout))>))
                )
                {
                    constexpr auto prefix = detail::array_take<n - 1uz>(get<0uz>(child_relayout));
                    if constexpr(size<subtree_t<Shape, prefix>> == size<decltype(Layout)>
                        && (... && (prefix == detail::array_take<n - 1uz>(get<I>(child_relayout))))
                        && (... && (get<I>(child_relayout)[n - 1uz] == I))
                    )
                    {
                        return prefix;
                    }
                    else
                    {
                        return child_relayout;
                    }
                }
                else
                {
                    return child_relayout;
                }
            }(std::make_index_sequence<size<decltype(Layout)>>{});
        }

        static constexpr auto value = get_value();
    };

    template<auto Layout, class Shape>
    inline constexpr auto fold_layout_v = fold_layout<Layout, Shape>::value;

    // template<auto Layout, class Shape>
    // constexpr auto fold_layout(Shape shape = {})
    // {
    //     if constexpr(indexical<decltype(Layout)>)
    //     {
    //         return detail::normalize_indices<Layout>(shape);
    //     }
    //     else return []<size_t...I>(std::index_sequence<I...>)
    //     {
    //         constexpr auto child_relayout = make_tuple(detail::fold_layout<subtree<I>(Layout)>(Shape{})...);
    //         constexpr size_t n = size<decltype(get<0uz>(child_relayout))>;

    //         if constexpr(n > 0uz
    //             && (... && detail::indexical_array<decltype(get<I>(child_relayout))>)
    //             && (... && (n == size<decltype(get<I>(child_relayout))>))
    //         )
    //         {
    //             constexpr auto prefix = detail::array_take<n - 1uz>(get<0uz>(child_relayout));
    //             if constexpr(size<subtree_t<Shape, prefix>> == size<decltype(Layout)>
    //                 && (... && (prefix == detail::array_take<n - 1uz>(get<I>(child_relayout))))
    //                 && (... && (get<I>(child_relayout)[n - 1uz] == I))
    //             )
    //             {
    //                 return prefix;
    //             }
    //             else
    //             {
    //                 return child_relayout;
    //             }
    //         }
    //         else
    //         {
    //             return child_relayout;
    //         }
    //     }(std::make_index_sequence<size<decltype(Layout)>>{});
    // }

    template<auto Layout, class BaseShape>
    constexpr auto unfold_layout(BaseShape base_shape = {})
    {
        if constexpr(indexical<decltype(Layout)>)
        {
            constexpr auto indexes = detail::normalize_indices_v<Layout, BaseShape>;
            using subshape_t = subtree_t<BaseShape, indexes>;
            if constexpr(terminal<subshape_t>)
            {
                return indexes;
            }
            else return [&]<size_t...I>(std::index_sequence<I...>)
            {
                constexpr auto indexes = detail::normalize_indices_v<Layout, BaseShape>;
                return make_tuple(detail::unfold_layout<detail::array_cat(indexes, array{ I })>(base_shape)...);
            }(std::make_index_sequence<size<subshape_t>>{});
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(detail::unfold_layout<get<I>(Layout)>(base_shape)...);
        }(std::make_index_sequence<size<decltype(Layout)>>{});
    }

    template<auto Layout, class T>
    constexpr auto relayout_tag_tree(const T tag_tree)
    {
        if constexpr(indexical<decltype(Layout)>)
        {
            return detail::tag_subtree<Layout>(tag_tree);
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(detail::relayout_tag_tree<get<I>(Layout)>(tag_tree)...);
        }(std::make_index_sequence<size<decltype(Layout)>>{});
    }

    template<class S1, class S2>
    constexpr auto merge_stricture_tree(const S1& tree1, const S2& tree2)
    {
        if constexpr(std::same_as<S1, stricture_t>)
        {
            if constexpr(std::same_as<S2, stricture_t>)
            {
                return tree1 & tree2;
            }
            else
            {
                return detail::merge_stricture_tree(tree2, tree1);
            }
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        { 
            //static_assert(size<S1> > 0);
            if constexpr(std::same_as<S2, stricture_t>)
            {
                return make_tuple(detail::merge_stricture_tree(get<I>(tree1), tree2)...);
            }
            else
            {
                return make_tuple(detail::merge_stricture_tree(get<I>(tree1), get<I>(tree2))...);
            }
        }(std::make_index_sequence<size<S1>>{});
    }

    template<auto Tree1, class S2>
    constexpr auto merge_stricture_tree(const S2& tree2)
    {
        if constexpr(std::same_as<decltype(Tree1), stricture_t>)
        {
            if constexpr(Tree1 == stricture_t::readonly)
            {
                return stricture_t::readonly;
            }
            else
            {
                return tree2;
            }
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        { 
            //static_assert(size<S1> > 0);
            if constexpr(std::same_as<S2, stricture_t>)
            {
                return make_tuple(detail::merge_stricture_tree<get<I>(Tree1)>(tree2)...);
            }
            else
            {
                return make_tuple(detail::merge_stricture_tree<get<I>(Tree1)>(get<I>(tree2))...);
            }
        }(std::make_index_sequence<size<decltype(Tree1)>>{});
    }

    template<auto Tree1, auto Tree2>
    constexpr auto merge_stricture_tree()
    {
        if constexpr(std::same_as<decltype(Tree1), stricture_t>)
        {
            if constexpr(Tree1 == stricture_t::readonly)
            {
                return stricture_t::readonly;
            }
            else
            {
                return Tree2;
            }
        }
        else if constexpr(std::same_as<decltype(Tree2), stricture_t>)
        {
            if constexpr(Tree2 == stricture_t::readonly)
            {
                return stricture_t::readonly;
            }
            else
            {
                return Tree1;
            }
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        { 
            return make_tuple(detail::merge_stricture_tree<get<I>(Tree1), get<I>(Tree2)>()...);
        }(std::make_index_sequence<size<decltype(Tree1)>>{});
    }

    template<auto TagTree>
    constexpr auto fold_tag_tree()
    {
        if constexpr(terminal<decltype(TagTree)>)
        {
            return TagTree;
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto subresults = make_tuple(detail::fold_tag_tree<subtree<I>(TagTree)>()...);
            if constexpr((terminal<decltype(get<0uz>(subresults))> && ... && detail::equal(get<I>(subresults), get<0uz>(subresults))))
            {
                return get<0uz>(subresults);
            }
            else
            {
                return subresults;
            }
        }(std::make_index_sequence<size<decltype(TagTree)>>{});
    }
    
    template<class Shape, class T>
    constexpr auto replicate(const T& value, Shape shape)
    {
        if constexpr (terminal<Shape>)
        {
            return value;
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(detail::replicate(value, get<I>(shape))...);
        }(std::make_index_sequence<size<Shape>>{});
    }

    template<typename TagTree, typename S>
    constexpr auto unfold_tag_tree(TagTree tree, S = {})
    {
        if constexpr(terminal<TagTree>)
        {
            return detail::replicate(tree, S{});
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(detail::unfold_tag_tree(get<I>(tree), get<I>(S{}))...);
        }(std::make_index_sequence<size<TagTree>>{});
    }
    
    template<auto OperationTree>
    constexpr auto fold_operation_tree()
    {
        if constexpr(std::same_as<decltype(OperationTree), operation_t>)
        {
            return OperationTree;
        }
        else return[]<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto subresults = make_tuple(detail::fold_operation_tree<subtree<I>(OperationTree)>()...);
            if constexpr((... && detail::equal(get<I>(subresults), operation_t::none)))
            {
                return operation_t::none;
            }
            else
            {
                return subresults;
            }
        }(std::make_index_sequence<size<decltype(OperationTree)>>{});
    }

    template<class U>
    constexpr usage_t fold_usage(const U& usage_tree)
    {
        if constexpr(std::same_as<U, usage_t>)
        {
            return usage_tree;
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return (... & detail::fold_usage(get<I>(usage_tree)));
        }(std::make_index_sequence<size<U>>{});
    }

    template<auto FoldedOperationTree, class U>
    constexpr auto fit_operation_usage_impl(const U& usage_tree, bool& need_plain)
    {
        if constexpr(std::same_as<decltype(FoldedOperationTree), operation_t>)
        {
            if constexpr(FoldedOperationTree == operation_t::none)
            {
                return usage_tree;
            }
            else
            {
                usage_t usage = detail::fold_usage(usage_tree);
                need_plain = need_plain || usage == usage_t::repeatedly;
                return usage;
            }
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(
                detail::fit_operation_usage_impl<get<I>(FoldedOperationTree)>(detail::tag_tree_get<I>(usage_tree), need_plain)...
            );
        }(std::make_index_sequence<size<decltype(FoldedOperationTree)>>{});
    }

    template<auto FoldedOperationTree, class U>
    constexpr auto fit_operation_usage(const U& usage_tree)
    {
        bool need_plain = false;

        struct result_t
        {
            decltype(detail::fit_operation_usage_impl<FoldedOperationTree>(usage_tree, need_plain)) usage_tree;
            bool need_plain;
        };

        return result_t{ detail::fit_operation_usage_impl<FoldedOperationTree>(usage_tree, need_plain), need_plain };
    }
}

namespace senluo::detail
{
    template<class DataStrictureTree>
    constexpr auto set_data_stricture_tree(DataStrictureTree& tree)
    {
        if constexpr(std::same_as<DataStrictureTree, stricture_t>)
        {
            tree = stricture_t::readonly;
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., detail::set_data_stricture_tree(get<I>(tree)));
        }(std::make_index_sequence<size<DataStrictureTree>>{});
    }

    template<auto Layout, class DataStrictureTree>
    constexpr auto set_data_stricture_tree_by_layout(DataStrictureTree& tree)
    {
        if constexpr(indexical<decltype(Layout)>)
        {
            detail::set_data_stricture_tree(subtree<Layout>(tree));
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., detail::set_data_stricture_tree_by_layout<get<I>(Layout)>(tree));
        }(std::make_index_sequence<size<decltype(Layout)>>{});
    }

    template<auto RawStrictureTree, auto IndependenceTree, auto Layout, auto UsageTree, class CurDataStrictureTree>
    constexpr auto get_inverse_sequence_stricture_tree_impl(CurDataStrictureTree& cur_tree)
    {
        if constexpr(detail::equal(IndependenceTree, independence_t::safe))
        {
            return stricture_t::none;
        }
        else if constexpr(std::same_as<decltype(UsageTree), usage_t>)
        {
            if constexpr(UsageTree == usage_t::none)
            {
                return stricture_t::none;
            }
            else if constexpr(detail::equal(RawStrictureTree, stricture_t::readonly))
            {
                detail::set_data_stricture_tree_by_layout<Layout>(cur_tree);
                return stricture_t::readonly;
            }
            else
            {
                auto result = detail::merge_stricture_tree<RawStrictureTree>(detail::relayout_tag_tree<Layout>(cur_tree));
                detail::set_data_stricture_tree_by_layout<Layout>(cur_tree);
                return result;
            }
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return tuple<decltype(detail::get_inverse_sequence_stricture_tree_impl<detail::tag_tree_get<I>(RawStrictureTree)
                                                        , detail::tag_tree_get<I>(IndependenceTree)
                                                        , detail::layout_get<I>(Layout)
                                                        , get<I>(UsageTree)>(cur_tree))...>
            {
                detail::get_inverse_sequence_stricture_tree_impl<detail::tag_tree_get<I>(RawStrictureTree)
                                                        , detail::tag_tree_get<I>(IndependenceTree)
                                                        , detail::layout_get<I>(Layout)
                                                        , get<I>(UsageTree)>(cur_tree)...
            };
        }(std::make_index_sequence<size<decltype(UsageTree)>>{});
    }

    template<auto RawStrictureTree, auto IndependenceTree, auto Layout, auto UsageTree, class CurDataStrictureTree>
    constexpr auto get_sequence_stricture_tree_impl(CurDataStrictureTree& cur_tree)
    {
        if constexpr(detail::equal(IndependenceTree, independence_t::safe))
        {
            return stricture_t::none;
        }
        else if constexpr(std::same_as<decltype(UsageTree), usage_t>)
        {
            if constexpr(UsageTree == usage_t::none)
            {
                return stricture_t::none;
            }
            else if constexpr(detail::equal(RawStrictureTree, stricture_t::readonly))
            {
                detail::set_data_stricture_tree_by_layout<Layout>(cur_tree);
                return stricture_t::readonly;
            }
            else
            {
                auto result = detail::merge_stricture_tree<RawStrictureTree>(detail::relayout_tag_tree<Layout>(cur_tree));
                detail::set_data_stricture_tree_by_layout<Layout>(cur_tree);
                return result;
            }
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            static_assert(size<decltype(UsageTree)> > 0);
            constexpr size_t last = size<decltype(UsageTree)> - 1uz;
            auto inverse_result = tuple<decltype(detail::get_sequence_stricture_tree_impl<detail::tag_tree_get<last - I>(RawStrictureTree)
                                                        , detail::tag_tree_get<last - I>(IndependenceTree)
                                                        , detail::layout_get<last - I>(Layout)
                                                        , get<last - I>(UsageTree)>(cur_tree))...>
            {
                detail::get_sequence_stricture_tree_impl<detail::tag_tree_get<last - I>(RawStrictureTree)
                                                        , detail::tag_tree_get<last - I>(IndependenceTree)
                                                        , detail::layout_get<last - I>(Layout)
                                                        , get<last - I>(UsageTree)>(cur_tree)...
            };
            return make_tuple(get<last - I>(inverse_result)...);
        }(std::make_index_sequence<size<decltype(UsageTree)>>{});
    }

    // template<auto RawStrictureTree, auto Layout, auto UsageTree, class DataShape>
    // constexpr auto get_inverse_sequence_stricture_tree(DataShape data_shape = {})
    // {
    //     auto data_stricture_tree = detail::replicate(stricture_t::none, data_shape);
    //     return get_inverse_sequence_stricture_tree_impl<RawStrictureTree, Layout, UsageTree>(data_stricture_tree);
    // }

    // template<auto RawStrictureTree, auto Layout, auto UsageTree, class DataShape>
    // constexpr auto get_sequence_stricture_tree(DataShape data_shape = {})
    // {
    //     auto data_stricture_tree = detail::replicate(stricture_t::none, data_shape);
    //     return detail::get_sequence_stricture_tree_impl<RawStrictureTree, Layout, UsageTree>(data_stricture_tree);
    // }
}

namespace senluo::detail
{
    template<auto Layout, size_t Depth = 0uz, typename T>
    constexpr bool inverse_layout_index_len_at(T& result_index_len, size_t& setted_count)
    {
        if constexpr(detail::indexical_array<decltype(Layout)>)
        {
            if(subtree<Layout>(result_index_len) == invalid_index)
            {
                subtree<Layout>(result_index_len) = Depth;
                ++setted_count;
                return true;
            }
            else
            {
                return false;
            }
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {   
            return (... && detail::inverse_layout_index_len_at<get<I>(Layout), Depth + 1uz>(result_index_len, setted_count));
        }(std::make_index_sequence<size<decltype(Layout)>>{});
    }

    template<class S, class T>
    constexpr auto make_tree_of_same_value_and_set_leaf_count(const T& value, size_t& count, S = {})
    {
        if constexpr(terminal<S>)
        {
            ++count;
            return value;
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(detail::make_tree_of_same_value_and_set_leaf_count(value, count, get<I>(S{}))...);
        }(std::make_index_sequence<size<S>>{});
    }

    template<auto Layout, typename S>
    constexpr auto inverse_layout_index_len(S = {})
    {
        size_t leaf_count = 0uz;
        auto index_len_tree = detail::make_tree_of_same_value_and_set_leaf_count(invalid_index, leaf_count, S{});
        size_t setted_count = 0uz;
        bool is_injective = detail::inverse_layout_index_len_at<Layout>(index_len_tree, setted_count);

        layout_mapping_type_t mapping_type;
        if(not is_injective)
        {
            mapping_type = layout_mapping_type_t::multi_shot;
        }
        else if(leaf_count != setted_count)
        {
            mapping_type = layout_mapping_type_t::injective;
        }
        else
        {
            mapping_type = layout_mapping_type_t::bijective;
        }

        struct result_t
        {
            layout_mapping_type_t mapping_type;
            decltype(index_len_tree) index_len_tree;
        };
        return result_t{ mapping_type, index_len_tree  };
    }

    template<auto IndexLenTree>
    constexpr auto init_layout()
    {
        if constexpr(std::same_as<decltype(IndexLenTree), size_t>)
        {
            return array<size_t, IndexLenTree>{};
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(detail::init_layout<get<I>(IndexLenTree)>()...);
        }(std::make_index_sequence<size<decltype(IndexLenTree)>>{});
    }

    template<auto Layout, auto CurIndex = indexes_of_whole, typename R>
    constexpr void inverse_layout_at(R& result)
    {
        if constexpr(detail::indexical_array<decltype(Layout)>)
        {
            subtree<Layout>(result) = CurIndex;
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {   
            return (..., detail::inverse_layout_at<get<I>(Layout), detail::array_cat(CurIndex, array{ I })>(result));
        }(std::make_index_sequence<size<decltype(Layout)>>{});
    }

    template<auto UnFoldedLayout, typename S>
    constexpr auto get_layout_cache(S = {})
    {
        constexpr auto inverse_layout_index_len_result = detail::inverse_layout_index_len<UnFoldedLayout, S>();
        
        if constexpr(inverse_layout_index_len_result.mapping_type == layout_mapping_type_t::bijective)
        {
            auto inverse_layout = detail::init_layout<inverse_layout_index_len_result.index_len_tree>();
            detail::inverse_layout_at<UnFoldedLayout>(inverse_layout);
            struct layout_chache_t
            {
                layout_mapping_type_t mapping_type;
                decltype(inverse_layout) inverse_layout;
            };
            return layout_chache_t{ inverse_layout_index_len_result.mapping_type, inverse_layout };
        }
        else
        {
            struct layout_chache_t
            {
                layout_mapping_type_t mapping_type;
            };
            return layout_chache_t{ inverse_layout_index_len_result.mapping_type };
        }
    }
}

namespace senluo::detail
{
    template<class Shape>
    constexpr auto make_tree_of_bool_false_and_set_count(size_t& count, Shape)
    {
        if(terminal<Shape>)
        {
            ++count;
            return false; 
        }
        else return[&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(detail::make_tree_of_bool_false_and_set_count(count, get<I>(Shape{}))...);
        }(std::make_index_sequence<size<Shape>>{});
    }

    template<auto UnfoldedLayout, class UsedFlagTree>
    constexpr bool is_injective_impl(UsedFlagTree& used_flag_tree, size_t& used_count)
    {
        if constexpr(indexical_array<decltype(UnfoldedLayout)>)
        {
            bool& used_flag = subtree<UnfoldedLayout>(used_flag_tree);
            if(used_flag == true)
            {
                return false;
            }
            else
            {
                used_flag_tree = true;
                ++used_count;
                return true;
            }
        }
        else return[&]<size_t...I>(std::index_sequence<I...>)
        {
            return (... && detail::is_injective_impl<get<I>(UnfoldedLayout)>(used_flag_tree));
        }(std::make_index_sequence<size<decltype(UnfoldedLayout)>>{});
    }

    template<auto UnfoldedLayout, class BaseShape>
    constexpr layout_mapping_type_t layout_mapping_type_of(BaseShape = {})
    {
        size_t leaf_count = 0;
        auto used_flag_tree = make_tree_of_bool_false_and_set_count(leaf_count, BaseShape{});
        size_t used_count = 0;
        bool is_injective = is_injective_impl<UnfoldedLayout>(used_flag_tree, used_count);
        if(not is_injective)
        {
            return layout_mapping_type_t::multi_shot;
        }
        if(used_count != leaf_count)
        {
            return layout_mapping_type_t::injective;
        }
        return layout_mapping_type_t::bijective;
    }

    template<class T>
    struct intrinsic_stricture_tree
    {
        static consteval auto get_value()
        {
            if constexpr(is_only_input_v<T>)
            {
                return stricture_t::readonly;
            }
            else if constexpr(terminal<T>)
            {
                return stricture_t::none;
            }
            else return []<size_t...I>(std::index_sequence<I...>)
            {
                return make_tuple(intrinsic_stricture_tree<tree_get_t<I, T>>::value...);
            }(std::make_index_sequence<size<T>>{});
        }

        static constexpr auto value = get_value();
    };

    template<class T>
    inline constexpr auto intrinsic_stricture_tree_v = intrinsic_stricture_tree<T>::value;

    template<auto BaseStrictureTree, class BaseUsedCountTree>
    constexpr void increase_used_count_tree(BaseUsedCountTree& count_tree, BaseUsedCountTree& inc_tree)
    {
        if constexpr(detail::equal(BaseStrictureTree, stricture_t::readonly))
        {
            return;
        }
        else if constexpr(std::same_as<BaseUsedCountTree, size_t>)
        {
            if(inc_tree == 0uz)
            {
                ++count_tree;
                inc_tree = 1uz;
            }
        }
        else [&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., increase_used_count_tree<detail::tag_tree_get<I>(BaseStrictureTree)>(get<I>(count_tree), get<I>(inc_tree)));
        }(std::make_index_sequence<size<BaseUsedCountTree>>{});
    }

    template<auto Indexes, auto BaseStrictureTree, class BaseUsedCountTree>
    constexpr void increase_used_count_tree_by_indexes(BaseUsedCountTree& count_tree, BaseUsedCountTree& inc_tree)
    {
        if constexpr(detail::equal(detail::tag_subtree<Indexes>(BaseStrictureTree), stricture_t::readonly))
        {
            return;
        }
        else if constexpr(std::same_as<BaseUsedCountTree, size_t>)
        {
            if(inc_tree == 0uz)
            {
                ++count_tree;
                inc_tree = 1uz;
            }
        }
        else if constexpr(Indexes.size() == 0uz)
        {
            increase_used_count_tree<BaseStrictureTree>(count_tree, inc_tree);
        }
        else 
        {   
            auto& sub_tree = get<Indexes[0]>(count_tree);
            auto& sub_inc_tree = get<Indexes[0]>(inc_tree);
            increase_used_count_tree_by_indexes<detail::array_drop<1uz>(Indexes),
                                                    detail::tag_tree_get<Indexes[0]>(BaseStrictureTree)>(
                    sub_tree, sub_inc_tree
                );
        }
    }

    template<auto Layout, auto BaseStrictureTree, class BaseUsedCountTree>
    constexpr void increase_used_count_tree_by_layout(BaseUsedCountTree& count_tree, BaseUsedCountTree& inc_tree)
    {
        if constexpr(indexical_array<decltype(Layout)>)
        {
            increase_used_count_tree_by_indexes<Layout, BaseStrictureTree>(count_tree, inc_tree);
        }
        else [&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., increase_used_count_tree_by_layout<get<I>(Layout), BaseStrictureTree>(count_tree, inc_tree));
        }(std::make_index_sequence<size<decltype(Layout)>>{});
    }

    template<auto Layout, auto UsageTree, auto BaseStrictureTree, class BaseUsedCountTree>
    constexpr auto get_used_order_and_set_count(BaseUsedCountTree& count_tree)
    {
        if constexpr(std::same_as<decltype(UsageTree), usage_t>)
        {
            if constexpr(UsageTree == usage_t::none)
            {
                return 0uz;
            }
            else
            {
                auto result = detail::relayout_tag_tree<Layout>(count_tree);
                BaseUsedCountTree inc_tree{};
                increase_used_count_tree_by_layout<Layout, BaseStrictureTree>(count_tree, inc_tree);
                return result;
            }
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return tuple<decltype(detail::get_used_order_and_set_count<detail::layout_get<I>(Layout)
                                                        , get<I>(UsageTree)
                                                        , BaseStrictureTree>(count_tree))...>
            {
                detail::get_used_order_and_set_count<detail::layout_get<I>(Layout)
                                                        , get<I>(UsageTree)
                                                        , BaseStrictureTree>(count_tree)...
            };
        }(std::make_index_sequence<size<decltype(UsageTree)>>{});
    }

    template<auto BaseStrictureTree, class BaseShape>
    constexpr auto make_used_count_tree(BaseShape = {})
    {
        if constexpr(std::same_as<decltype(BaseStrictureTree), stricture_t>)
        {
            if constexpr(BaseStrictureTree == stricture_t::readonly)
            {
                return 0uz;
            }
            else
            {
                return detail::replicate(0uz, BaseShape{});
            }
        }
        else return[]<size_t...I>(std::index_sequence<I...>)
        {
            static_assert(size<decltype(BaseStrictureTree)> > 0);
            return make_tuple(detail::make_used_count_tree<get<I>(BaseStrictureTree)>(get<I>(BaseShape{}))...);
        }(std::make_index_sequence<size<decltype(BaseStrictureTree)>>{});
    }

    template<auto Layout, auto UsageTree, auto BaseStrictureTree, class BaseShape>
    constexpr auto get_used_order_and_count()
    {
        auto used_count_tree = make_used_count_tree<BaseStrictureTree, BaseShape>();
        auto used_order_tree = get_used_order_and_set_count<Layout, UsageTree, BaseStrictureTree>(used_count_tree);
        struct used_order_and_count_t
        {
            decltype(used_order_tree) used_order_tree;
            decltype(used_count_tree) used_count_tree;
        };
        return used_order_and_count_t{ used_order_tree, used_count_tree };
    }

    template<auto Layout, auto UsageTree, auto BaseStrictureTree, class BaseShape>
    inline constexpr auto used_order_and_count = detail::get_used_order_and_count<Layout, UsageTree, BaseStrictureTree, BaseShape>();

    template<auto Layout, auto UsedOrderTree, auto UsedCountTree>
    struct sequence_stricture
    {
        static consteval auto get_value()
        {
            if constexpr(std::same_as<decltype(UsedOrderTree), size_t>)
            {
                static_assert(indexical_array<decltype(Layout)>);
                static_assert(std::same_as<decltype(detail::tag_subtree<Layout>(UsedCountTree)), size_t>);
                constexpr size_t used_count = detail::tag_subtree<Layout>(UsedCountTree);
                if constexpr(used_count == 0uz || UsedOrderTree == used_count - 1uz)
                {
                    return stricture_t::none;
                }
                else
                {
                    return stricture_t::readonly;
                }
            }
            else return[]<size_t...I>(std::index_sequence<I...>)
            {
                constexpr auto substrictures = make_tuple(
                    sequence_stricture<detail::layout_get<I>(Layout), get<I>(UsedOrderTree), UsedCountTree>::value...
                );
                if constexpr((std::same_as<decltype(get<0uz>(substrictures)), const stricture_t&> && ... && detail::equal(get<I>(substrictures), get<0uz>(substrictures))))
                {
                    return get<0uz>(substrictures);
                }
                else
                {
                    return substrictures;
                }
            }(std::make_index_sequence<size<decltype(UsedOrderTree)>>{});
        }

        static constexpr auto value = get_value();
    };

    template<auto Layout, auto UsageTree, auto BaseStrictureTree, class BaseShape>
    inline constexpr auto sequence_stricture_v = 
        sequence_stricture<
            Layout, 
            used_order_and_count<Layout, UsageTree, BaseStrictureTree, BaseShape>.used_order_tree, 
            used_order_and_count<Layout, UsageTree, BaseStrictureTree, BaseShape>.used_count_tree
        >::value;

    template<auto UsedOrderTree>
    struct inverse_sequence_stricture
    {
        static consteval auto get_value()
        {
            if constexpr(std::same_as<decltype(UsedOrderTree), size_t>)
            {
                if constexpr(UsedOrderTree == 0uz)
                {
                    return stricture_t::none;
                }
                else
                {
                    return stricture_t::readonly;
                }
            }
            else return[]<size_t...I>(std::index_sequence<I...>)
            {
                constexpr auto substrictures = make_tuple(
                    inverse_sequence_stricture<get<I>(UsedOrderTree)>::value...
                );
                if constexpr((std::same_as<decltype(get<0uz>(substrictures)), const stricture_t&> && ... && detail::equal(get<I>(substrictures), get<0uz>(substrictures))))
                {
                    return get<0uz>(substrictures);
                }
                else
                {
                    return substrictures;
                }
            }(std::make_index_sequence<size<decltype(UsedOrderTree)>>{});
        }

        static constexpr auto value = get_value();
    };

    template<auto Layout, auto UsageTree, auto BaseStrictureTree, class BaseShape>
    inline constexpr auto inverse_sequence_stricture_v = 
        inverse_sequence_stricture<used_order_and_count<Layout, UsageTree, BaseStrictureTree, BaseShape>.used_order_tree>::value;

    template<auto Layout, auto UsedCountTree>
    struct seperate_stricture
    {
        static consteval auto get_value()
        {
            if constexpr(indexical_array<decltype(Layout)>)
            {
                constexpr auto used_count = detail::tag_subtree<Layout>(UsedCountTree);
                if constexpr(std::same_as<decltype(used_count), const size_t>)
                {
                    if constexpr(used_count <= 1uz)
                    {
                        return stricture_t::none;
                    }
                    else
                    {
                        return stricture_t::readonly;
                    }
                }
                else return []<size_t...I>(std::index_sequence<I...>)
                {
                    constexpr auto used_count = detail::tag_subtree<Layout>(UsedCountTree);
                    constexpr auto substrictures = make_tuple(
                        seperate_stricture<array{ I }, used_count>::value...
                    );
                    if constexpr((std::same_as<decltype(get<0uz>(substrictures)), const stricture_t&> && ... && detail::equal(get<I>(substrictures), get<0uz>(substrictures))))
                    {
                        return get<0uz>(substrictures);
                    }
                    else
                    {
                        return substrictures;
                    }
                }(std::make_index_sequence<size<decltype(used_count)>>{});
            }
            else return[]<size_t...I>(std::index_sequence<I...>)
            {
                constexpr auto substrictures = make_tuple(
                    seperate_stricture<detail::layout_get<I>(Layout), UsedCountTree>::value...
                );
                if constexpr((std::same_as<decltype(get<0uz>(substrictures)), const stricture_t&> && ... && detail::equal(get<I>(substrictures), get<0uz>(substrictures))))
                {
                    return get<0uz>(substrictures);
                }
                else
                {
                    return substrictures;
                }
            }(std::make_index_sequence<size<decltype(Layout)>>{});
        }

        static constexpr auto value = get_value();
    };

    template<auto Layout, auto UsageTree, auto BaseStrictureTree, class BaseShape>
    inline constexpr auto seperate_stricture_v = 
        seperate_stricture<Layout, used_order_and_count<Layout, UsageTree, BaseStrictureTree, BaseShape>.used_count_tree>::value;
}

namespace senluo::detail 
{
    template<auto Layout, auto BaseLayout, class BaseShape>
    constexpr auto relayout_layout(BaseShape = {})
    {
        if constexpr(indexical_array<decltype(BaseLayout)>)
        {
            if constexpr(BaseLayout.size() == 0uz)
            {
                return Layout;
            }
            else
            {
                return detail::layout_add_prefix(Layout, BaseLayout);
            }
        }
        else if constexpr(indexical_array<decltype(Layout)>)
        {
            if constexpr(Layout.size() == 0uz)
            {
                return BaseLayout;
            }
            else
            {
                return detail::relayout_layout<detail::array_drop<1uz>(Layout), get<Layout[0]>(BaseLayout), BaseShape>();
            }
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto sublayouts = make_tuple(detail::relayout_layout<get<I>(Layout), BaseLayout, BaseShape>()...);
            constexpr size_t n = size<decltype(get<0uz>(sublayouts))>;

            if constexpr(n > 0uz
                && (... && detail::indexical_array<decltype(get<I>(sublayouts))>)
                && (... && (n == size<decltype(get<I>(sublayouts))>))
            )
            {
                constexpr auto prefix = detail::array_take<n - 1uz>(get<0uz>(sublayouts));
                if constexpr(size<subtree_t<BaseShape, prefix>> == size<decltype(Layout)>
                    && (... && (prefix == detail::array_take<n - 1uz>(get<I>(sublayouts))))
                    && (... && (get<I>(sublayouts)[n - 1uz] == I))
                )
                {
                    return prefix;
                }
                else
                {
                    return sublayouts;
                }
            }
            else
            {
                return sublayouts;
            }
        }(std::make_index_sequence<size<decltype(Layout)>>{});
    }

    template<auto Layout, auto Tree>
    constexpr auto relayout_stricture_tree()
    {
        if constexpr(std::same_as<decltype(Tree), stricture_t>)
        {
            return Tree;
        }
        else if constexpr(indexical<decltype(Layout)>)
        {
            return detail::tag_subtree<Layout>(Tree);
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto subtrees = make_tuple(detail::relayout_stricture_tree<get<I>(Layout), Tree>()...);
            if constexpr((std::same_as<decltype(get<0uz>(subtrees)), const stricture_t&> && ... && detail::equal(get<I>(subtrees), get<0uz>(subtrees))))
            {
                return get<0uz>(subtrees);
            }
            else
            {
                return subtrees;
            }
        }(std::make_index_sequence<size<decltype(Layout)>>{});
    }

    //todo... should change to discard repeat.
    template<auto FoldedLayout, class O>
    constexpr bool is_enable_to_relayout_operation_tree(const O& folded_operation_tree)
    {
        if constexpr(std::same_as<O, operation_t>)
        {
            if(folded_operation_tree == operation_t::none)
            {
                return true;
            }
        }
        if constexpr(indexical<decltype(FoldedLayout)>)
        {
            if constexpr(FoldedLayout.size() == 0uz)
            {
                return true;
            }
            else
            {
                //msvc bug
                constexpr auto i = detail::array_take<FoldedLayout.size() - 1uz>(FoldedLayout);
                return not std::same_as<decltype(detail::tag_subtree<i>(folded_operation_tree)), operation_t>
                || detail::equal(detail::tag_subtree<FoldedLayout>(folded_operation_tree), operation_t::none);
            }
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return (... && detail::is_enable_to_relayout_operation_tree<get<I>(FoldedLayout)>(folded_operation_tree));
        }(std::make_index_sequence<size<decltype(FoldedLayout)>>{});
    };

    template<auto Layout, auto Tree>
    constexpr auto relayout_operation_tree()
    {
        if constexpr(std::same_as<decltype(Tree), operation_t>)
        {
            return Tree;
        }
        else if constexpr(indexical<decltype(Layout)>)
        {
            return detail::tag_subtree<Layout>(Tree);
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto subtrees = make_tuple(detail::relayout_operation_tree<get<I>(Layout), Tree>()...);
            if constexpr((... && detail::equal(get<I>(subtrees), operation_t::none)))
            {
                return operation_t::none;
            }
            else
            {
                return subtrees;
            }
        }(std::make_index_sequence<size<decltype(Layout)>>{});
    }

    template<typename UsageTree>
    constexpr void set_usage_tree(UsageTree& usage_tree, usage_t usage)
    {
        if constexpr(std::same_as<UsageTree, usage_t>)
        {
            usage_tree = usage_tree & usage;
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., detail::set_usage_tree(get<I>(usage_tree), usage));
        }(std::make_index_sequence<size<UsageTree>>{});
    }

    template<auto Indexes, typename U, typename R>
    constexpr void inverse_usage_tree_at(const U& usage_tree, R& result)
    {
        if constexpr(std::same_as<U, usage_t>)
        {
            detail::set_usage_tree(result | subtree<Indexes>, usage_tree);
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., detail::inverse_usage_tree_at<detail::array_cat(Indexes, array{I})>(get<I>(usage_tree), result));
        }(std::make_index_sequence<size<U>>{});
    }

    template<auto UnfoldedLayout, typename U, typename R>
    constexpr void inverse_relayout_usage_tree_at(const U& usage_tree, R& result)
    {
        if constexpr(indexical<decltype(UnfoldedLayout)>)
        {
            detail::inverse_usage_tree_at<UnfoldedLayout>(usage_tree, result);
        }
        else return[&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., detail::inverse_relayout_usage_tree_at<get<I>(UnfoldedLayout)>(detail::tag_tree_get<I>(usage_tree), result));
        }(std::make_index_sequence<size<decltype(UnfoldedLayout)>>{});
    }

    //return an unfolded usage tree
    template<auto UnfoldedLayout, typename U, typename BaseShape>
    constexpr auto inverse_relayout_usage_tree(const U& usage_tree, BaseShape = {})
    {
        auto result = detail::replicate(usage_t::none, BaseShape{});
        detail::inverse_relayout_usage_tree_at<UnfoldedLayout>(usage_tree, result);
        return result;
    }

    template<auto UsageTree, class Shape>
    constexpr auto unfold_usage_when_used()
    {
        if constexpr(std::same_as<decltype(UsageTree), usage_t>)
        {
            if constexpr(UsageTree == usage_t::none)
            {
                return usage_t::none;
            }
            else
            {
                return detail::replicate<Shape>(UsageTree);
            }
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(detail::unfold_usage_when_used<get<I>(UsageTree), std::tuple_element_t<I, Shape>>()...);
        }(std::make_index_sequence<size<decltype(UsageTree)>>{});
    }

    template<auto UsageTree>
    constexpr auto fold_usage_when_unused()
    {
        if constexpr(terminal<decltype(UsageTree)>)
        {
            return UsageTree;
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            constexpr auto subresults = make_tuple(detail::fold_tag_tree<subtree<I>(UsageTree)>()...);
            if constexpr((... && detail::equal(get<I>(subresults), usage_t::none)))
            {
                return usage_t::none;
            }
            else
            {
                return subresults;
            }
        }(std::make_index_sequence<size<decltype(UsageTree)>>{});
    }
}

#include "../tools/macro_undef.hpp"
#endif