#include <senluo/core.hpp>

int main()
{
    auto tpl = senluo::tuple{1, 2, 3};
    tpl = +senluo::zip_transform(std::plus<>{}, tpl, tpl);

    return 0;
}