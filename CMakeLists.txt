cmake_minimum_required(VERSION 3.15)
project(CanteenQueueOptimizationSystem VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlite
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/httplib
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann
)

# Core Library Sources
set(CORE_SOURCES
    src/models/FoodItem.cpp
    src/models/Order.cpp
    src/models/Counter.cpp
    src/core/MenuManager.cpp
    src/core/CounterManager.cpp
    src/core/OrderManager.cpp
    src/core/CanteenSystem.cpp
    src/ml/RandomForestPredictor.cpp
    src/db/DatabaseManager.cpp
    src/api/HttpServer.cpp
    third_party/sqlite/sqlite3.c
)

# Core Library
add_library(canteen_core STATIC ${CORE_SOURCES})
target_include_directories(canteen_core PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlite
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/httplib
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann
)

if(WIN32)
    target_link_libraries(canteen_core PUBLIC ws2_32 crypt32)
endif()

# Main Application Executable
add_executable(canteen_app src/main.cpp)
target_link_libraries(canteen_app PRIVATE canteen_core)

# Tests
enable_testing()

set(TEST_SOURCES
    tests/test_main.cpp
    tests/test_array.cpp
    tests/test_linked_list.cpp
    tests/test_queue.cpp
    tests/test_stack.cpp
    tests/test_searching.cpp
    tests/test_sorting.cpp
    tests/test_integration.cpp
    tests/test_ml.cpp
    tests/test_database.cpp
    tests/test_api.cpp
)

add_executable(canteen_tests ${TEST_SOURCES})
target_include_directories(canteen_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(canteen_tests PRIVATE canteen_core)

add_test(NAME CanteenAllTests COMMAND canteen_tests)
