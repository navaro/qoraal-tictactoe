# Collect all test source files
set(TEST_SRCS
    main.c
    platform.c
    tictactoe.c
    services/engine.c
    services/wserver_inst.c
    services/html/htmlpart.c
    services/html/wengine.c
)

# Add the executable target
add_executable(tictactoe ${TEST_SRCS})

target_compile_definitions(tictactoe PRIVATE CFG_OS_POSIX)
target_compile_definitions(qoraal PRIVATE CFG_OS_POSIX)
target_compile_definitions(qoraal-http PRIVATE CFG_OS_POSIX)
target_compile_definitions(qoraal-engine PRIVATE CFG_OS_POSIX CFG_ENGINE_REGISTRY_DISABLE)
target_compile_options(tictactoe PRIVATE -O0 -g)
target_compile_options(qoraal PRIVATE -O0 -g)
target_compile_options(qoraal-http PRIVATE -O0 -g)
target_compile_options(qoraal-engine PRIVATE -O0 -g)

# Link the qoraal library and pthread
if(WIN32)
    target_link_libraries(tictactoe qoraal qoraal-http qoraal-engine pthread ws2_32)
else()
    target_link_libraries(tictactoe qoraal qoraal-http qoraal-engine pthread m)
endif()



# Define CFG_OS_MEM_DEBUG_ENABLE for qoraal




target_link_options(tictactoe PRIVATE
    "-Wl,--whole-archive"
    "$<TARGET_FILE:qoraal-engine>"
    "-Wl,--no-whole-archive"
)

# Generate a map file
set_target_properties(tictactoe PROPERTIES LINK_FLAGS "-Wl,-Map=output.map -T ${CMAKE_SOURCE_DIR}/src/posix.ld")