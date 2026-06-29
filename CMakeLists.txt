cmake_minimum_required(VERSION 3.26)
project("keten")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE KETEN_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
add_library(keten STATIC ${KETEN_SOURCES} 
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/monocypher/monocypher.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/msock/src/msock.c"
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/msock/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/picoSHA2
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/monocypher
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/nlohmann
)

if(WIN32)
    target_link_libraries(keten PUBLIC ws2_32)
endif()

if(MSVC)
    target_compile_options(keten PRIVATE /W4)
else()
    target_compile_options(keten PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_subdirectory(examples)