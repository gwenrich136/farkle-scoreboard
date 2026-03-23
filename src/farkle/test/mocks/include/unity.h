#ifndef UNITY_H
#define UNITY_H

#include <iostream>

#define UNITY_BEGIN() (void)0
#define UNITY_END() 0

void setUp(void);
void tearDown(void);
#define RUN_TEST(func) \
    std::cout << "Running " << #func << "..." << std::endl; \
    setUp(); \
    func(); \
    tearDown(); \
    std::cout << "Passed." << std::endl

#define TEST_ASSERT(cond) if (!(cond)) { std::cerr << "Assertion failed: " << #cond << " at " << __FILE__ << ":" << __LINE__ << std::endl; exit(1); }
#define TEST_ASSERT_TRUE(cond) TEST_ASSERT(cond)
#define TEST_ASSERT_FALSE(cond) TEST_ASSERT(!(cond))
#define TEST_ASSERT_EQUAL(expected, actual) if ((expected) != (actual)) { std::cerr << "Assertion failed: Expected " << (expected) << " but got " << (actual) << " at " << __FILE__ << ":" << __LINE__ << std::endl; exit(1); }
#define TEST_ASSERT_NOT_EQUAL(expected, actual) if ((expected) == (actual)) { std::cerr << "Assertion failed: Expected not " << (expected) << " but got " << (actual) << " at " << __FILE__ << ":" << __LINE__ << std::endl; exit(1); }
#define TEST_ASSERT_EQUAL_STRING(expected, actual) if (std::string(expected) != std::string(actual)) { std::cerr << "Assertion failed: Expected \"" << (expected) << "\" but got \"" << (actual) << "\" at " << __FILE__ << ":" << __LINE__ << std::endl; exit(1); }
#define TEST_ASSERT_EQUAL_INT(expected, actual) TEST_ASSERT_EQUAL(expected, actual)

#endif
