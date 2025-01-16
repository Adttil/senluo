#ifndef SENLUO_PRINCIPLE_HPP
#define SENLUO_PRINCIPLE_HPP

#include "../general.hpp"
#include "../tree.hpp"
#include "wrap.hpp"

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
            return senluo::array_cat(layout, array{ I });
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
        if constexpr(senluo::equal(indexes, indexes_of_whole))
        {
            return tag_tree;
        }
        else
        {
            return tag_subtree<array_drop<1uz>(indexes)>(tag_tree_get<indexes[0]>(tag_tree));
        }
    }

    template<typename TLayout, size_t N>
    constexpr auto layout_add_prefix(const TLayout& layout, const array<size_t, N>& prefix)
    {
        if constexpr(indexical_array<TLayout>)
        {
            return senluo::array_cat(prefix, layout);
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
            if constexpr((... && senluo::equal(subresults | subtree<I>, subresults | subtree<0uz>)))
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
            if constexpr((... && senluo::equal(subresults | subtree<I>, operation_t::none)))
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
    namespace detail
    {
        struct pass_t : adaptor_closure<pass_t>
        {
            template<class T>
            constexpr T operator()(T&& t)const
            {
                return FWD(t);
            }
        };

        template<auto Layout>
        struct relayout_t;

        template<auto StrictureTree>
        struct astrict_t;

        template<auto OperationTree>
        struct operate_t;
    }
    
    inline constexpr detail::pass_t pass{};

    template<auto Layout>
    inline constexpr detail::relayout_t<Layout> relayout{};

    template<auto StrictureTree>
    inline constexpr detail::astrict_t<fold_tag_tree<StrictureTree>()> astrict{};

    template<auto OperationTree>
    inline constexpr detail::operate_t<fold_operation_tree<OperationTree>()> operate{};
}

namespace senluo 
{
    template<typename PrinciledTree>
    struct principle_interface;

    namespace detail::principle_t_ns 
    {
        template<auto UsageTree, bool NoCopy>
        struct principle_fn;

        template<auto UsageTree>
        void principle();

        template<class T, auto UsageTree>
        constexpr bool is_plain()
        {
            constexpr bool no_custom = not bool
            {
                requires{ std::declval<unwrap_t<T>>().template principle<UsageTree, false>(); }
                ||
                requires{ principle<UsageTree, false>(std::declval<unwrap_t<T>>()); }
            };
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                return (no_custom && ... && is_plain<subtree_t<T, I>, tag_tree_get<I>(UsageTree)>());
            }(std::make_index_sequence<size<T>>{});
        }
    }
    
    template<auto UsageTree, bool NoCopy = false>
    inline constexpr detail::principle_t_ns::principle_fn<UsageTree, NoCopy> principle;
    
    template<typename T, auto UsageTree>
    using principle_t = decltype(std::declval<T>() | principle<UsageTree>);

    template<class T, auto UsageTree>
    concept plain = (detail::principle_t_ns::is_plain<T, UsageTree>());

    template<typename T>
    concept principled = requires(std::remove_cvref_t<T>& t)
    {
        { []<class U>(principle_interface<U>&)->U*{}(t) } -> std::same_as<std::remove_cvref_t<T>*>;
        std::declval<T>().data();
        std::remove_cvref_t<T>::layout();
        std::remove_cvref_t<T>::stricture_tree();
        std::remove_cvref_t<T>::operation_tree();
    };

    template<class Pretreater>
    struct pretreater_interface : adaptor_closure<Pretreater>
    {
        constexpr decltype(auto) operator()(this auto&& self, auto&& tree)
        {
            return self(FWD(tree) | principle<Pretreater::usage_tree()>);
        }

        constexpr decltype(auto) operator()(this auto&&, principled auto&& tree)
        {
            constexpr auto data_shape = shape<decltype(FWD(tree).data())>;
            
            constexpr auto layout = tree.layout();
            constexpr auto raw_stricture_tree = tree.stricture_tree();
            constexpr auto stricture_tree = Pretreater::template pretreat_stricture<raw_stricture_tree, layout>(data_shape);
            constexpr auto operation_tree = tree.operation_tree();

            return decltype(FWD(tree).data() | relayout<layout> | astrict<stricture_tree> | operate<operation_tree>)
            {
                FWD(tree).data()
            };
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
    namespace detail
    {
        template<auto UsageTree, template<class...> class Tpl>
        struct plainize_fn : adaptor_closure<plainize_fn<UsageTree, Tpl>>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& tree) const
            {
                return impl(FWD(tree) | sequence_by_usage<senluo::unfold_tag_tree(UsageTree, shape<T>)>);
            }

            template<class TSrc>
            constexpr decltype(auto) impl(TSrc&& src)const
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
                    return tuple<decltype(plainize_fn<tag_tree_get<I>(UsageTree), Tpl>{}.impl(FWD(src) | subtree<I>))...>
                    {
                        plainize_fn<tag_tree_get<I>(UsageTree), Tpl>{}.impl(FWD(src) | subtree<I>)...
                    };
                }(std::make_index_sequence<size<TSrc>>{});
            }
        };
    }
    
    template<auto UsageTree = usage_t::once, template<class...> class Tpl = tuple>
    inline constexpr detail::plainize_fn<UsageTree, Tpl> plainize{};
    
    template<template<class...> class Tpl = tuple>
    constexpr const auto& to()
    {
        return plainize<usage_t::once, Tpl>;
    }
}

