cmake_minimum_required(VERSION 3.26)

project(aparse)

add_executable(tests ../aparse.c test.c)
target_compile_options(tests PUBLIC -g --std=c89 -Wall -Werror -Wextra -pedantic -ferror-limit=0)
target_include_directories(tests SYSTEM PUBLIC ..)
