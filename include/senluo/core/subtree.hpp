#ifndef SENLUO_SUBTREE_HPP
#define SENLUO_SUBTREE_HPP

#include "../tools/array.hpp"
#include "tree.hpp"

#include "../tools/macro_define.hpp"

namespace senluo::detail
{
    template<class T>
    concept indexical_array = requires(std::remove_cvref_t<T> t, size_t i)
    {
        t[i];
        requires std::integral<std::remove_cvref_t<decltype(t[i])>>;
    };

    constexpr size_t normalize_index(std::integral auto index, size_t size)noexcept
    {
        if(index >= 0)
        {
            return static_cast<size_t>(index % size);
        }
        else
        {
            return static_cast<size_t>(size - -index % size);
        }
    }

    template<class...TIndexes>
    constexpr auto to_indexes(const TIndexes&...indexes)noexcept
    {
        if constexpr(sizeof...(TIndexes) == 0uz)
        {
            return array<size_t, 0uz>{};
        }
        else if constexpr(sizeof...(indexes) > 1uz)
        {
            return detail::array_cat(detail::to_indexes(indexes)...);
        }
        else if constexpr(requires{ (..., indexes[0]); })
        {
            return (..., indexes);
        }
        else
        {
            return array{ indexes... };
        }
    }
}

namespace senluo
{
    template<class T>
    concept indexical = std::integral<T> || detail::indexical_array<T>;

    inline constexpr array<size_t, 0uz> indexes_of_whole{};

    namespace detail
    {
        template<auto indexes>
        struct subtree_fn;
    }
    
    inline namespace functors
    {
        template<indexical auto...I>
        inline constexpr auto subtree = detail::subtree_fn<detail::to_indexes(I...)>{};
    }

    template<auto Indexes>
    struct detail::subtree_fn : adaptor_closure<subtree_fn<Indexes>>
    {
        template<class T> requires (Indexes.size() == 0uz)
        constexpr decltype(auto) operator()(T&& t) const
        {
            return unwrap(FWD(t));
        }

        template<class T> requires (Indexes.size() == 1uz && size<T> > 0)
        constexpr decltype(auto) operator()(T&& t) const
        {
            return tree_get<detail::normalize_index(Indexes[0], size<T>)>(FWD(t));
        }

        template<class T> requires (Indexes.size() > 1uz && size<T> > 0)
        constexpr decltype(auto) operator()(T&& t) const
        {
            return tree_get<detail::normalize_index(Indexes[0], size<T>)>(FWD(t)) | subtree<detail::array_drop<1uz>(Indexes)>;
        }
    };

    template<class T, indexical auto...I>
    using subtree_t = decltype(subtree<I...>(std::declval<T>()));
}

#include "../tools/macro_undef.hpp"
#endif // SENLUO_SUBTREE_HPP