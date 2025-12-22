cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(qoraal_test)

# target_sources(app PRIVATE src/main.c)
set(TEST_SRCS
    ../main.c
    src/platform.c
)
target_sources(app PRIVATE ${TEST_SRCS})

set(QORAAL_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)

add_subdirectory(
    ${QORAAL_ROOT}
    ${CMAKE_CURRENT_BINARY_DIR}/qoraal
)

# Suppose the library defined in the repo is called "qoraal"
target_link_libraries(qoraal      PUBLIC zephyr_interface)
target_link_libraries(qoraal-http PUBLIC zephyr_interface)
target_link_libraries(qoraal-engine PUBLIC zephyr_interface)
target_link_libraries(qoraal-tictactoe PUBLIC zephyr_interface)

target_compile_definitions(qoraal PRIVATE CFG_OS_ZEPHYR=1)
target_compile_definitions(qoraal-http PUBLIC CFG_OS_ZEPHYR=1 
                                CFG_HTTPSERVER_TLS_DISABLE=1  
                                CFG_HTTPCLIENT_TLS_DISABLE=1)
target_compile_definitions(qoraal-engine PUBLIC CFG_OS_ZEPHYR=1)
target_compile_definitions(qoraal-tictactoe PUBLIC CFG_OS_ZEPHYR=1
                                CFG_HTTPSERVER_TLS_DISABLE=1)   

target_compile_options(app PRIVATE -O0 -g)
target_compile_options(qoraal PRIVATE -O0 -g)
target_compile_options(qoraal-http PRIVATE -O0 -g)
target_compile_options(qoraal-engine PRIVATE -O0 -g)
target_compile_options(qoraal-tictactoe PRIVATE -O0 -g)

# link tictactoe.e as binary blob:
set(BLOB_SRC    ${QORAAL_ROOT}/tictactoe.e)
set(BLOB_SIMPLE ${CMAKE_CURRENT_BINARY_DIR}/tictactoe_e)   # no ext, no path
set(BLOB_OBJ    ${CMAKE_CURRENT_BINARY_DIR}/tictactoe_e.o)

# Step 0: copy to a simple name in the build dir
add_custom_command(
  OUTPUT ${BLOB_SIMPLE}
  COMMAND ${CMAKE_COMMAND} -E copy ${BLOB_SRC} ${BLOB_SIMPLE}
  DEPENDS ${BLOB_SRC}
  COMMENT "Copy tictactoe.e -> ${BLOB_SIMPLE}"
)

# Step 1: run objcopy FROM the build dir so symbols don't include a path
add_custom_command(
  OUTPUT ${BLOB_OBJ}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND ${CMAKE_OBJCOPY}
          -I binary -O elf32-littlearm -B arm
          --rename-section .data=.rodata.tictactoe,alloc,load,readonly,data,contents
          tictactoe_e tictactoe_e.o
  DEPENDS ${BLOB_SIMPLE}
  COMMENT "Embedding tictactoe_e -> tictactoe_e.o with short symbols"
)

target_sources(app PRIVATE ${BLOB_OBJ})

zephyr_linker_sources(SECTIONS src/zephyr.ld)

target_link_libraries(app PRIVATE   qoraal 
                                    qoraal-http 
                                    qoraal-engine 
                                    qoraal-tictactoe)
