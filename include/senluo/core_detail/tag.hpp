#ifndef SENLUO_TAG_HPP
#define SENLUO_TAG_HPP

#include "../general.hpp"
#include "../tree.hpp"

#include "../macro_define.hpp"

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

    template<size_t I, class T>
    constexpr auto tag_tree_get(const T& tag_tree)
    {
        if constexpr(terminal<T>)
        {
            return tag_tree;
        }
        else
        {
            return tag_tree | subtree<I>;
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
            return layout | subtree<I>;
        }
        else
        {
            return invalid_index;
        }
    }

    template<indexical_array auto indexes, class T>
    constexpr auto tag_subtree(const T& tag_tree)
    {
        if constexpr(detail::equal(indexes, indexes_of_whole))
        {
            return tag_tree;
        }
        else
        {
            return tag_subtree<detail::array_drop<1uz>(indexes)>(tag_tree_get<indexes[0]>(tag_tree));
        }
    }

    template<typename TLayout, size_t N>
    constexpr auto layout_add_prefix(const TLayout& layout, const array<size_t, N>& prefix)
    {
        if constexpr(indexical_array<TLayout>)
        {
            return detail::array_cat(prefix, layout);
        }
        else return[&]<size_t...I>(std::index_sequence<I...>)
        {
            return senluo::make_tuple(senluo::layout_add_prefix(layout | subtree<I>, prefix)...);
        }(std::make_index_sequence<size<TLayout>>{});
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
            constexpr auto subresults = senluo::make_tuple(fold_tag_tree<TagTree | subtree<I>>()...);
            if constexpr((... && detail::equal(subresults | subtree<I>, subresults | subtree<0uz>)))
            {
                return subresults | subtree<0uz>;
            }
            else
            {
                return subresults;
            }
        }(std::make_index_sequence<size<decltype(TagTree)>>{});
    }

    template<typename TagTree, typename S>
    constexpr auto unfold_tag_tree(TagTree tree, S shape = {})
    {
        if constexpr(terminal<TagTree>)
        {
            return senluo::make_tree_of_same_value(tree, shape);
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return senluo::make_tuple(senluo::unfold_tag_tree(tree | subtree<I>, shape | subtree<I>)...);
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
            constexpr auto subresults = senluo::make_tuple(fold_operation_tree<OperationTree | subtree<I>>()...);
            if constexpr((... && detail::equal(subresults | subtree<I>, operation_t::none)))
            {
                return operation_t::none;
            }
            else
            {
                return subresults;
            }
        }(std::make_index_sequence<size<decltype(OperationTree)>>{});
    }

    template<auto OperationTree, class U>
    constexpr auto fit_operation_usage_impl(const U& usage_tree, bool& need_plain)
    {
        if constexpr(std::same_as<decltype(OperationTree), operation_t>)
        {
            if constexpr(OperationTree == operation_t::none)
            {
                return usage_tree;
            }
            else
            {
                usage_t usage = fold_usage(usage_tree);
                need_plain = need_plain || usage == usage_t::repeatedly;
                return usage;
            }
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return senluo::make_tuple(fit_operation_usage_impl<OperationTree | subtree<I>>(tag_tree_get<I>(usage_tree), need_plain)...);
        }(std::make_index_sequence<size<decltype(OperationTree)>>{});
    }

    template<auto OperationTree, class U>
    constexpr auto fit_operation_usage(const U& usage_tree)
    {
        bool need_plain = false;

        struct result_t
        {
            decltype(fit_operation_usage_impl<OperationTree>(usage_tree, need_plain)) usage_tree;
            bool need_plain;
        };

        return result_t{ fit_operation_usage_impl<OperationTree>(usage_tree, need_plain), need_plain };
    }
}

