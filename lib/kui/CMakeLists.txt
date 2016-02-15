
cmake_minimum_required(VERSION 2.8)

file(GLOB ADT_SRC "kui.c"
                  "kui_term.c"
                  "kui_tree.c")

include_directories("../util"
                    "../adt")

add_library("kui" STATIC ${ADT_SRC})

####################################################################################################
# TESTS
####################################################################################################

file(GLOB KUI_DRIVER_SRC "kui_driver.c")
add_executable("kui_driver" ${KUI_DRIVER_SRC})
target_link_libraries("kui_driver" "kui" "adt" "util" "${NCURSES_LIB}")

####################################################################################################

