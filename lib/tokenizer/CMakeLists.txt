
cmake_minimum_required(VERSION 2.8)

FIX_OUTPUT_NAME_AND_ADD_FLEX_TARGET(ada_lexer ${CMAKE_CURRENT_SOURCE_DIR}/adalexer.l
                                              ${CMAKE_CURRENT_BINARY_DIR}/adalexer_fixed.l
                                              ${CMAKE_CURRENT_BINARY_DIR}/adalexer.c)
FIX_OUTPUT_NAME_AND_ADD_FLEX_TARGET(c_lexer ${CMAKE_CURRENT_SOURCE_DIR}/clexer.l
                                            ${CMAKE_CURRENT_BINARY_DIR}/clexer_fixed.l
                                            ${CMAKE_CURRENT_BINARY_DIR}/clexer.c)
FIX_OUTPUT_NAME_AND_ADD_FLEX_TARGET(d_lexer ${CMAKE_CURRENT_SOURCE_DIR}/dlexer.l
                                            ${CMAKE_CURRENT_BINARY_DIR}/dlexer_fixed.l
                                            ${CMAKE_CURRENT_BINARY_DIR}/dlexer.c)
FIX_OUTPUT_NAME_AND_ADD_FLEX_TARGET(go_lexer ${CMAKE_CURRENT_SOURCE_DIR}/golexer.l
                                             ${CMAKE_CURRENT_BINARY_DIR}/golexer_fixed.l
                                             ${CMAKE_CURRENT_BINARY_DIR}/golexer.c)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    "../adt"
                    "../util")

file(GLOB TOKENIZER_SRC "tokenizer.c")

add_library(tokenizer STATIC ${TOKENIZER_SRC}
                             ${FLEX_ada_lexer_OUTPUTS}
                             ${FLEX_c_lexer_OUTPUTS}
                             ${FLEX_d_lexer_OUTPUTS}
                             ${FLEX_go_lexer_OUTPUTS})

####################################################################################################
# TESTS
####################################################################################################

file(GLOB TOKENIZER_DRIVER_SRC "tokenizer_driver.c")
add_executable("tokenizer_driver" ${TOKENIZER_DRIVER_SRC})
target_link_libraries("tokenizer_driver" "tokenizer" "adt" "util")

####################################################################################################

