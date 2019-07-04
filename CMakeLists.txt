cmake_minimum_required(VERSION 3.10)
project(Practice)

set(CMAKE_CXX_STANDARD 11)


include_directories(include)

link_directories(lib)


add_subdirectory(tools)


add_executable(Practice shell/main.cpp)

target_link_libraries(Practice tools pthread)