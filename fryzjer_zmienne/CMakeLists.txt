cmake_minimum_required(VERSION 2.8)
project(fryzjerv2)
add_executable(${PROJECT_NAME} "main.c" "queue.c" "queue.h")
SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-pthread")
