#ifndef RUZHOUXIE_PRETREATMENT_HPP
#define RUZHOUXIE_PRETREATMENT_HPP

#include "tree.hpp"
#include "general.hpp"
#include "principle.hpp"
#include "astrict.hpp"
#include "relayout.hpp"
#include "operate.hpp"

#include "macro_define.hpp"

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
            else if constexpr(equal(RawStrictureTree, stricture_t::readonly))
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
            else if constexpr(equal(RawStrictureTree, stricture_t::readonly))
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
    template<class Pretreater>
    struct pretreater_interface : adaptor_closure<Pretreater>
    {
        template<typename T, derived_from<Pretreater> Self>
        constexpr auto operator()(this Self&& self, T&& tree)
        {
            constexpr auto usage = Pretreater::usage_tree();

            auto principle = FWD(tree) | senluo::principle<usage>;

            constexpr auto data_shape = shape<decltype(std::move(principle).data())>;
            
            constexpr auto layout = principle.layout();
            constexpr auto raw_stricture_tree = principle.stricture_tree();
            constexpr auto stricture_tree = Pretreater::template pretreat_stricture<raw_stricture_tree, layout>(data_shape);
            constexpr auto operation_tree = principle.operation_tree();

            return decltype(std::move(principle).data() | relayout<layout> | astrict<stricture_tree> | operate<operation_tree>)
            {
                std::move(principle).data()
            };
            //return std::move(principle).data() | relayout<layout> | astrict<stricture_tree> | operate<operation_tree>;
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

            template<auto RawStrictureTree, auto Layout, class S>
            static consteval auto pretreat_stricture(S data_shape)
            {
                return get_sequence_stricture_tree<RawStrictureTree, Layout, UsageTree>(data_shape);
            }
        };

        template<auto UsageTree>
        struct inverse_sequence_by_usage_t : pretreater_interface<inverse_sequence_by_usage_t<UsageTree>>
        {
            static consteval auto usage_tree()
            {
                return UsageTree;
            }

            template<auto RawStrictureTree, auto Layout, class S>
            static consteval auto pretreat_stricture(S data_shape)
            {
                return get_inverse_sequence_stricture_tree<RawStrictureTree, Layout, UsageTree>(data_shape);
            }
        };

        template<auto UsageTree>
        struct seperate_by_usage_t : pretreater_interface<seperate_by_usage_t<UsageTree>>
        {
            static consteval auto usage_tree()
            {
                return UsageTree;
            }

            template<auto RawStrictureTree, auto Layout, class S>
            static consteval auto pretreat_stricture(S data_shape)
            {
                constexpr auto seq_stricture_tree = get_sequence_stricture_tree<RawStrictureTree, Layout, UsageTree>(S{});
                constexpr auto inv_stricture_tree = get_inverse_sequence_stricture_tree<RawStrictureTree, Layout, UsageTree>(S{});

                return merge_stricture_tree(seq_stricture_tree, inv_stricture_tree);
            }
        };
    }

    template<auto UsageTree>
    inline constexpr detail::sequence_by_usage_t<UsageTree> sequence_by_usage{};

    template<auto UsageTree>
    inline constexpr detail::inverse_sequence_by_usage_t<UsageTree> inverse_sequence_by_usage{};

    template<auto UsageTree>
    inline constexpr detail::seperate_by_usage_t<UsageTree> seperate_by_usage{};

    template<class Tree>
    inline constexpr detail::sequence_by_usage_t<make_tree_of_same_value(usage_t::once, shape<Tree>)> sequence_by_shape{};

    template<class Tree>
    inline constexpr detail::inverse_sequence_by_usage_t<make_tree_of_same_value(usage_t::once, shape<Tree>)> inverse_sequence_by_shape{};

    template<class Tree>
    inline constexpr detail::seperate_by_usage_t<make_tree_of_same_value(usage_t::once, shape<Tree>)> seperate_by_shape{};

    namespace detail
    {
        struct sequence_t : adaptor_closure<sequence_t>
        {
            template<branched T>
            constexpr decltype(auto) operator()(T&& tree)const
            {
                constexpr auto usage_tree = make_tree_of_same_value(usage_t::once, shape<array<int, size<T>>>);
                return FWD(tree) | sequence_by_usage<usage_tree>;
            }

            template<terminal T>
            constexpr tuple<> operator()(T&& t)const
            {
                return {};
            }
        };

        struct inverse_sequence_t : adaptor_closure<inverse_sequence_t>
        {
            template<branched T>
            constexpr decltype(auto) operator()(T&& tree)const
            {
                constexpr auto usage_tree = make_tree_of_same_value(usage_t::once, shape<array<int, size<T>>>);
                return FWD(tree) | inverse_sequence_by_usage<usage_tree>;
            }

            template<terminal T>
            constexpr tuple<> operator()(T&& t)const
            {
                return {};
            }
        };

        struct seperate_t : adaptor_closure<seperate_t>
        {
            template<branched T>
            constexpr decltype(auto) operator()(T&& tree)const
            {
                constexpr auto usage_tree = make_tree_of_same_value(usage_t::once, shape<array<int, size<T>>>);
                return FWD(tree) | seperate_by_usage<usage_tree>;
            }

            template<terminal T>
            constexpr tuple<> operator()(T&& t)const
            {
                return {};
            }
        };
    }

    inline constexpr detail::sequence_t sequence{};
    
    inline constexpr detail::inverse_sequence_t inverse_sequence{};

    inline constexpr detail::seperate_t seperate{};
}

