cmake_minimum_required(VERSION 3.10)
project(YAVM C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(YAVM
        chunk.c
        chunk.h
        commons.h
        compiler.c
        compiler.h
        debug.c
        debug.h
        main.c
        memory.c
        memory.h
        scanner.c
        scanner.h
        value.c
        value.h
        vm.c
        vm.h object.h object.c table.h table.c)