namespace senluo
{        
    template<typename PrinciledTree>
    struct principle_interface : standard_interface<PrinciledTree>
    {
        // template<auto UsageTree, bool NoCopy, typename Self>
        // constexpr decltype(auto) principle(this Self&& self)
        // {
        //     using self_t = std::remove_cvref_t<Self>;
        //     constexpr auto fitted_usage_result = fit_operation_usage<self_t::operation_tree()>(UsageTree);
        //     constexpr auto fittedd_usage = fitted_usage_result.usage_tree;
        //     constexpr bool need_plain = fitted_usage_result.need_plain;

        //     if constexpr(need_plain)
        //     {
        //         return FWD(self) | plainize;
        //     }
        // }
    };
    
    struct null_principle : principle_interface<null_principle>
    {
        constexpr auto data() const
        {
            return std::in_place_t{};
        }
        
        static constexpr auto layout(){ return indexes_of_whole; }
        
        static constexpr auto stricture_tree(){ return stricture_t::none; }
        
        static constexpr auto operation_tree(){ return operation_t::none; }
    };

    template<class T>
    struct plain_principle : based_on<T>, principle_interface<plain_principle<T>>
    {
        constexpr decltype(auto) data(this auto&& self)
        {
            return pass(FWD(self, base));
        }
        
        static constexpr auto layout(){ return indexes_of_whole; }
        
        static constexpr auto stricture_tree(){ return stricture_t::none; }
        
        static constexpr auto operation_tree(){ return operation_t::none; }
    };

    template<class T, auto UsageTree>
    struct trivial_principle : based_on<T>, principle_interface<trivial_principle<T, UsageTree>>
    {        
        constexpr auto data(this auto&& self)
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                return tuple<decltype((FWD(self, base) | subtree<I> | principle<tag_tree_get<I>(UsageTree)>).data())...>
                {
                    (FWD(self, base) | subtree<I> | principle<tag_tree_get<I>(UsageTree)>).data()...
                };
            }(std::make_index_sequence<size<T>>{});
        }

        static constexpr auto layout()
        {
            constexpr auto raw = []<size_t...I>(std::index_sequence<I...>)
            {
                return senluo::make_tuple(
                    senluo::layout_add_prefix(principle_t<subtree_t<T, I>, tag_tree_get<I>(UsageTree)>::layout(), array{ I })...
                );
            }(std::make_index_sequence<size<T>>{});
            return fold_layout<raw>(shape<decltype(std::declval<trivial_principle>().data())>);
        }

        static constexpr auto stricture_tree()
        {
            constexpr auto raw = []<size_t...I>(std::index_sequence<I...>)
            {
                return senluo::make_tuple(
                    principle_t<subtree_t<T, I>, tag_tree_get<I>(UsageTree)>::stricture_tree()...
                );
            }(std::make_index_sequence<size<T>>{});
            return fold_tag_tree<raw>();
        }

        static constexpr auto operation_tree()
        {
            constexpr auto raw = []<size_t...I>(std::index_sequence<I...>)
            {
                return senluo::make_tuple(
                    principle_t<subtree_t<T, I>, tag_tree_get<I>(UsageTree)>::operation_tree()...
                );
            }(std::make_index_sequence<size<T>>{});
            return fold_operation_tree<raw>();
        }
    };

    template<auto UsageTree, bool NoCopy>
    struct detail::principle_t_ns::principle_fn : adaptor_closure<principle_fn<UsageTree, NoCopy>>
    {
        template<typename T>
        constexpr auto operator()(T&& tree)const
        {
            if constexpr(NoCopy)
            {
                return FWD(tree) | refer | senluo::principle<UsageTree>;
            }
            else
            {
                return impl<unwrap_t<T>>(unwrap_fwd(FWD(tree)));
            }
        }

        template<typename T>
        constexpr auto impl(T&& tree)const
        {
            constexpr bool no_copy = std::is_rvalue_reference_v<T>;
            if constexpr(senluo::equal(UsageTree, usage_t::none))
            {
                return null_principle{};
            }
            else if constexpr(requires{ { FWD(tree).template principle<UsageTree, no_copy>() } -> principled; })
            {
                return FWD(tree).template principle<UsageTree, no_copy>();
            }
            else if constexpr(requires{ { principle<UsageTree, no_copy>(FWD(tree)) } -> principled; })
            {
                return principle<UsageTree, no_copy>(FWD(tree));
            }
            else if constexpr(plain<T, UsageTree>)
            {                
                return plain_principle<T>{ FWD(tree) };
            }
            else
            { 
                return trivial_principle<T, UsageTree>{ FWD(tree) };
            }
        }
    };

    

    namespace detail
    {
        template<auto UsageTree, bool NoCopy, template<class...> class Tpl>
        struct plainize_principle_fn : adaptor_closure<plainize_principle_fn<UsageTree, NoCopy, Tpl>>
        {
            template<class T>
            constexpr decltype(auto) operator()(T&& tree)const
            {
                if constexpr(NoCopy)
                {
                    return plain_principle<decltype(FWD(tree) | refer | plainize<UsageTree, Tpl>)>{ 
                        FWD(tree) | refer | plainize<UsageTree, Tpl> 
                    };
                }
                else
                {
                    return plain_principle<decltype(FWD(tree) | plainize<UsageTree, Tpl>)>{ 
                        FWD(tree) | plainize<UsageTree, Tpl> 
                    };
                }
            }
        };
    };

    template<auto UsageTree = usage_t::once, bool NoCopy = false, template<class...> class Tpl = tuple>
    inline constexpr detail::plainize_principle_fn<UsageTree, NoCopy, Tpl> plainize_principle{};
}

#include "../macro_undef.hpp"
#endif