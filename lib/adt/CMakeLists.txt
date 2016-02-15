
cmake_minimum_required(VERSION 2.8)

file(GLOB ADT_SRC "ibuf.c"
                  "queue.c"
                  "tgdb_list.c"
                  "std_bbtree.c"
                  "std_btree.c"
                  "std_hash.c"
                  "std_list.c")

include_directories("../util")

add_library("adt" STATIC ${ADT_SRC})

####################################################################################################
# TESTS
####################################################################################################

file(GLOB IBUF_DRIVER_SRC "ibuf_driver.c")
add_executable("ibuf_driver" ${IBUF_DRIVER_SRC})
target_link_libraries("ibuf_driver" "adt" "util")

####################################################################################################

file(GLOB STD_HASH_DRIVER_SRC "std_hash_driver.c")
add_executable("std_hash_driver" ${STD_HASH_DRIVER_SRC})
target_link_libraries("std_hash_driver" "adt" "util")

####################################################################################################

file(GLOB STD_LIST_DRIVER_SRC "std_list_driver.c")
add_executable("std_list_driver" ${STD_LIST_DRIVER_SRC})
target_link_libraries("std_list_driver" "adt" "util")

####################################################################################################

file(GLOB STD_BTREE_DRIVER_SRC "std_btree_driver.c")
add_executable("std_btree_driver" ${STD_BTREE_DRIVER_SRC})
target_link_libraries("std_btree_driver" "adt" "util")

####################################################################################################

file(GLOB STD_BBTREE_DRIVER_SRC "std_bbtree_driver.c")
add_executable("std_bbtree_driver" ${STD_BBTREE_DRIVER_SRC})
target_link_libraries("std_bbtree_driver" "adt" "util")

####################################################################################################

