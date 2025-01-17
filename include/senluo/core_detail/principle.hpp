#ifndef SENLUO_PRINCIPLE_HPP
#define SENLUO_PRINCIPLE_HPP

#include "../general.hpp"
#include "../tree.hpp"
#include "wrap.hpp"
#include "tag.hpp"

#include "../macro_define.hpp"

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