#ifndef UNITY_H
#define UNITY_H

#include <iostream>
#include <string>

void setUp(void);
void tearDown(void);

extern int unity_fail_count;

#define UNITY_BEGIN() do { std::cout << "Starting tests..." << std::endl; unity_fail_count = 0; } while(0)
#define UNITY_END() (unity_fail_count)

#define RUN_TEST(func) do { setUp(); std::cout << "Running " << #func << "... "; func(); std::cout << "OK" << std::endl; tearDown(); } while(0)

#define TEST_ASSERT_EQUAL(expected, actual) do { if ((expected) != (actual)) { std::cerr << "Assertion failed: expected " << (expected) << ", but was " << (actual) << " at " << __FILE__ << ":" << __LINE__ << std::endl; unity_fail_count++; return; } } while(0)
#define TEST_ASSERT_TRUE(condition) do { if (!(condition)) { std::cerr << "Assertion failed: condition is false at " << __FILE__ << ":" << __LINE__ << std::endl; unity_fail_count++; return; } } while(0)
#define TEST_ASSERT_FALSE(condition) TEST_ASSERT_TRUE(!(condition))
#define TEST_ASSERT_NOT_EQUAL(expected, actual) TEST_ASSERT_TRUE((expected) != (actual))
#define TEST_ASSERT_FALSE_MESSAGE(condition, message) do { if (condition) { std::cerr << "Assertion failed: " << message << " at " << __FILE__ << ":" << __LINE__ << std::endl; unity_fail_count++; return; } } while(0)
#define TEST_ASSERT_EQUAL_STRING(expected, actual) do { if (std::string(expected) != std::string(actual)) { std::cerr << "Assertion failed: expected '" << (expected) << "', but was '" << (actual) << "' at " << __FILE__ << ":" << __LINE__ << std::endl; unity_fail_count++; return; } } while(0)
#define TEST_ASSERT_EQUAL_CHAR(expected, actual) TEST_ASSERT_EQUAL((char)(expected), (char)(actual))
#define TEST_ASSERT_EQUAL_INT(expected, actual) TEST_ASSERT_EQUAL((int)(expected), (int)(actual))
#define TEST_ASSERT_EQUAL_PTR(expected, actual) TEST_ASSERT_EQUAL((void*)(expected), (void*)(actual))
#define TEST_ASSERT_LESS_THAN(threshold, actual) TEST_ASSERT_TRUE((actual) < (threshold))
#define TEST_ASSERT_GREATER_OR_EQUAL(threshold, actual) TEST_ASSERT_TRUE((actual) >= (threshold))

#endif
