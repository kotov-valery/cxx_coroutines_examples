# For now works only with clang:
# CXX=/usr/bin/clang++-9 CC=/usr/bin/clang-9 cmake ..
cmake_minimum_required (VERSION 3.15)

project (CoRoutinesExamples
    VERSION 1.0
    DESCRIPTION "Show some C++ coroutines examples"
    LANGUAGES CXX)


# Simple resumable type
add_executable (resumable_type resumable_type.cpp)
set_target_properties (resumable_type PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
target_compile_options (resumable_type PRIVATE -stdlib=libc++)
target_link_libraries (resumable_type PRIVATE -stdlib=libc++)


# Return a value from a coroutine
add_executable (return_value return_value.cpp)
set_target_properties (return_value PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
target_compile_options (return_value PRIVATE -stdlib=libc++)
target_link_libraries (return_value PRIVATE -stdlib=libc++)

# Coroutine generator
add_executable (generator generator.cpp)
set_target_properties (generator PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
target_compile_options (generator PRIVATE -stdlib=libc++)
target_link_libraries (generator PRIVATE -stdlib=libc++)

# Async operation with coroutine
add_executable (asyncop asyncop.cpp)
set_target_properties (asyncop PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
target_compile_options (asyncop PRIVATE -stdlib=libc++)
target_link_libraries (asyncop PRIVATE -stdlib=libc++ -pthread)