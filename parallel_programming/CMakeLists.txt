cmake_minimum_required(VERSION 3.26)
project(parallel_programming)



set(CMAKE_CXX_STANDARD 17)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp")


add_executable(parallel_programming main.cpp
        bmp_reader.h
        bmp_reader.cpp
        methods.h
        methods.cpp
)


add_executable(for_process for_process.cpp
        bmp_reader.h
        bmp_reader.cpp
        methods.h
        methods.cpp
)

