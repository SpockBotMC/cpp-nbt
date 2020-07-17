cmake_minimum_required(VERSION 3.17)
project(cpp-nbt CXX)

include_directories("include" "../nbt")
add_executable(main example.cpp)
target_link_libraries(main z)
target_compile_options(main PRIVATE -Wall -pedantic)
target_compile_features(main PRIVATE cxx_std_20)
