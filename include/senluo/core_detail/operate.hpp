#ifndef RUZHOUXIE_OPERATE_HPP
#define RUZHOUXIE_OPERATE_HPP

#include "../general.hpp"
#include "principle.hpp"
#include "wrap.hpp"
#include "tree.hpp"
#include "relayout.hpp"

#include "../macro_define.hpp"

namespace senluo 
{
    
}

namespace senluo::detail::operate_ns 
{
    template<typename TBasePrinciple, auto OperationTree>
    struct principle_t : based_on<TBasePrinciple>, principle_interface<principle_t<TBasePrinciple, OperationTree>>
    {
        friend constexpr decltype(auto) data(unwarp_derived_from<principle_t> auto&& self)
        {
            return data(FWD(self) | base);
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
    struct tree_t : based_on<T>, standard_interface<tree_t<T, OperationTree>>
    {
        template<size_t I, unwarp_derived_from<tree_t> Self> 
        friend constexpr decltype(auto) subtree(Self&& self)
        {
            constexpr auto op_subtree = tag_tree_get<I>(OperationTree);
            if constexpr(I >= size<decltype(OperationTree)>)
            {
                return end();
            }
            else if constexpr(not std::same_as<decltype(op_subtree), const operation_t>)
            {
                return tree_t<decltype(FWD(self) | base | senluo::subtree<I>), op_subtree>
                {
                    FWD(self) | base | senluo::subtree<I>
                };
            }
            else if constexpr(op_subtree == operation_t::none)
            {
                return FWD(self) | base | senluo::subtree<I>;
            }
            else
            {
                return apply_invoke(FWD(self) | base | senluo::subtree<I>);
            }
        }

        template<auto UsageTree, unwarp_derived_from<tree_t> Self>
        friend constexpr auto principle(Self&& self)
        {
            constexpr auto fitted_usage_result = fit_operation_usage<OperationTree>(UsageTree);
            constexpr auto fittedd_usage = fitted_usage_result.usage_tree;
            constexpr bool need_plain = fitted_usage_result.need_plain;

            using base_principle_t = decltype(FWD(self) | base | senluo::principle<fittedd_usage>);
            
            if constexpr(detail::equal(base_principle_t::operation_tree(), operation_t::none))
            {
                if constexpr(not need_plain)
                {
                    return principle_t<base_principle_t, OperationTree>{ 
                        FWD(self) | base | senluo::principle<fittedd_usage> 
                    };
                }
                else
                {
                    using base_plain_principle_t = decltype(FWD(self) | base | plainize_principle<fittedd_usage>);
                    return principle_t<base_plain_principle_t, OperationTree>{ 
                        FWD(self) | base | plainize_principle<fittedd_usage>
                    };
                }
            }
            else
            {
                auto base_plain = FWD(self) | base | plainize_principle<fittedd_usage>;
                if constexpr(not need_plain)
                {
                    return principle_t<decltype(base_plain), OperationTree>{ std::move(base_plain) };
                }
                else
                {
                    return principle_t<decltype(base_plain), OperationTree>{ std::move(base_plain) }
                           | plainize_principle<fittedd_usage>;
                }
            }
        }
    };
}

namespace senluo 
{
    template<auto OperationTree>
    struct detail::operate_t : adaptor_closure<operate_t<OperationTree>>
    {
        template<typename T>
        constexpr decltype(auto) operator()(T&& t)const
        {
            if constexpr(not std::same_as<decltype(OperationTree), operation_t>)
            {
                return operate_ns::tree_t<senluo::unwrap_t<T>, OperationTree>{ unwrap_fwd(FWD(t)) };
            }
            else if constexpr(OperationTree == operation_t::none)
            {
                return decltype(wrap(FWD(t))){ unwrap_fwd(FWD(t)) };
            }
            else
            {
                return decltype(wrap(apply_invoke(FWD(t)))){ 
                    unwrap_fwd(apply_invoke(FWD(t))) 
                };
            }
        }
    };
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
                    | operate<detail::make_tree_of_same_value(operation_t::apply_invoke, shape<array<size_t, n>>)>;
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
                    | operate<detail::make_tree_of_same_value(operation_t::apply_invoke, shape<array<size_t, n>>)>;
            }
        };
    }

    inline constexpr detail::zip_transform_t zip_transform{};
}

#include "../macro_undef.hpp"
#endif