file(GLOB KETEN_NODE_SOURCES CONFIGURE_DEPENDS "*.cpp" "*.h")

add_executable(example-node ${KETEN_NODE_SOURCES})

target_link_libraries(example-node PRIVATE keten)