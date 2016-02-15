
# NOTE: currently this project isn't used by cgdb

cmake_minimum_required(VERSION 2.8)

find_package(BISON REQUIRED)

BISON_TARGET(gdbmi_grammar ${CMAKE_CURRENT_SOURCE_DIR}/gdbmi_grammar.y
                           ${CMAKE_CURRENT_BINARY_DIR}/gdbmi_grammar.c)

FIX_OUTPUT_NAME_AND_ADD_FLEX_TARGET(gdbmi_lexer ${CMAKE_CURRENT_SOURCE_DIR}/gdbmi_lexer.l
                                                ${CMAKE_CURRENT_BINARY_DIR}/gdbmi_lexer_fixed.l
                                                ${CMAKE_CURRENT_BINARY_DIR}/gdbmi_lexer.c)

ADD_FLEX_BISON_DEPENDENCY(gdbmi_lexer gdbmi_grammar)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB GDBMI_SRC "gdbmi_oc.c"
                    "gdbmi_pt.c"
                    "gdbmi_parser.c")

add_library(gdbmi STATIC ${GDBMI_SRC}
                         ${BISON_gdbmi_grammar_OUTPUTS}
                         ${FLEX_gdbmi_lexer_OUTPUTS})

####################################################################################################
# TESTS
####################################################################################################

file(GLOB GDBMI_DRIVER_SRC "gdbmi_driver.c")
add_executable("gdbmi_driver" ${GDBMI_DRIVER_SRC})
target_link_libraries("gdbmi_driver" "gdbmi")

####################################################################################################

