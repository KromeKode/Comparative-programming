# The program's build. CMake downloads cJSON, a JSON library in one C file, when it
# first sets up the build.
cmake_minimum_required(VERSION 3.24)
project(books LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)
# The commit of the v1.7.18 release. A tag can be moved to another commit; a commit's
# hash names that commit and no other.
FetchContent_Declare(cjson
    GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
    GIT_TAG acc76239bee01d8e9c858ae2cab296704e52d916
    # cJSON's own CMakeLists.txt builds its tests too; with this, CMake only downloads
    # it, and the library is built below, from its one file.
    SOURCE_SUBDIR none)
FetchContent_MakeAvailable(cjson)
add_library(cjson STATIC ${cjson_SOURCE_DIR}/cJSON.c)
target_include_directories(cjson PUBLIC ${cjson_SOURCE_DIR})

add_executable(main main.c)
target_compile_options(main PRIVATE -Wall -Wextra -Werror)
target_link_libraries(main PRIVATE cjson)
