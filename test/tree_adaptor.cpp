#include <senluo/core/tree_adaptor.hpp>
#include "test_tool.hpp"

using namespace senluo;

TEST(tree_adaptor, refer_pass)
{
    int i = 0;

    MAGIC_CHECK((refer(i)), 0, int&);
    MAGIC_CHECK((refer(std::as_const(i))), 0, const int&);
    MAGIC_CHECK((refer(std::move(i))), wrapper<int&&>{0}, wrapper<int&&>);
    MAGIC_CHECK((refer(std::move(std::as_const(i)))), wrapper<const int&&>{0}, wrapper<const int&&>);

    MAGIC_CHECK((pass(i)), 0, int&);
    MAGIC_CHECK((pass(std::as_const(i))), 0, const int&);
    MAGIC_CHECK((pass(std::move(i))), 0, int);
    MAGIC_CHECK((pass(std::move(std::as_const(i)))), 0, int);

    MAGIC_CHECK((i | pass), 0, int&);
    MAGIC_CHECK((std::as_const(i) | pass), 0, const int&);
    MAGIC_CHECK((std::move(i) | pass), 0, int);
    MAGIC_CHECK((std::move(std::as_const(i)) | pass), 0, int);

    MAGIC_CHECK((refer(i) | pass), 0, int&);
    MAGIC_CHECK((refer(std::as_const(i)) | pass), 0, const int&);
    MAGIC_CHECK((refer(std::move(i)) | pass), 0, int&&);
    MAGIC_CHECK((refer(std::move(std::as_const(i))) | pass), 0, const int&&);

    MAGIC_CHECK((i | pass | pass), 0, int&);
    MAGIC_CHECK((std::as_const(i) | pass | pass), 0, const int&);
    MAGIC_CHECK((std::move(i) | pass | pass), 0, int);
    MAGIC_CHECK((std::move(std::as_const(i)) | pass | pass), 0, int);

    MAGIC_CHECK((i | (pass | pass)), 0, int&);
    MAGIC_CHECK((std::as_const(i) | (pass | pass)), 0, const int&);
    MAGIC_CHECK((std::move(i) | (pass | pass)), 0, int);
    MAGIC_CHECK((std::move(std::as_const(i)) | (pass | pass)), 0, int);

    MAGIC_CHECK((refer(i) | pass | pass), 0, int&);
    MAGIC_CHECK((refer(std::as_const(i)) | pass | pass), 0, const int&);
    MAGIC_CHECK((refer(std::move(i)) | pass | pass), 0, int);
    MAGIC_CHECK((refer(std::move(std::as_const(i))) | pass | pass), 0, int);

    MAGIC_CHECK((refer(i) | (pass | pass)), 0, int&);
    MAGIC_CHECK((refer(std::as_const(i)) | (pass | pass)), 0, const int&);
    MAGIC_CHECK((refer(std::move(i)) | (pass | pass)), 0, int&&);
    MAGIC_CHECK((refer(std::move(std::as_const(i))) | (pass | pass)), 0, const int&&);

    MAGIC_CHECK((i | refer | (pass | pass)), 0, int&);
    MAGIC_CHECK((std::as_const(i) | refer | (pass | pass)), 0, const int&);
    MAGIC_CHECK((std::move(i) | refer | (pass | pass)), 0, int&&);
    MAGIC_CHECK((std::move(std::as_const(i)) | refer | (pass | pass)), 0, const int&&);
}