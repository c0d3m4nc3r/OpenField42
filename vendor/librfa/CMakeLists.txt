cmake_minimum_required(VERSION 3.10)

project(librfa VERSION 1.0.0 LANGUAGES C)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_library(librfa STATIC
    ${SRC_DIR}/rfa_compression.c
    ${SRC_DIR}/rfa_extract.c
    ${SRC_DIR}/rfa_internal.c
    ${SRC_DIR}/rfa_files.c
    ${SRC_DIR}/rfa.c
    ${SRC_DIR}/minilzo.c
)

target_include_directories(librfa PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(librfa PRIVATE ${STRICT_C_FLAGS})

option(BUILD_TESTS "Build librfa test executable" OFF)

if(BUILD_TESTS)
    add_executable(librfa_test ${SRC_DIR}/test.c)
    target_link_libraries(librfa_test PRIVATE librfa)
    target_compile_options(librfa_test PRIVATE ${STRICT_C_FLAGS})
endif()
