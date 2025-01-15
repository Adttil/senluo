#ifndef SENLUO_PRINCIPLE_HPP
#define SENLUO_PRINCIPLE_HPP

#include "general.hpp"
#include "tree.hpp"
#include "standard.hpp"

#include "macro_define.hpp"

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
    };

    inline constexpr detail::pass_t pass{};
}

namespace senluo
{
    template<typename PrinciledTree>
    struct principle_interface : standard_interface<PrinciledTree>{};

    template<typename T>
    concept principled = requires(std::remove_cvref_t<T>& t)
    {
        { []<class U>(principle_interface<U>&)->U*{}(t) } -> std::same_as<std::remove_cvref_t<T>*>;
        std::declval<T>().data();
        std::remove_cvref_t<T>::layout();
        std::remove_cvref_t<T>::stricture_tree();
        std::remove_cvref_t<T>::operation_tree();
    };

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
                return (no_custom && ... && is_plain<senluo::subtree_t<T, I>, tag_tree_get<I>(UsageTree)>());
            }(std::make_index_sequence<size<T>>{});
        }
    }
    
    template<auto UsageTree, bool NoCopy = false>
    inline constexpr detail::principle_t_ns::principle_fn<UsageTree, NoCopy> principle;
    
    template<typename T, auto UsageTree>
    using principle_t = decltype(std::declval<T>() | principle<UsageTree>);

    template<class T, auto UsageTree>
    concept plain = (detail::principle_t_ns::is_plain<T, UsageTree>());

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
        template<auto UsageTree, template<class...> class Tpl>
        struct plainize_fn;
    }

    template<auto UsageTree = usage_t::once, template<class...> class Tpl = tuple>
    inline constexpr detail::plainize_fn<UsageTree, Tpl> plainize{};
}

#include "macro_undef.hpp"
#endif