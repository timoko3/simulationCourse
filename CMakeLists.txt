cmake_minimum_required(VERSION 3.24)

project(SimCourse
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_EXTENSIONS OFF)

# Common utilities (sources from the generalFunctions submodule).
add_library(general_functions STATIC
    generalFunctions/file.cpp
    generalFunctions/logger.cpp
)

add_library(cache INTERFACE
    cacheResearch/cache.h)


target_compile_features(general_functions PUBLIC cxx_std_17)

target_include_directories(general_functions PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

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
    toyFunctionalSim/memory.cpp
    toyFunctionalSim/functionalSimulator.cpp

    toyFunctionalSim/simplePipeline/fetchStage.cpp
    toyFunctionalSim/simplePipeline/decodeStage.cpp
    toyFunctionalSim/simplePipeline/execStage.cpp
)

target_compile_features(simulator_core PUBLIC cxx_std_17)

target_include_directories(simulator_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/toyFunctionalSim"
)

target_link_libraries(simulator_core PUBLIC isa)

# Application entry point.
add_executable(simcourse main.cpp)

target_compile_features(simcourse PRIVATE cxx_std_17)

target_link_libraries(simcourse PRIVATE
    simulator_core
    general_functions
)

# Keep warning policy local to the project's targets.
foreach(target IN ITEMS
    general_functions
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