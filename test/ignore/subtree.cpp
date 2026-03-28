#include "senluo/core/subtree.hpp"
#include "test_tool.hpp"

using namespace senluo;

TEST(subtree, subtree)
{
    auto t = tuple<int, tuple<float, double>>{ 1, { 2.0f, 3.0 } };

    MAGIC_CHECK((subtree<0>(t)), 1, int&);    
    MAGIC_CHECK((subtree<-2>(t)), 1, int&);
    MAGIC_CHECK((subtree<-4>(t)), 1, int&);
    MAGIC_CHECK((subtree<2>(t)), 1, int&);
    MAGIC_CHECK((subtree<4>(t)), 1, int&);
    MAGIC_CHECK((subtree<1>(t)), (tuple{ 2.0f, 3.0 }), tuple<float, double>&);
    MAGIC_CHECK((subtree<1, 0>(t)), 2.0f, float&);
    MAGIC_CHECK((subtree<1, 1>(t)), 3.0, double&);
    MAGIC_CHECK((subtree<array{1, 0}>(t)), 2.0f, float&);
    MAGIC_CHECK((subtree<array{1, 1}>(t)), 3.0, double&);
    MAGIC_CHECK((subtree<array{1}, 0>(t)), 2.0f, float&);
    MAGIC_CHECK((subtree<array{1}, 1>(t)), 3.0, double&);
    MAGIC_CHECK((subtree<1, array{0}>(t)), 2.0f, float&);
    MAGIC_CHECK((subtree<1, array{1}>(t)), 3.0, double&);

    MAGIC_CHECK((subtree<0>(std::move(t))), 1, int);
    MAGIC_CHECK((subtree<0>(std::move(std::as_const(t)))), 1, int);
}