find_package(GTest 1.17 CONFIG QUIET)
if(NOT TARGET GTest::gtest_main)
    if(MSVC)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(googletest
        URL https://codeload.github.com/google/googletest/tar.gz/52eb8108c5bdec04579160ae17225d66034bd723
        URL_HASH SHA256=745c55415660044610f7fcd3af7a6420d5de16a7dbb9ebfe2e131275676232be
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(googletest)
endif()

add_executable(simcourse_tests
    cpuCacheTests.cpp
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

configure_file(fibonacci.elf fibonacci.elf COPYONLY)
target_compile_definitions(simcourse_tests PRIVATE
    FIBONACCI_ELF_PATH="${CMAKE_CURRENT_BINARY_DIR}/fibonacci.elf"
    SIMCOURSE_HAS_MUSTTAIL=$<BOOL:${SIMCOURSE_HAS_MUSTTAIL}>
)

target_link_libraries(simcourse_tests PRIVATE
    isa
    simulator_core
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(simcourse_tests PROPERTIES TIMEOUT 10)
