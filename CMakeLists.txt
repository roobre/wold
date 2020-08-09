cmake_minimum_required(VERSION 3.18)
project(wold C)

set(CMAKE_C_STANDARD 99)

add_executable(wold main.c wold.c wold.h wold_cmd_callback.c wold_cmd_callback.h)
