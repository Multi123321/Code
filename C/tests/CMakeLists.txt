find_package(Boost REQUIRED)
include_directories(../src)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests/)

file(GLOB cxx_files *_test.cpp)
foreach(filename ${cxx_files})
  get_filename_component(target ${filename} NAME_WE)
  add_executable(${target} ${filename})
  target_link_libraries(${target} ${PROJECT_NAME} boost_unit_test_framework)
  add_test(NAME ${target}
           COMMAND ${target})
endforeach(filename)
