cmake_minimum_required(VERSION 3.17)
project(cpp-nbt CXX)

include_directories(${CMAKE_SOURCE_DIR} example/include)
add_executable(main example/example.cpp)
target_link_libraries(main z)
target_compile_options(main PRIVATE -Wall -Wextra -pedantic)
target_compile_features(main PRIVATE cxx_std_20)


include(CTest)

function(setup_test name)
  set(target ${name}_bin)
  add_executable(${target} tests/test_${name}.cpp)
  target_compile_options(${target} PRIVATE -Wall -Wextra -pedantic -Wno-unused-value -Werror)
  target_compile_features(${target} PRIVATE cxx_std_20)
  add_test(
      NAME test_${name}
      COMMAND ${CMAKE_BINARY_DIR}/${target}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/data
  )
endfunction(setup_test)

setup_test(numerics)
setup_test(string)
#setup_test(arrays)
