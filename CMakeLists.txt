# CMakeList.txt : CMake project for Project, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("Project")
set (CMAKE_CXX_STANDARD 17)

# Add source to this project's executable.
add_executable (Project "headers.h" "main.c" "utils.h" "utils.c" "files.h" "files.c")

# TODO: Add tests and install targets if needed.
