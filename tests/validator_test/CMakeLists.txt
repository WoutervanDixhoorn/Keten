file(GLOB KETEN_TEST_NODE_SOURCES CONFIGURE_DEPENDS "*.cpp" "*.h")

add_executable(validator-test ${KETEN_TEST_NODE_SOURCES})

target_link_libraries(validator-test PRIVATE keten)