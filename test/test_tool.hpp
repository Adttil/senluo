#ifndef SENLUO_TEST_TOOL_H
#define SENLUO_TEST_TOOL_H

#include <gtest/gtest.h>
#include <magic/visualize.h>

namespace senluo::test
{
    constexpr ::magic::VisualizeOption magic_option
    {
        .utf_support = false 
    };
}

#define MAGIC_TCHECK(type1, ...) \
GTEST_EXPECT_TRUE((std::same_as<type1, __VA_ARGS__>)) \
    << "Type [" << #type1 << "] is:\n" << magic::visualize<type1>(::senluo::test::magic_option) << '\n'\
    << "Type [" << #__VA_ARGS__ << "] is:\n" << magic::visualize<__VA_ARGS__>(::senluo::test::magic_option) << '\n'

#define MAGIC_CHECK(exp1_, ...) \
[]<typename T1, typename T2>(const char* exp1_str, const char* exp2_str, const auto& exp1, const auto& exp2)\
{\
    constexpr bool equal_valid = requires{ requires requires{ exp1 == exp2; }; };\
    GTEST_EXPECT_TRUE(equal_valid)\
    	<< '"' << exp1_str << " == " << exp2_str << "\" is invalid.\n"\
    	<< "Type of [" << exp1_str << "] is:\n" << ::magic::visualize<T1>(senluo::test::magic_option) << '\n'\
        << "Type of [" << exp2_str << "] is:\n" << ::magic::visualize<T2>(senluo::test::magic_option) << '\n';\
    if constexpr(equal_valid)\
    {\
        GTEST_ASSERT_(::testing::internal::EqHelper::Compare(exp1_str, exp2_str, exp1, exp2), GTEST_NONFATAL_FAILURE_);\
    }\
}.template operator()<decltype(exp1_), decltype(__VA_ARGS__)>(#exp1_, #__VA_ARGS__, exp1_, __VA_ARGS__);

#endif