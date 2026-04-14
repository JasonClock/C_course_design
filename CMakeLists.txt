cmake_minimum_required(VERSION 3.20)
project(C C)

set(CMAKE_C_STANDARD 99)

add_executable(C
        main.c
        information/src/hotel.c
        information/src/menu.c
        information/src/role.c
        information/src/input.c)

target_include_directories(C PRIVATE information/inc)

if (MSVC)
    target_compile_options(C PRIVATE /utf-8)
else()
    target_compile_options(C PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
