cmake_minimum_required(VERSION 3.24)

project(SimCourse
    VERSION 1.0.0
    LANGUAGES CXX
)

include(FetchContent)

find_package(CLI11 2.6.2 CONFIG QUIET)
if(NOT TARGET CLI11::CLI11)
    FetchContent_Declare(cli11
        URL https://codeload.github.com/CLIUtils/CLI11/tar.gz/refs/tags/v2.6.2
        URL_HASH SHA256=c6ea6b2e5608b3ea8617999bd5f47420c71b2ebdb8dc4767c1034d1da5785711
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(cli11)
endif()

find_package(elfio 3.12 CONFIG QUIET)
if(NOT TARGET elfio::elfio)
    FetchContent_Declare(elfio
        URL https://codeload.github.com/serge1/ELFIO/tar.gz/8ae6cec5d60495822ecd57d736f66149da9b1830
        URL_HASH SHA256=71c1a72e3fac6c9db3d4126c8f9b84bbda902e8dfb563f6dfcbed454636fa1c5
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(elfio)
endif()

find_package(CacheResearch 0.1 CONFIG QUIET)
if(NOT TARGET CacheResearch::cache)
    # Consume only the library, without the upstream tools or test suite.
    set(CACHE_RESEARCH_BUILD_APP OFF)
    set(CACHE_RESEARCH_BUILD_BENCHMARKS OFF)
    set(CACHE_RESEARCH_BUILD_TESTS OFF)
    set(CACHE_RESEARCH_INSTALL OFF)
    FetchContent_Declare(cache_research
        URL https://codeload.github.com/timoko3/cacheResearch/tar.gz/53b2af70a56335489c8d00565a5228f0bbc06f28
        URL_HASH SHA256=e77e974b1fd235feff88a0832e035dca8a22af596b9d17e4308da4938ab0e592
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(cache_research)
endif()

set(CMAKE_CXX_EXTENSIONS OFF)

# Instruction representation and decoding helpers.
add_library(isa STATIC
    isa/extractors.cpp
)

target_compile_features(isa PUBLIC cxx_std_17)

target_include_directories(isa PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Simulator implementation, reusable by applications and tests.
add_library(simulator_core STATIC
    toyFunctionalSim/cpu.cpp
    toyFunctionalSim/cpuRun.cpp
    toyFunctionalSim/memory.cpp
    toyFunctionalSim/functionalSimulator.cpp

    toyFunctionalSim/programLoader/elfLoader.cpp

    toyFunctionalSim/simplePipeline/fetchStage.cpp
    toyFunctionalSim/simplePipeline/decodeStage.cpp
    toyFunctionalSim/simplePipeline/execStage.cpp
    toyFunctionalSim/simplePipeline/syscallHandlers.cpp
)

target_compile_features(simulator_core PUBLIC cxx_std_17)

include(CheckCXXSourceCompiles)
check_cxx_source_compiles([=[
    #ifndef __has_cpp_attribute
    #error Attribute detection is unavailable
    #endif
    #if !__has_cpp_attribute(clang::musttail)
    #error clang::musttail is unavailable
    #endif
    void next(void*) {}
    void step(void* context) {
        [[clang::musttail]] return next(context);
    }
    int main() { step(nullptr); }
]=] SIMCOURSE_HAS_MUSTTAIL)

if(SIMCOURSE_HAS_MUSTTAIL)
    target_sources(simulator_core PRIVATE toyFunctionalSim/threadedExecutor.cpp)
endif()
target_compile_definitions(simulator_core PRIVATE
    SIMCOURSE_HAS_MUSTTAIL=$<BOOL:${SIMCOURSE_HAS_MUSTTAIL}>
)

target_include_directories(simulator_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/toyFunctionalSim"
)

target_link_libraries(simulator_core
    PUBLIC isa CacheResearch::cache
    PRIVATE elfio::elfio
)

# Application entry point.
add_executable(simcourse main.cpp)

target_compile_features(simcourse PRIVATE cxx_std_17)

target_link_libraries(simcourse PRIVATE simulator_core CLI11::CLI11)

include(GNUInstallDirs)
install(TARGETS simcourse RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

# Keep warning policy local to the project's targets.
foreach(target IN ITEMS
    isa
    simulator_core
    simcourse
)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()

include(CTest)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
