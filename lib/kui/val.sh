valgrind -v --gen-suppressions=yes --suppressions=kui_driver.supp --num-callers=20 --show-below-main=no --leak-check=yes  ./kui_driver
