cmake_minimum_required(VERSION 3.26)
project("keten")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/msock
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/picoSHA2
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/monocypher
)

file(GLOB_RECURSE KETEN_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
add_library(keten STATIC ${KETEN_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/vendor/monocypher/monocypher.c")

file(GLOB_RECURSE KETEN_EXAMPLE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/examples/example_validator/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/examples/example_validator/*.h")
add_executable(example-validator ${KETEN_EXAMPLE_SOURCES})

target_link_libraries(example-validator PRIVATE keten)

if(MSVC)
    target_compile_options(keten PRIVATE /W4)
else()
    target_compile_options(keten PRIVATE -Wall -Wextra -Wpedantic)
endif()