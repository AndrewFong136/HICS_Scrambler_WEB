cmake_minimum_required(VERSION 3.10)
project(scrambler)

set(CMAKE_CXX_STANDARD 17)
set(mysql-concpp_DIR "/path/to/HICS_Scrambler_WEB/source/mysqlx")
find_package(mysql-concpp)

# Adjust this path to wherever you installed the Connector/C++ headers
include_directories(mysqlx/include)

add_subdirectory(pybind11)

pybind11_add_module(scrambler scrambler.cpp)

target_link_libraries(scrambler PRIVATE mysql::concpp-static)
