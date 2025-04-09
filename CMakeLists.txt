cmake_minimum_required(VERSION 3.16)
project(ffcli)
find_package(Curses REQUIRED)
add_library(fuzzy_find src/fuzzy_find.cpp)

add_executable(ffcli main.cpp)

target_link_libraries(ffcli PRIVATE ${CURSES_LIBRARIES})
target_include_directories(ffcli PRIVATE ${CURSES_INCLUDE_DIR})

target_include_directories(fuzzy_find PRIVATE src/headers)
target_link_libraries(fuzzy_find PRIVATE ${CURSES_LIBRARIES})
target_link_libraries(ffcli PRIVATE fuzzy_find)
