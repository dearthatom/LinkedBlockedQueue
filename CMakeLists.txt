cmake_minimum_required(VERSION 3.15)
project(block_queue)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(block_queue main.cpp LinkedBlockingQueue.h)