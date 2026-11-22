# A static library: its object files, bundled into libtextstats.a.
add_library(textstats STATIC textstats.cpp)
# PUBLIC: this library and everything that links to it see include/.
target_include_directories(textstats PUBLIC include)