namespace senluo
{
    namespace detail 
    {
        struct apply_t : adaptor<apply_t>
        {
            template<class Args, class Fn>
            constexpr decltype(auto) adapt(Args&& args, Fn&& fn)const
            {
                return [&]<size_t...I>(std::index_sequence<I...>) -> decltype(auto)
                {
                    auto&& seperate_args = FWD(args) | refer | seperate;
                    return FWD(fn)(FWD(seperate_args) | subtree<I>...);
                }(std::make_index_sequence<size<Args>>{});
            }
        };
        
        struct apply_invoke_t : adaptor_closure<apply_invoke_t>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& tree)const
            {
                return [&]<size_t...I>(std::index_sequence<I...>) -> decltype(auto)
                {
                    auto&& seperate_tree = FWD(tree) | refer | seperate;
                    return (FWD(seperate_tree) | subtree<0uz>)(FWD(seperate_tree) | subtree<I + 1>...);
                }(std::make_index_sequence<size<T> - 1uz>{});
            }
        };
    }

    inline constexpr detail::apply_t apply{};

    inline constexpr detail::apply_invoke_t apply_invoke{};

    template<operation_t operation>
    constexpr auto get_operation_functor()
    {
        if constexpr(operation == operation_t::none)
        {
            return pass;
        }
        else if constexpr(operation == operation_t::apply_invoke)
        {
            return apply_invoke;
        }
    }
}

namespace senluo 
{
    template<auto UsageTree>
    struct detail::to_plain_principle_t : adaptor_closure<to_plain_principle_t<UsageTree>>
    {
        template<class TPrinciple>
        constexpr auto operator()(TPrinciple&& principle)const
        {
            constexpr auto data_shape = shape<decltype(std::move(principle).data())>;
                
            constexpr auto layout = principle.layout();
            constexpr auto raw_stricture_tree = principle.stricture_tree();
            constexpr auto stricture_tree = get_sequence_stricture_tree<raw_stricture_tree, layout, UsageTree>(data_shape);
            constexpr auto operation_tree = principle.operation_tree();

            static_assert(not std::is_rvalue_reference_v<decltype(unwrap(std::move(principle).data()))>);
            decltype(auto) src = std::move(principle).data() | relayout<layout> | astrict<stricture_tree> | operate<operation_tree>;
            static_assert(not std::is_rvalue_reference_v<decltype((std::move(principle).data() | relayout<layout>).base)>);

            return plain_principle<decltype(make_data(FWD(src)))>{ make_data(FWD(src)) };
        }

        template<class TSrc>
        constexpr decltype(auto) make_data(TSrc&& src)const
        {
            if constexpr(equal(UsageTree, usage_t::none))
            {
                return tuple{};
            }
            else if constexpr(terminal<TSrc>)
            {
                return (TSrc)FWD(src);
            }
            else return [&]<size_t...I>(std::index_sequence<I...>)
            {
                return tuple<decltype(to_plain_principle<tag_tree_get<I>(UsageTree)>.make_data(FWD(src) | subtree<I>))...>
                {
                    to_plain_principle<tag_tree_get<I>(UsageTree)>.make_data(FWD(src) | subtree<I>)...
                };
            }(std::make_index_sequence<size<TSrc>>{});

            // if constexpr(std::same_as<decltype(UsageTree), usage_t>)
            // {
            //     return std::decay_t<TSrc>{ FWD(src) };
            // }
            // else return [&]<size_t...I>(std::index_sequence<I...>)
            // {
            //     return tuple<decltype(to_plain_principle<UsageTree | subtree<I>>.make_data(FWD(src) | relayout<I>))...>
            //     {
            //         to_plain_principle<UsageTree | subtree<I>>.make_data(FWD(src) | relayout<I>)...
            //     };
            // }(std::make_index_sequence<size<decltype(UsageTree)>>{});
        }
    };
}

#include "macro_undef.hpp"
#endif