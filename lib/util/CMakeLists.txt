
cmake_minimum_required(VERSION 2.8)

project(util)

file(GLOB UTIL_SRC "fork_util.c"
                   "fs_util.c"
                   "io.c"
                   "logger.c"
                   "pseudo.c"
                   "sys_util.c"
                   "terminal.c")

add_library(${PROJECT_NAME} STATIC ${UTIL_SRC})

