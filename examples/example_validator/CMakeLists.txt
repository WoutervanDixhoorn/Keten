file(GLOB KETEN_VALIDATOR_SOURCES CONFIGURE_DEPENDS "*.cpp" "*.h")

add_executable(example-validator ${KETEN_VALIDATOR_SOURCES})

target_link_libraries(example-validator PRIVATE keten)