#ifndef SENLUO_VISIT_HPP
#define SENLUO_VISIT_HPP

#include "subview.hpp"

#include "../tools/macro_define.hpp"

namespace senluo::detail
{
    template<class T>
    inline constexpr size_t code_size = T::code_size();
    
    template<class...T>
    constexpr size_t code_size<tuple<T...>> = (sizeof...(T) + ... + code_size<T>);

    template<class...T>
    constexpr size_t* to_code_on(const tuple<T...>& tree, size_t* code)
    {
        const auto base = code;   
        code += sizeof...(T);

        [&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., (base[I] = code - base, code = to_code_on(get<I>(tree), code)));
        }(std::index_sequence_for<T...>{});

        return code;
    }

    // for indexes
    template<size_t N>
    constexpr size_t code_size<array<size_t, N>> = 1uz + 1uz + N;

    template<size_t N>
    constexpr size_t* to_code_on(const array<size_t, N>& tree, size_t* code)
    {
        *code++ = 0uz;
        *code++ = N;
        for(const size_t& i : tree)
        {
            *code++ = i;
        }
        return code;
    }

    template<const size_t* Code>
    consteval auto layout_from_code()
    {
        constexpr size_t n = Code[0];
        if constexpr(n == 0)
        {
            constexpr size_t m = Code[1];
            array<size_t, m> result;
            for(size_t i = 0; i < m; ++i)
            {
                result[i] = Code[2 + i];
            }
            return result;
        }
        else return []<size_t...I>(std::index_sequence<I...>)
        {
            return make_tuple(layout_from_code<Code + Code[I]>()...);
        }(std::make_index_sequence<n>{});
    }
    
    enum class visit_type
    {
        ref,
        read
    };

    template<size_t N>
    struct visit
    {
        visit_type type;
        array<size_t, N> indexes;
        
        static consteval size_t code_size()
        {
            return 1uz + 1uz + N;
        }
    };

    template<size_t N>
    constexpr size_t* to_code_on(const visit<N>& v, size_t* code)
    {
        *code++ = (size_t)v.type;
        *code++ = N;
        for(size_t i : v.indexes)
        {
            *code++ = i;
        }
        return code;
    }
    
    enum class visits_relation
    {
        sequence = 2,
        parallel = 3
    };

    template<class...TVisitTrees>
    struct visits
    {
        visits_relation relation;
        tuple<TVisitTrees...> children;
        
        static consteval size_t code_size()
        {
            return 1uz + (sizeof...(TVisitTrees) + ... + detail::code_size<TVisitTrees>);
        }
    };
    
    template<class...TVisitTrees>
    constexpr size_t* to_code_on(const visits<TVisitTrees...>& v, size_t* code)
    {
        *code++ = (size_t)v.relation;
        const auto base = code;
        
        code += sizeof...(TVisitTrees);

        [&]<size_t...I>(std::index_sequence<I...>)
        {
            (..., (base[I] = code - base, code = to_code_on(get<I>(v.children), code)));
        }(std::index_sequence_for<TVisitTrees...>{});

        return code;
    }

    template<class VisitTree>
    consteval auto to_code(const VisitTree& tree)
    {
        array<size_t, code_size<VisitTree>> result{};
        to_code_on(tree, result.data());
        return result;
    }

    template<const size_t* code>
    constexpr auto visit_tree_from_code()
    {
        constexpr size_t type = code[0];
        constexpr size_t n = code[1];
        if constexpr(type < 2)
        {
            visit<n> result{
                .type = (visit_type)type
            };

            for(size_t i = 0; i < n; ++i)
            {
                result.indexes[i] = code[2 + i];
            }
            return result;
        }
        else return[&]<size_t...I>(std::index_sequence<I...>)
        {
            return visits{
                (visits_relation)type,
                tuple{ visit_tree_from_code<code + 1 + code[1 + I]>()... }
            };

        }(std::make_index_sequence<n>{});
    }

    template<auto Tree>
    constexpr auto foo()
    {
        static constexpr auto code = to_code(Tree);
        return visit_tree_from_code<code.data()>();
    }

    constexpr auto tt = foo<
        visits{visits_relation::sequence, tuple{ 
            visit<2>{visit_type::read, {1uz, 2uz} },
            visit<2>{visit_type::read, {3uz, 4uz} }
        } }  
    >();
    constexpr auto ttt = get<1>(tt.children).indexes[1];

    template<size_t CodeLen, size_t ChildCount>
    constexpr auto default_query_subvisits_and_layout(const array<size_t, CodeLen>& code)
    {
        struct result
        {
            array<array<size_t, CodeLen>, ChildCount> subvisits;
            array<size_t, CodeLen> layout;
        } result{};

        
    }

    template<auto VisitTree, typename T>
    constexpr auto default_query(T&& t)
    {
        constexpr auto tree = detail::to_code(VisitTree);

    }
}

namespace senluo 
{
    namespace detail::query_ns
    {
        template<auto VisitTree>
        void query();

        template<auto VisitTree>
        struct query_fn
        {
            template<typename T>
            static constexpr decltype(auto) operator()(T&& t)
            {
                if constexpr(std::is_bounded_array_v<std::remove_cvref_t<T>>)
                {

                }
                else if constexpr(requires{ std::declval<T>().template query<VisitTree>(); })
                {
                    return FWD(t).template query<VisitTree>();
                }
                else if constexpr(requires{ query<VisitTree>(std::declval<T>()); })
                {
                    return query<VisitTree>(FWD(t));
                }
            }
        };    
    }
}

#include "../tools/macro_undef.hpp"
#endif