# The program's build: CMake reads this and writes a build for make or Ninja.
cmake_minimum_required(VERSION 3.20)
project(stats LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Werror)

# The library has a CMakeLists.txt of its own, which defines the target textstats.
add_subdirectory(textstats)

add_executable(main main.cpp)
# Linking to textstats also gives main its include folder, which it made PUBLIC.
target_link_libraries(main PRIVATE textstats)
