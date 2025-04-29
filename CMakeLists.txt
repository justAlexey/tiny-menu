cmake_minimum_required(VERSION 3.22)

set(PROJECT_NAME menu)
project(${PROJECT_NAME})
enable_language(C)

add_library(${PROJECT_NAME} STATIC)

target_include_directories(${PROJECT_NAME} PUBLIC
    inc
)

target_sources(${PROJECT_NAME} PRIVATE
    src/menu.c
)