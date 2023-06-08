cmake_minimum_required (VERSION 3.5)

project ("CXX Coroutines Example")

set (CMAKE_CXX_STANDARD 20)

add_executable (asyncop asyncop.cpp)
add_executable (generator generator.cpp)
add_executable (parser parser.cpp)
add_executable (resumable_rype resumable_type.cpp)
add_executable (return_value return_value.cpp)
