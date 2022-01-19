cmake_minimum_required(VERSION 3.5)

project(parse_fix_file_with_thread_pool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(parse_fix_file_with_thread_pool main.cpp)
