#ifndef UNITY_H
#define UNITY_H

#include <iostream>
#include <string>

// Make sure variable is defined globally here instead of externally so tests compile without unity.cpp
inline int unity_fail_count = 0;

#define UNITY_BEGIN() do { unity_fail_count = 0; std::cout << "Starting tests..." << std::endl; } while(0)
#define UNITY_END() (std::cout << "Tests finished. Failures: " << unity_fail_count << std::endl, unity_fail_count)

extern void setUp(void);
extern void tearDown(void);

#define RUN_TEST(func) do { setUp(); std::cout << "Running " << #func << "... "; func(); std::cout << "OK" << std::endl; tearDown(); } while(0)

#define TEST_ASSERT_EQUAL(expected, actual) do { if ((expected) != (actual)) { std::cerr << "Assertion failed: expected " << (expected) << ", but was " << (actual) << " at " << __FILE__ << ":" << __LINE__ << std::endl; unity_fail_count++; return; } } while(0)
#define TEST_ASSERT_TRUE(condition) do { if (!(condition)) { std::cerr << "Assertion failed: condition is false at " << __FILE__ << ":" << __LINE__ << std::endl; unity_fail_count++; return; } } while(0)
#define TEST_ASSERT_FALSE(condition) do { if ((condition)) { std::cerr << "Assertion failed: condition is true at " << __FILE__ << ":" << __LINE__ << std::endl; unity_fail_count++; return; } } while(0)
#define TEST_ASSERT_FALSE_MESSAGE(condition, message) do { if ((condition)) { std::cerr << "Assertion failed: " << message << " at " << __FILE__ << ":" << __LINE__ << std::endl; unity_fail_count++; return; } } while(0)
#define TEST_ASSERT_EQUAL_STRING(expected, actual) do { if (std::string(expected) != std::string(actual)) { std::cerr << "Assertion failed: expected '" << (expected) << "', but was '" << (actual) << "' at " << __FILE__ << ":" << __LINE__ << std::endl; unity_fail_count++; return; } } while(0)
#define TEST_ASSERT_EQUAL_CHAR(expected, actual) TEST_ASSERT_EQUAL((char)(expected), (char)(actual))
#define TEST_ASSERT_EQUAL_INT(expected, actual) TEST_ASSERT_EQUAL((int)(expected), (int)(actual))
#define TEST_ASSERT_LESS_THAN(expected, actual) do { if ((actual) >= (expected)) { std::cerr << "Assertion failed: " << (actual) << " not less than " << (expected) << " at " << __FILE__ << ":" << __LINE__ << std::endl; unity_fail_count++; return; } } while(0)
#define TEST_ASSERT_EQUAL_PTR(expected, actual) TEST_ASSERT_EQUAL((void*)(expected), (void*)(actual))
#define TEST_ASSERT_NOT_EQUAL(expected, actual) do { if ((expected) == (actual)) { std::cerr << "Assertion failed: " << (expected) << " equals " << (actual) << " at " << __FILE__ << ":" << __LINE__ << std::endl; unity_fail_count++; return; } } while(0)
#define TEST_ASSERT_GREATER_OR_EQUAL(expected, actual) do { if ((actual) < (expected)) { std::cerr << "Assertion failed: " << (actual) << " is less than " << (expected) << " at " << __FILE__ << ":" << __LINE__ << std::endl; unity_fail_count++; return; } } while(0)

#endif
