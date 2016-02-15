
cmake_minimum_required(VERSION 2.8)

project(tgdb)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    "../annotate-two"
                    "../gdbmi"
                    "../../adt"
                    "../../rline"
                    "../../util")

file(GLOB TGDB_SRC "tgdb.c"
                   "tgdb_client_interface.c"
                   "tgdb_command.c"
                   "tgdb_types.c")

add_library(${PROJECT_NAME} STATIC ${TGDB_SRC})

####################################################################################################
# TESTS
####################################################################################################

file(GLOB TGDB_DRIVER_SRC "driver.c")
add_executable("driver" ${TGDB_DRIVER_SRC})
target_link_libraries("driver" "tgdb"
                               "tgdb_a2"
                               "tgdb_mi"
                               "rline"
                               "adt"
                               "util"
                               ${READLINE_LIB})

####################################################################################################



