cmake_minimum_required(VERSION 2.8)
project(fryzjer)
add_executable(${PROJECT_NAME} "main.c" "queue.h" "queue.c")
SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-pthread")
