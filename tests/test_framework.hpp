#ifndef CANTEEN_TEST_FRAMEWORK_HPP
#define CANTEEN_TEST_FRAMEWORK_HPP

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <stdexcept>

namespace canteen_test {

struct TestFailure {
    std::string testName;
    std::string file;
    int line;
    std::string message;
};

class TestRegistry {
private:
    struct TestCase {
        std::string name;
        std::function<void()> func;
    };

    std::vector<TestCase> tests;
    int passedCount{0};
    int failedCount{0};
    std::vector<TestFailure> failures;

    TestRegistry() = default;

public:
    static TestRegistry& instance() {
        static TestRegistry reg;
        return reg;
    }

    void registerTest(const std::string& name, std::function<void()> func) {
        tests.push_back({name, std::move(func)});
    }

    void recordFailure(const std::string& currentTest, const char* file, int line, const std::string& msg) {
        failures.push_back({currentTest, file, line, msg});
    }

    int runAll() {
        std::cout << "==============================================================================\n";
        std::cout << "               RUNNING CANTEEN DSA FOUNDATION TEST SUITE                      \n";
        std::cout << "==============================================================================\n";

        passedCount = 0;
        failedCount = 0;
        failures.clear();

        for (const auto& t : tests) {
            size_t failBefore = failures.size();
            try {
                t.func();
            } catch (const std::exception& ex) {
                recordFailure(t.name, "UNKNOWN", 0, std::string("Unhandled exception: ") + ex.what());
            } catch (...) {
                recordFailure(t.name, "UNKNOWN", 0, "Unhandled non-std exception.");
            }

            if (failures.size() == failBefore) {
                std::cout << "  [PASS] " << t.name << "\n";
                ++passedCount;
            } else {
                std::cout << "  [FAIL] " << t.name << "\n";
                ++failedCount;
            }
        }

        std::cout << "==============================================================================\n";
        std::cout << "TEST SUMMARY: " << passedCount << " PASSED, " << failedCount << " FAILED "
                  << "(Total: " << tests.size() << ")\n";

        if (!failures.empty()) {
            std::cout << "\n--- FAILURE DETAILS ---\n";
            for (const auto& f : failures) {
                std::cout << "In test '" << f.testName << "' (" << f.file << ":" << f.line << "):\n"
                          << "  " << f.message << "\n";
            }
            std::cout << "==============================================================================\n";
            return 1;
        }

        std::cout << "ALL TESTS PASSED SUCCESSFULLY!\n";
        std::cout << "==============================================================================\n";
        return 0;
    }
};

struct TestAutoRegister {
    TestAutoRegister(const std::string& name, std::function<void()> func) {
        TestRegistry::instance().registerTest(name, std::move(func));
    }
};

#define TEST_CASE(name) \
    static void test_func_##name(); \
    static canteen_test::TestAutoRegister reg_##name(#name, test_func_##name); \
    static void test_func_##name()

#define ASSERT_TRUE(cond) \
    do { \
        if (!(cond)) { \
            canteen_test::TestRegistry::instance().recordFailure(__func__, __FILE__, __LINE__, \
                "Assertion failed: (" #cond ") is false"); \
            return; \
        } \
    } while (0)

#define ASSERT_FALSE(cond) \
    do { \
        if (cond) { \
            canteen_test::TestRegistry::instance().recordFailure(__func__, __FILE__, __LINE__, \
                "Assertion failed: (" #cond ") is true, expected false"); \
            return; \
        } \
    } while (0)

#define ASSERT_EQ(a, b) \
    do { \
        if (!((a) == (b))) { \
            std::ostringstream ss; \
            ss << "Assertion failed: (" #a " == " #b ") -> (" << (a) << " != " << (b) << ")"; \
            canteen_test::TestRegistry::instance().recordFailure(__func__, __FILE__, __LINE__, ss.str()); \
            return; \
        } \
    } while (0)

#define ASSERT_NE(a, b) \
    do { \
        if ((a) == (b)) { \
            std::ostringstream ss; \
            ss << "Assertion failed: (" #a " != " #b ") -> values are equal: " << (a); \
            canteen_test::TestRegistry::instance().recordFailure(__func__, __FILE__, __LINE__, ss.str()); \
            return; \
        } \
    } while (0)

#define ASSERT_THROWS(expr, ExceptionType) \
    do { \
        bool caught = false; \
        try { \
            expr; \
        } catch (const ExceptionType&) { \
            caught = true; \
        } catch (...) { \
            canteen_test::TestRegistry::instance().recordFailure(__func__, __FILE__, __LINE__, \
                "Assertion failed: (" #expr ") threw unexpected exception type instead of " #ExceptionType); \
            return; \
        } \
        if (!caught) { \
            canteen_test::TestRegistry::instance().recordFailure(__func__, __FILE__, __LINE__, \
                "Assertion failed: (" #expr ") did not throw expected exception " #ExceptionType); \
            return; \
        } \
    } while (0)

#define ASSERT_NO_THROW(expr) \
    do { \
        try { \
            expr; \
        } catch (const std::exception& e) { \
            canteen_test::TestRegistry::instance().recordFailure(__func__, __FILE__, __LINE__, \
                std::string("Assertion failed: (" #expr ") threw exception: ") + e.what()); \
            return; \
        } catch (...) { \
            canteen_test::TestRegistry::instance().recordFailure(__func__, __FILE__, __LINE__, \
                "Assertion failed: (" #expr ") threw unknown non-std exception"); \
            return; \
        } \
    } while (0)

} // namespace canteen_test

#endif // CANTEEN_TEST_FRAMEWORK_HPP
