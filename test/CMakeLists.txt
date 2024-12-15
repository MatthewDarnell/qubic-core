cmake_minimum_required(VERSION 3.16)
enable_testing()

project(
        QubicTests
        #VERSION 0.1
        LANGUAGES CXX C
)

include_directories(.)
include_directories(../)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
################################
# GTest
################################
# Enable ExternalProject CMake module
include(ExternalProject)
include(FetchContent)

ExternalProject_Add(cScaleBuild
        GIT_REPOSITORY https://github.com/MatthewDarnell/cScale.git
        GIT_TAG 58b2acb9db3af2b3e5108f9fdf111187ca00f39d
        CONFIGURE_COMMAND ""
        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        BUILD_IN_SOURCE 1
        BUILD_COMMAND make all
        INSTALL_DIR ${CMAKE_SOURCE_DIR}/cScale
        INSTALL_COMMAND PREFIX=<INSTALL_DIR> make install
)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(NetworkMessages ${CMAKE_CURRENT_SOURCE_DIR}/network_messages.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../src/randombytes.cpp)
add_dependencies(NetworkMessages cScaleBuild)

target_include_directories(NetworkMessages PRIVATE ${CMAKE_SOURCE_DIR}/cScale/include)
target_link_directories(NetworkMessages PRIVATE ${CMAKE_SOURCE_DIR}/cScale/lib)
add_library(libcScale.a STATIC IMPORTED GLOBAL)
target_link_libraries(
        NetworkMessages
        GTest::gtest_main
        cScale
)

include(GoogleTest)
enable_testing()

gtest_discover_tests(NetworkMessages)

file(GLOB qubic_test_files
    assets.cpp
        common_def.cpp
        contract_core.cpp
        contract_qearn.cpp
        contract_qvault.cpp
        contract_qx.cpp
        kangaroo_twelve.cpp
        m256.cpp
        math_lib.cpp
        network_messages.cpp
        platform.cpp
        qpi.cpp
        qpi_collection.cpp
        qpi_hash_map.cpp
        score.cpp
        score_cache.cpp
        spectrum.cpp
        stdlib_impl.cpp
        tick_storage.cpp
        tx_status_request.cpp
        vote_counter.cpp
)