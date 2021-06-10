cmake_minimum_required(VERSION 3.17)
project(cpp-nbt CXX)

set(RUN_COVERAGE OFF CACHE BOOL "Compile with coverage options")

if(MSVC)
  set(WARNINGS /W4;/WX)
else()
  set(WARNINGS -Wall;-Wextra;-pedantic;-Werror;-Wno-unused-value)
endif()

include_directories(${CMAKE_SOURCE_DIR})

include(CTest)

function(setup_test name)
  set(target ${name}_bin)
  add_executable(${target} tests/test_${name}.cpp)
  target_compile_options(${target} PRIVATE ${WARNINGS})
  target_compile_features(${target} PRIVATE cxx_std_20)
  if(RUN_COVERAGE)
    target_compile_options(${target} PRIVATE -fprofile-arcs;-ftest-coverage;-fno-inline)
    target_link_options(${target} PRIVATE --coverage)
  endif()
  add_test(
      NAME test_${name}
      COMMAND ${CMAKE_BINARY_DIR}/${target}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/data
  )
endfunction(setup_test)

setup_test(numerics)
setup_test(string)
setup_test(arrays)
setup_test(list)
setup_test(compound)
