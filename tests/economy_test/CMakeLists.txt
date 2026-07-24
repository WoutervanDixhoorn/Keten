file(GLOB KETEN_TEST_ECON_SOURCES CONFIGURE_DEPENDS "*.cpp" "*.h")

add_executable(test-econ ${KETEN_TEST_ECON_SOURCES})

target_link_libraries(test-econ PRIVATE keten)