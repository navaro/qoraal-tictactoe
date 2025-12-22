cmake_minimum_required(VERSION 3.16)
project(qoraal-tic-tac-toe LANGUAGES C)

# --------------------
# 1) Fetch the Qoraal repo
# --------------------
include(FetchContent)

FetchContent_Declare(
    qoraal
    GIT_REPOSITORY https://github.com/navaro/qoraal.git
    GIT_TAG        main  # or whatever branch/commit you need
    SOURCE_SUBDIR  .
)

FetchContent_Declare(
    qoraal_engine
    GIT_REPOSITORY https://github.com/navaro/qoraal-engine.git
    GIT_TAG        main  # or whatever branch/commit you need
    SOURCE_SUBDIR  .
)

FetchContent_Declare(
    qoraal_http
    GIT_REPOSITORY https://github.com/navaro/qoraal-http.git
    GIT_TAG        main  # or whatever branch/commit you need
    SOURCE_SUBDIR  .
)
FetchContent_MakeAvailable(qoraal qoraal_engine qoraal_http)

include_directories("${qoraal_SOURCE_DIR}/include")
include_directories("${qoraal_http_SOURCE_DIR}/include")
include_directories("${qoraal_engine_SOURCE_DIR}/include")

# Include directories
include_directories(include)

# Add subdirectories
add_subdirectory(src)

# Option to build tests
option(BUILD_TICTACTOE_TESTS "Build the test directory" OFF)

if(BUILD_TICTACTOE_TESTS)
    add_subdirectory(test/posix)
endif()





