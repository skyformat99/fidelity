#pragma once

#define GET_OVERLOADED(_1, _2, _3, _4, _5, _6, MACRO, ...) MACRO
#define CAT0(GroupName) GroupName
#define CAT1(GroupName, a) GroupName##_##a
#define CAT2(GroupName, a, b) GroupName##_##a##_##b
#define CAT3(GroupName, a, b, c) GroupName##_##a##_##b##_##c
#define CAT4(GroupName, a, b, c, d) GroupName##_##a##_##b##_##c##_##d
#define CAT5(GroupName, a, b, c, d, e) GroupName##_##a##_##b##_##c##_##d##_##e
#define CAT(...) GET_OVERLOADED(__VA_ARGS__, CAT5, CAT4, CAT3, CAT2, CAT1, CAT0)(__VA_ARGS__)

/** Optimize TEST_F usage by adding context description */
#define DESCRIBE_F(TestBase, Context, ...) TEST_F(TestBase, CAT(Context, __VA_ARGS__))

/** Optimize TEST usage by adding context description */
#define DESCRIBE(TestBase, Context, ...) TEST(TestBase, CAT(Context, __VA_ARGS__))
