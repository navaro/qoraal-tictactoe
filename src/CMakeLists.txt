# Collect all source files
set(FLASH_SRC
    tictactoe.c
    services/telnetserver.c
    services/engine.c
    services/wserver_inst.c
    services/html/htmlpart.c
    services/html/wengine.c

)

# Add the library target
add_library(qoraal-tictactoe OBJECT ${FLASH_SRC})

# Set target include dirs
target_include_directories(qoraal-http PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../include)
