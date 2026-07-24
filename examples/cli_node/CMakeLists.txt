file(GLOB KETEN_NODE_SOURCES CONFIGURE_DEPENDS "*.cpp" "*.h")

add_executable(cli-node ${KETEN_NODE_SOURCES})

target_link_libraries(cli-node PRIVATE keten)