# The program's build. CMake downloads nlohmann/json, a JSON library in headers only,
# when it first sets up the build.
cmake_minimum_required(VERSION 3.24)
project(jsondiff LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
# A fixed release, checked against its SHA-256, so that a changed download fails.
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    URL_HASH SHA256=42f6e95cad6ec532fd372391373363b62a14af6d771056dbfc86160e6dfff7aa)
FetchContent_MakeAvailable(json)

add_executable(main main.cpp)
target_compile_options(main PRIVATE -Wall -Wextra -Werror)
# Linking to it gives main its include folder.
target_link_libraries(main PRIVATE nlohmann_json::nlohmann_json)
