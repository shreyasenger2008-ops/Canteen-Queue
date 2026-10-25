#include "test_framework.hpp"

int main() {
    return canteen_test::TestRegistry::instance().runAll();
}
