file(GLOB KETEN_TEST_NODE_SOURCES CONFIGURE_DEPENDS "*.cpp" "*.h")

add_executable(test-node ${KETEN_TEST_NODE_SOURCES})

target_link_libraries(test-node PRIVATE keten)