cmake_minimum_required(VERSION 3.24)

project(SimCourse
    VERSION 1.0.0
    LANGUAGES C CXX
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
        GIT_REPOSITORY https://github.com/timoko3/cacheResearch.git
        GIT_TAG origin/master
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(cache_research)
endif()

set(CMAKE_CXX_EXTENSIONS OFF)
option(SIMCOURSE_BUILD_BENCHMARKS "Build dispatch comparison benchmark (requires musttail)" OFF)

option(SIMCOURSE_ENABLE_LLVM "Link the LLVM backend dependencies" ON)
option(SIMCOURSE_FETCH_LLVM "Build LLVM from source when no installed package is found" OFF)
set(SIMCOURSE_LLVM_TAG "llvmorg-23.1.4" CACHE STRING "LLVM release tag used for the source fallback")

if(SIMCOURSE_ENABLE_LLVM)
    if(SIMCOURSE_LLVM_TAG MATCHES "^llvmorg-([0-9]+)\\.")
        if(CMAKE_MATCH_1 LESS 23)
            set(SIMCOURSE_LLVM_TAG "llvmorg-23.1.4" CACHE STRING "LLVM release tag used for the source fallback" FORCE)
        endif()
    endif()

    find_package(LLVM 23.1 CONFIG QUIET)
    if(NOT LLVM_FOUND)
        if(NOT SIMCOURSE_FETCH_LLVM)
            message(FATAL_ERROR
                "LLVM 23 development package was not found. Install llvm-23-dev from "
                "https://apt.llvm.org/ or enter the project environment with nix develop. "
                "Set LLVM_DIR to the directory reported by llvm-config-23 --cmakedir "
                "(llvm-config --cmakedir inside nix develop). "
                "To build LLVM from source explicitly, set -DSIMCOURSE_FETCH_LLVM=ON.")
        endif()

        message(STATUS "LLVM 23 not found; building ${SIMCOURSE_LLVM_TAG} from source")
        set(LLVM_TARGETS_TO_BUILD "Native" CACHE STRING "LLVM targets to build")
        set(LLVM_ENABLE_PROJECTS "" CACHE STRING "Additional LLVM projects")
        set(LLVM_ENABLE_RUNTIMES "" CACHE STRING "Additional LLVM runtimes")
        set(LLVM_INCLUDE_TESTS OFF CACHE BOOL "Build LLVM tests")
        set(LLVM_INCLUDE_EXAMPLES OFF CACHE BOOL "Build LLVM examples")
        set(LLVM_INCLUDE_BENCHMARKS OFF CACHE BOOL "Build LLVM benchmarks")
        set(LLVM_INCLUDE_DOCS OFF CACHE BOOL "Build LLVM documentation")
        set(LLVM_INCLUDE_TOOLS OFF CACHE BOOL "Build LLVM tools")
        FetchContent_Declare(llvm_project
            GIT_REPOSITORY https://github.com/llvm/llvm-project.git
            GIT_TAG ${SIMCOURSE_LLVM_TAG}
            GIT_SHALLOW TRUE
            GIT_SUBMODULES ""
            SOURCE_SUBDIR llvm
        )
        FetchContent_MakeAvailable(llvm_project)

        # LLVM's source build keeps these variables in its directory scope.
        foreach(variable LLVM_NATIVE_ARCH LLVM_TARGETS_TO_BUILD)
            get_directory_property(${variable}
                DIRECTORY "${llvm_project_SOURCE_DIR}/llvm" DEFINITION ${variable})
        endforeach()
        set(LLVM_INCLUDE_DIRS
            "${llvm_project_SOURCE_DIR}/llvm/include"
            "${llvm_project_BINARY_DIR}/include"
        )
        get_directory_property(SIMCOURSE_LLVM_DEFINITIONS
            DIRECTORY "${llvm_project_SOURCE_DIR}/llvm" COMPILE_DEFINITIONS)
        include("${llvm_project_SOURCE_DIR}/llvm/cmake/modules/LLVM-Config.cmake")
    else()
        message(STATUS "Using installed LLVM ${LLVM_PACKAGE_VERSION}")
        separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND "${LLVM_DEFINITIONS}")
        string(REGEX REPLACE "(^|;)-D" "\\1" SIMCOURSE_LLVM_DEFINITIONS
            "${LLVM_DEFINITIONS_LIST}")
    endif()

    llvm_map_components_to_libnames(SIMCOURSE_LLVM_LIBS
        core support orcjit native passes
    )
endif()

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
    toyFunctionalSim/dbtExecutor.cpp
    toyFunctionalSim/memory.cpp
    toyFunctionalSim/functionalSimulator.cpp

    toyFunctionalSim/programLoader/elfLoader.cpp

    toyFunctionalSim/simplePipeline/fetchStage.cpp
    toyFunctionalSim/simplePipeline/decodeStage.cpp
    toyFunctionalSim/simplePipeline/execStage.cpp
    toyFunctionalSim/simplePipeline/syscallHandlers.cpp
)

target_compile_features(simulator_core PUBLIC cxx_std_17)

if(SIMCOURSE_ENABLE_LLVM)
    target_sources(simulator_core PRIVATE toyFunctionalSim/dbt/llvmBackend/llvmBackend.cpp)
    target_include_directories(simulator_core SYSTEM PUBLIC ${LLVM_INCLUDE_DIRS})
    target_compile_definitions(simulator_core PRIVATE ${SIMCOURSE_LLVM_DEFINITIONS})
    target_link_libraries(simulator_core PRIVATE ${SIMCOURSE_LLVM_LIBS})

endif()

target_compile_definitions(simulator_core PUBLIC
    SIMCOURSE_HAS_LLVM=$<BOOL:${SIMCOURSE_ENABLE_LLVM}>
)

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

if(SIMCOURSE_BUILD_BENCHMARKS AND NOT SIMCOURSE_HAS_MUSTTAIL)
    message(FATAL_ERROR "Dispatch benchmarks require clang::musttail. Configure a fresh build directory with -DCMAKE_CXX_COMPILER=clang++")
endif()

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

if(SIMCOURSE_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()
