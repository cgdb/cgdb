
cmake_minimum_required(VERSION 2.8)

project(rline)

include_directories("../adt"
                    "../util")

file(GLOB RLINE_SRC "rline.c")

add_library(${PROJECT_NAME} STATIC ${RLINE_SRC})

