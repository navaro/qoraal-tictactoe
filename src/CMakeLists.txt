# Collect all source files
set(FLASH_SRC
    platform.c
    tictactoe.c
    services/engine.c
    services/wserver_inst.c
    services/html/htmlpart.c
    services/html/wengine.c

)


# Add the library target
add_library(qoraal-tictactoe STATIC ${FLASH_SRC})

# Link necessary libraries
target_link_libraries(qoraal qoraal-http qoraal-engine)

