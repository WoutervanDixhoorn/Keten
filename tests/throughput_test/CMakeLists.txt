file(GLOB KETEN_TEST_ECON_SOURCES CONFIGURE_DEPENDS "*.cpp" "*.h")

add_executable(throughput-test ${KETEN_TEST_ECON_SOURCES})

target_link_libraries(throughput-test PRIVATE keten)