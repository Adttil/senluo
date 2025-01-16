#ifndef RUZHOUXIE_OPERATE_HPP
#define RUZHOUXIE_OPERATE_HPP

#include "tree.hpp"
#include "constant.hpp"
#include "general.hpp"
#include "principle.hpp"
#include "wrap.hpp"
#include "relayout.hpp"

#include "macro_define.hpp"

namespace senluo 
{
    template<class U>
    constexpr usage_t fold_usage(const U& usage_tree)
    {
        if constexpr(std::same_as<U, usage_t>)
        {
            return usage_tree;
        }
        else return [&]<size_t...I>(std::index_sequence<I...>)
        {
            return (... & senluo::fold_usage(usage_tree | subtree<I>));
        }(std::make_index_sequence<size<U>>{});
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
    template<operation_t operation>
    constexpr auto get_operation_functor();
}

namespace senluo 
{
    template<typename TBasePrinciple, auto OperationTree>
    struct operate_principle : based_on<TBasePrinciple>, principle_interface<operate_principle<TBasePrinciple, OperationTree>>
    {
        constexpr decltype(auto) data(this auto&& self)
        {
            return FWD(self, base).data();
        }
        
        static constexpr auto layout()
        {
            return TBasePrinciple::layout();
        }
        
        static constexpr auto stricture_tree()
        { 
            return TBasePrinciple::stricture_tree();
        }
        
        static constexpr auto operation_tree()
        { 
            return OperationTree;
        }
    };

    template<typename T, auto OperationTree>
    struct operate_tree : based_on<T>, standard_interface<operate_tree<T, OperationTree>>
    {
        template<size_t I, typename Self> 
        constexpr decltype(auto) get(this Self&& self)
        {
            constexpr auto op_subtree = tag_tree_get<I>(OperationTree);
            if constexpr(I >= size<decltype(OperationTree)>)
            {
                return end();
            }
            else if constexpr(not std::same_as<decltype(op_subtree), const operation_t>)
            {
                return operate_tree<decltype(FWD(self, base) | subtree<I>), op_subtree>
                {
                    FWD(self, base) | subtree<I>
                };
            }
            else if constexpr(op_subtree == operation_t::none)
            {
                return FWD(self, base) | subtree<I>;
            }
            else
            {
                return get_operation_functor<tag_tree_get<I>(OperationTree)>()(FWD(self, base) | subtree<I>);
            }
        }

        template<auto UsageTree, bool NoCopy, typename Self>
        constexpr auto principle(this Self&& self)
        {
            constexpr auto fitted_usage_result = fit_operation_usage<OperationTree>(UsageTree);
            constexpr auto fittedd_usage = fitted_usage_result.usage_tree;
            constexpr bool need_plain = fitted_usage_result.need_plain;

            using base_principle_t = decltype(FWD(self, base) | senluo::principle<fittedd_usage, NoCopy>);
            
            if constexpr(equal(base_principle_t::operation_tree(), operation_t::none))
            {
                if constexpr(not need_plain)
                {
                    return operate_principle<base_principle_t, OperationTree>{ 
                        FWD(self, base) | senluo::principle<fittedd_usage, NoCopy> 
                    };
                }
                else
                {
                    using base_plain_principle_t = plain_principle<decltype(FWD(self, base) | plainize<fittedd_usage>)>;
                    return operate_principle<base_plain_principle_t, OperationTree>{ 
                        FWD(self, base) | plainize<fittedd_usage>
                    };
                }
            }
            else
            {
                auto base_plain = plain_principle<decltype(FWD(self, base) | plainize<fittedd_usage>)>{
                    FWD(self, base) | plainize<fittedd_usage> 
                };
                
                auto raw_principle = operate_principle<decltype(base_plain), OperationTree>{ std::move(base_plain) };
                if constexpr(not need_plain)
                {
                    return operate_principle<decltype(base_plain), OperationTree>{ std::move(base_plain) };
                }
                else
                {
                    return plain_principle<decltype(std::move(raw_principle) | plainize<fittedd_usage>)>{
                        std::move(raw_principle) | plainize<fittedd_usage>
                    };
                }
            }
        }
    };

    namespace detail
    {
        template<auto OperationTree>
        struct operate_t : adaptor_closure<operate_t<OperationTree>>
        {
            template<typename T>
            constexpr decltype(auto) operator()(T&& t)const
            {
                if constexpr(not std::same_as<decltype(OperationTree), operation_t>)
                {
                    return operate_tree<senluo::unwrap_t<T>, OperationTree>{ unwrap_fwd(FWD(t)) };
                }
                else if constexpr(OperationTree == operation_t::none)
                {
                    return decltype(wrap(FWD(t))){ unwrap_fwd(FWD(t)) };
                }
                else
                {
                    return decltype(wrap(get_operation_functor<OperationTree>()(FWD(t)))){ 
                        unwrap_fwd(get_operation_functor<OperationTree>()(FWD(t))) 
                    };
                }
            }
        };
    }

    template<auto OperationTree>
    inline constexpr detail::operate_t<OperationTree> operate{};
}

namespace senluo
{
    namespace detail
    {
        struct transform_t : adaptor<transform_t>
        {
            template<class ArgTable, class Fn>
            constexpr decltype(auto) adapt(ArgTable&& arg_table, Fn&& fn)const
            {
                constexpr size_t n = size<ArgTable>;
                return zip(FWD(fn) | repeat<n>, FWD(arg_table)) 
                    | operate<make_tree_of_same_value(operation_t::apply_invoke, shape<array<size_t, n>>)>;
            }
        };
    }
    
    inline constexpr detail::transform_t transform{};
    
    namespace detail
    {
        struct zip_transform_t
        {
            template<class Fn, class Args, class...Rest>
            constexpr decltype(auto) operator()(Fn&& fn, Args&& args, Rest&&...rest)const
            {
                constexpr size_t n = size<Args>;
                return zip(FWD(fn) | repeat<n>, FWD(args), FWD(rest)...) 
                    | operate<make_tree_of_same_value(operation_t::apply_invoke, shape<array<size_t, n>>)>;
            }
        };
    }

    inline constexpr detail::zip_transform_t zip_transform{};
}

#include "macro_undef.hpp"
#endif