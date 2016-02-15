
cmake_minimum_required(VERSION 2.8)

project(tgdb_a2)

include_directories("../tgdb-base"
                    "../../adt"
                    "../../util")

file(GLOB A2_SRC "a2-tgdb.c"
                 "annotate.c"
                 "commands.c"
                 "data.c"
                 "globals.c"
                 "state_machine.c")

add_library(${PROJECT_NAME} STATIC ${A2_SRC})

