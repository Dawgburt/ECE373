cmake_minimum_required(VERSION 3.22)
project(_3 C)

set(CMAKE_C_STANDARD 11)

add_executable(_3
        hw3_pci_driver.c
        "test_chardev(USERSPACE).c")
