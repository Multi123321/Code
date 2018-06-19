project(BasicTrader_c)
cmake_minimum_required(VERSION 2.8.7)

enable_testing()

OPTION( ARCHITECTURE_OPTIMIZATION   "enables/disables the flag -march=native for gnu c++ compiler (on by default)" ON )
OPTION( ENABLE_WALL "enables/disables the flag -Wall for gnu c++ compiler (on by default)" ON )
OPTION( SPEED_MODE "enables/disables flags which result in a faster executable but disables debug flags (off by default)" OFF )
OPTION( SHOW_OPTIMIZATION "enables/disables flags which shows optimization dumps from various optimization passes (off by default)" OFF )


IF( CMAKE_COMPILER_IS_GNUCC OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wextra -Wshadow -pedantic")
   IF( ARCHITECTURE_OPTIMIZATION )
      SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native" )
   ENDIF()
   IF( ENABLE_WALL )
      SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" )
   ENDIF()
   IF( SHOW_OPTIMIZATION )
    SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopt-info" )
   ENDIF()
   IF( SPEED_MODE )
    SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math -DNDEBUG" )
   ELSE()
    SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Winline" )
   ENDIF()
ENDIF()


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_COMPILE_FLAGS ${CMAKE_CXX_COMPILE_FLAGS})
set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS})

add_subdirectory(src)
add_subdirectory(tests)