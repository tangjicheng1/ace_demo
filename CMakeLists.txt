cmake_minimum_required(VERSION 3.20)

project(ace_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3 -Wall -Wextra ")

add_executable(reactor_server src/reactor_server.cpp)
target_link_libraries(reactor_server ACE)
