# Collect all test source files
set(TEST_SRCS
    ../main.c
    platform.c
)

# Add the executable target
add_executable(tictactoe ${TEST_SRCS})

target_compile_definitions(qoraal PRIVATE CFG_OS_POSIX)
target_compile_definitions(qoraal-http PRIVATE CFG_OS_POSIX
                                CFG_HTTPSERVER_TLS_DISABLE=1
                                CFG_HTTPCLIENT_TLS_DISABLE=1)
target_compile_definitions(qoraal-tictactoe PRIVATE CFG_OS_POSIX
                                CFG_HTTPSERVER_TLS_DISABLE=1)
target_compile_definitions(tictactoe PRIVATE CFG_OS_POSIX)

target_compile_options(tictactoe PRIVATE -O0 -g)
target_compile_options(qoraal PRIVATE -O0 -g)
target_compile_options(qoraal-http PRIVATE -O0 -g)
target_compile_options(qoraal-engine PRIVATE -O0 -g)
target_compile_options(qoraal-tictactoe PRIVATE -O0 -g)

# Link the qoraal library and pthread
if(WIN32)
    target_link_libraries(tictactoe qoraal qoraal-http qoraal-engine qoraal-tictactoe pthread ws2_32)
else()
    target_link_libraries(tictactoe qoraal qoraal-http qoraal-engine qoraal-tictactoe pthread m)
endif()


# Generate a map file
set_target_properties(tictactoe PROPERTIES LINK_FLAGS "-Wl,-Map=output.map -T ${CMAKE_SOURCE_DIR}/test/posix/posix.ld")