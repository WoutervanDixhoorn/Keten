file(GLOB KETEN_TEST_SYNC_SOURCES CONFIGURE_DEPENDS "*.cpp" "*.h")

add_executable(test-sync ${KETEN_TEST_SYNC_SOURCES})

target_link_libraries(test-sync PRIVATE keten)