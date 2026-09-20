include(FetchContent)

if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(simcourse_tests
    extractorTests.cpp
    fibonacciTests.cpp
    isaTests/addTests.cpp
    isaTests/addiTests.cpp
    isaTests/beqTests.cpp
    isaTests/bextTests.cpp
    isaTests/liTests.cpp
    isaTests/xorTests.cpp
    isaTests/jTests.cpp
    isaTests/clsTests.cpp
    isaTests/usatTests.cpp
    isaTests/ldRegTests.cpp
    isaTests/ldTests.cpp
    isaTests/stTests.cpp
    isaTests/ldpTests.cpp
    isaTests/sbitTests.cpp
    isaTests/syscallTests.cpp
)

target_compile_features(simcourse_tests PRIVATE cxx_std_17)

configure_file(fibonacci.bin fibonacci.bin COPYONLY)
target_compile_definitions(simcourse_tests PRIVATE
    FIBONACCI_BINARY_PATH="${CMAKE_CURRENT_BINARY_DIR}/fibonacci.bin"
)

target_link_libraries(simcourse_tests PRIVATE
    isa
    simulator_core
    general_functions
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(simcourse_tests PROPERTIES TIMEOUT 10)
