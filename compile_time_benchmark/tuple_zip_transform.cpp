#include <senluo/core.hpp>

int main()
{
    auto tpl = senluo::tuple{1, 2, 3};
    using tuple_t = decltype(tpl);
    tpl = senluo::zip_transform(std::plus<>{}, tpl, tpl) | senluo::make<tuple_t>;


    return 0;
}