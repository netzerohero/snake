cmake_minimum_required(VERSION 3.13)

project(MY-RP6502-PROJECT C ASM)

add_subdirectory(tools)

add_executable(snake)
rp6502_executable(snake)
target_sources(snake PRIVATE
    src/snake.c
    src/basgraf.c
    src/controller.s
    )
