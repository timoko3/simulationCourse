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
    isaTests/addTests.cpp
    isaTests/liTests.cpp
    isaTests/syscallTests.cpp
)

target_compile_features(simcourse_tests PRIVATE cxx_std_17)

target_link_libraries(simcourse_tests PRIVATE
    isa
    simulator_core
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(simcourse_tests)
