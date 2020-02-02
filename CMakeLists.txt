# For now works only with clang:
# CXX=/usr/bin/clang++-9 CC=/usr/bin/clang-9 cmake ..
cmake_minimum_required (VERSION 3.15)

project (CoRoutinesExamples
    VERSION 1.0
    DESCRIPTION "Show some C++ coroutines examples"
    LANGUAGES CXX)


add_executable (resumable_type resumable_type.cpp)
set_target_properties (resumable_type PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
target_compile_options (resumable_type PRIVATE -stdlib=libc++)
target_link_libraries (resumable_type PRIVATE -stdlib=libc++)
