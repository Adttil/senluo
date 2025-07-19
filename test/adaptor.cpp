#include <senluo/tools/adaptor.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(adaptor, refer_pass)
{
    int i = 0;

    MAGIC_CHECK((pass(i)), 0, int&);
    MAGIC_CHECK((pass(std::as_const(i))), 0, const int&);
    MAGIC_CHECK((pass(std::move(i))), 0, int);
    MAGIC_CHECK((pass(std::move(std::as_const(i)))), 0, int);

    MAGIC_CHECK((i | pass), 0, int&);
    MAGIC_CHECK((std::as_const(i) | pass), 0, const int&);
    MAGIC_CHECK((std::move(i) | pass), 0, int);
    MAGIC_CHECK((std::move(std::as_const(i)) | pass), 0, int);
}