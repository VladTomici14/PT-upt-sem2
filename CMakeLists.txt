cmake_minimum_required(VERSION 3.29)
project(Programming_Techniques C)

set(CMAKE_C_STANDARD 11)

set(GCC_CUSTOM_COMPILE_FLAGS "-I/usr/include -lm")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_CUSTOM_COMPILE_FLAGS}" )

add_executable(Programming_Techniques main.c)


