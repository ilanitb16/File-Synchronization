cmake_minimum_required(VERSION 3.28)
project(File_Synchronization C)

set(CMAKE_C_STANDARD 11)

add_executable(File_Synchronization
#        Part1/part1.c
#        Part2/part2.c
#        Part4/part4.c)
        Part3/buffered_open.h
#        Part3/main.c
        Part3/buffered_open.c
        Part3/test1.c)
