cmake_minimum_required(VERSION 3.26)

project(aparse)

add_executable(tests aparse.c test/test.c)
target_compile_options(tests PUBLIC -g --std=c89 -Wall -Werror -Wextra -pedantic)
target_include_directories(tests SYSTEM PUBLIC .)
