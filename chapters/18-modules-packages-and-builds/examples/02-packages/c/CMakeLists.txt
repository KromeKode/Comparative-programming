# The program's build: CMake reads this and writes a build for make or Ninja.
cmake_minimum_required(VERSION 3.20)
project(stats LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Werror)

# The library has a CMakeLists.txt of its own, which defines the target textstats.
add_subdirectory(textstats)

add_executable(main main.c)
# Linking to textstats also gives main its include folder, which it made PUBLIC.
target_link_libraries(main PRIVATE textstats)
