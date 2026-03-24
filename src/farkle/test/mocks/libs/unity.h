#ifndef UNITY_H
#define UNITY_H

#include <iostream>
#include <string>

#define UNITY_BEGIN() do { std::cout << "Starting tests..." << std::endl; } while(0)
#define UNITY_END() do { std::cout << "All tests passed." << std::endl; } while(0)

#define RUN_TEST(func) do { setUp(); std::cout << "Running " << #func << "... "; func(); std::cout << "OK" << std::endl; tearDown(); } while(0)

#define TEST_ASSERT_EQUAL(expected, actual) do { if ((expected) != (actual)) { std::cerr << "Assertion failed: expected " << (expected) << ", but was " << (actual) << " at " << __FILE__ << ":" << __LINE__ << std::endl; exit(1); } } while(0)
#define TEST_ASSERT_TRUE(condition) do { if (!(condition)) { std::cerr << "Assertion failed: condition is false at " << __FILE__ << ":" << __LINE__ << std::endl; exit(1); } } while(0)
#define TEST_ASSERT_EQUAL_STRING(expected, actual) do { if (std::string(expected) != std::string(actual)) { std::cerr << "Assertion failed: expected '" << (expected) << "', but was '" << (actual) << "' at " << __FILE__ << ":" << __LINE__ << std::endl; exit(1); } } while(0)

#endif
