cmake_minimum_required(VERSION 3.23)
project(PA4)

set(CMAKE_CXX_STANDARD 14)

add_executable(label label.cpp queue.h queue.cpp bmplib.cpp bmplib.h)
