cmake_minimum_required(
    VERSION 4.3.0
)
project(
    "Yaregex"
    VERSION 1.0
    LANGUAGES C
)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE debug)
endif()

set(
    LIB_NAME
    "YarLib"
)

include(FetchContent)

set(CUNIT_VERSION 3.5.5)
FetchContent_Declare(
    cunit
    URL https://gitlab.com/cunity/cunit/-/archive/${CUNIT_VERSION}/cunit-${CUNIT_VERSION}.tar.gz
    FIND_PACKAGE_ARGS ${CUNIT_VERSION} EXACT
)
FetchContent_MakeAvailable(cunit)


set(
    LIB_SRC
    src/yar_scanner.c
    src/yar_parser.c
    src/yar_token_handler.c
    src/vector.c
)

add_library(
    ${LIB_NAME} 
    STATIC 
    ${LIB_SRC}
)

target_compile_options(
    ${LIB_NAME}
    PUBLIC 
    "-DYAR_DEBUG"
)

target_include_directories(
    ${LIB_NAME} 
    PUBLIC 
    include 
)

set(
    MAIN 
    "tests"
)

add_executable(
    ${MAIN} 
    tests/yar_parser_tests.c
)

target_compile_options(
    ${MAIN}
    PRIVATE
    "-lcunit"
)


target_link_libraries(
    ${MAIN}
    PRIVATE 
    ${LIB_NAME}
)

target_link_libraries(
    ${MAIN}
    PUBLIC     
    cunit 
)
