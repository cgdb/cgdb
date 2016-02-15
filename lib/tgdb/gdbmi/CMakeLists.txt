
cmake_minimum_required(VERSION 2.8)

project(tgdb_mi)

include_directories("../tgdb-base"
                    "../../adt"
                    "../../util")

file(GLOB MI_SRC "gdbmi_tgdb.c")

add_library(${PROJECT_NAME} STATIC ${MI_SRC})