namespace senluo 
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
            (..., set_data_stricture_tree(tree | subtree<I>));
        }(std::make_index_sequence<size<DataStrictureTree>>{});
    }

    template<auto Layout, class DataStrictureTree>
    constexpr auto set_data_stricture_tree_by_layout(DataStrictureTree& tree)
    {
        if constexpr(indexical<decltype(Layout)>)
        {
            set_data_stricture_tree(tree | subtree<Layout>);
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., set_data_stricture_tree_by_layout<Layout | subtree<I>>(tree));
        }(std::make_index_sequence<size<decltype(Layout)>>{});
    }

    template<auto RawStrictureTree, auto Layout, auto UsageTree, class CurDataStrictureTree>
    constexpr auto get_inverse_sequence_stricture_tree_impl(CurDataStrictureTree& cur_tree)
    {
        if constexpr(std::same_as<decltype(UsageTree), usage_t>)
        {
            if constexpr(UsageTree == usage_t::none)
            {
                return stricture_t::none;
            }
            else if constexpr(detail::equal(RawStrictureTree, stricture_t::readonly))
            {
                set_data_stricture_tree_by_layout<Layout>(cur_tree);
                return stricture_t::readonly;
            }
            else
            {
                auto result = merge_stricture_tree(RawStrictureTree, relayout_tag_tree<Layout>(cur_tree));
                set_data_stricture_tree_by_layout<Layout>(cur_tree);
                return result;
            }
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return tuple<decltype(get_inverse_sequence_stricture_tree_impl<tag_tree_get<I>(RawStrictureTree)
                                                        , layout_get<I>(Layout)
                                                        , UsageTree | subtree<I>>(cur_tree))...>
            {
                get_inverse_sequence_stricture_tree_impl<tag_tree_get<I>(RawStrictureTree)
                                                        , layout_get<I>(Layout)
                                                        , UsageTree | subtree<I>>(cur_tree)...
            };
        }(std::make_index_sequence<size<decltype(UsageTree)>>{});
    }

    template<auto RawStrictureTree, auto Layout, auto UsageTree, class CurDataStrictureTree>
    constexpr auto get_sequence_stricture_tree_impl(CurDataStrictureTree& cur_tree)
    {
        if constexpr(std::same_as<decltype(UsageTree), usage_t>)
        {
            if constexpr(UsageTree == usage_t::none)
            {
                return stricture_t::none;
            }
            else if constexpr(detail::equal(RawStrictureTree, stricture_t::readonly))
            {
                set_data_stricture_tree_by_layout<Layout>(cur_tree);
                return stricture_t::readonly;
            }
            else
            {
                static_assert(not std::same_as<decltype(RawStrictureTree), tuple<>>);
                static_assert(not std::same_as<decltype(relayout_tag_tree<Layout>(cur_tree)), tuple<>>);
                auto result = merge_stricture_tree(RawStrictureTree, relayout_tag_tree<Layout>(cur_tree));
                set_data_stricture_tree_by_layout<Layout>(cur_tree);
                //static_assert(not std::same_as<decltype(result), tuple<>>);
                return result;
            }
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            static_assert(size<decltype(UsageTree)> > 0);
            constexpr size_t last = size<decltype(UsageTree)> - 1uz;
            auto inverse_resilt = tuple<decltype(get_sequence_stricture_tree_impl<tag_tree_get<last - I>(RawStrictureTree)
                                                        , layout_get<last - I>(Layout)
                                                        , UsageTree | subtree<last - I>>(cur_tree))...>
            {
                get_sequence_stricture_tree_impl<tag_tree_get<last - I>(RawStrictureTree)
                                                        , layout_get<last - I>(Layout)
                                                        , UsageTree | subtree<last - I>>(cur_tree)...
            };
            return make_tuple(inverse_resilt | subtree<last - I>...);
        }(std::make_index_sequence<size<decltype(UsageTree)>>{});
    }

    template<auto RawStrictureTree, auto Layout, auto UsageTree, class DataShape>
    constexpr auto get_inverse_sequence_stricture_tree(DataShape data_shape = {})
    {
        auto data_stricture_tree = make_tree_of_same_value(stricture_t::none, data_shape);
        return get_inverse_sequence_stricture_tree_impl<RawStrictureTree, Layout, UsageTree>(data_stricture_tree);
    }

    template<auto RawStrictureTree, auto Layout, auto UsageTree, class DataShape>
    constexpr auto get_sequence_stricture_tree(DataShape data_shape = {})
    {
        auto data_stricture_tree = make_tree_of_same_value(stricture_t::none, data_shape);
        return get_sequence_stricture_tree_impl<RawStrictureTree, Layout, UsageTree>(data_stricture_tree);
    }
}

namespace senluo 
{
    template<auto Layout, size_t Depth = 0uz, typename T>
    constexpr bool inverse_layout_index_len_at(T& result_index_len, size_t& setted_count)
    {
        if constexpr(indexical_array<decltype(Layout)>)
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
            return (... && inverse_layout_index_len_at<Layout | subtree<I>, Depth + 1uz>(result_index_len, setted_count));
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
            return senluo::make_tuple(senluo::make_tree_of_same_value_and_set_leaf_count(value, count, S{} | subtree<I>)...);
        }(std::make_index_sequence<size<S>>{});
    }

    template<auto Layout, typename S>
    constexpr auto inverse_layout_index_len(S = {})
    {
        size_t leaf_count = 0uz;
        auto index_len_tree = senluo::make_tree_of_same_value_and_set_leaf_count(invalid_index, leaf_count, S{});
        size_t setted_count = 0uz;
        bool is_injective = senluo::inverse_layout_index_len_at<Layout>(index_len_tree, setted_count);
        struct result_t
        {
            bool is_surjection;
            decltype(index_len_tree) index_len_tree;
        };
        return result_t{ is_injective && (leaf_count == setted_count), index_len_tree  };
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
            return senluo::make_tuple(init_layout<IndexLenTree | subtree<I>>()...);
        }(std::make_index_sequence<size<decltype(IndexLenTree)>>{});
    }

    template<auto Layout, auto CurIndex = indexes_of_whole, typename R>
    constexpr void inverse_layout_at(R& result)
    {
        if constexpr(indexical_array<decltype(Layout)>)
        {
            subtree<Layout>(result) = CurIndex;
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {   
            return (..., inverse_layout_at<Layout | subtree<I>, detail::array_cat(CurIndex, array{ I })>(result));
        }(std::make_index_sequence<size<decltype(Layout)>>{});
    }

    template<auto UnFoldedLayout, typename S>
    constexpr auto inverse_layout(S = {})
    {
        constexpr auto inverse_layout_index_len_result = inverse_layout_index_len<UnFoldedLayout, S>();
        
        if constexpr(inverse_layout_index_len_result.is_surjection)
        {
            auto result = init_layout<inverse_layout_index_len_result.index_len_tree>();
            inverse_layout_at<UnFoldedLayout>(result);
            return result;
        }
        else
        {
            return tuple{};
        }
    }
}

#include "../macro_undef.hpp"
#endif