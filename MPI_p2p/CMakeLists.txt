cmake_minimum_required(VERSION 3.20)
project (MPI_pp)

set(CMAKE_CXX_STANDARD 14)


include_directories(SYSTEM ${MPI_INCLUDE_PATH})
# Add MPI Package to Project
find_package(MPI REQUIRED)


add_executable(MPI_pp main.cpp
        bmp_reader.cpp
        bmp_reader.h
)
# Add libraries for code completion and compiling
target_link_libraries(MPI_pp PUBLIC MPI::MPI_CXX)


