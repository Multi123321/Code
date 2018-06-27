project(BasicTrader_c)
cmake_minimum_required(VERSION 2.8.7)

enable_testing()

#set(CMAKE_CXX_COMPILER "g++-5")

OPTION( ARCHITECTURE_OPTIMIZATION   "enables/disables the flag -march=native for gnu c++ compiler (on by default)" ON )
OPTION( ENABLE_WALL "enables/disables the flag -Wall for gnu c++ compiler (on by default)" ON )
OPTION( SPEED_MODE "enables/disables flags which result in a faster executable but disables debug flags (off by default)" ON )
OPTION( PRINT_RESULTS "enables/disables file output (off by deafult)" OFF)
OPTION( SHOW_OPTIMIZATION "enables/disables flags which shows optimization dumps from various optimization passes (off by default)" OFF )

IF( CMAKE_COMPILER_IS_GNUCC OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
   SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wextra -Wshadow -pedantic -fopenmp")
   IF( ARCHITECTURE_OPTIMIZATION )
      SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native" )
   ENDIF()
   IF( ENABLE_WALL )
      SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" )
   ENDIF()
   IF( SPEED_MODE )
    SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -pthread -mtune=native -mavx -DLIKWID_PERFMON -I/opt/apps/x86_64/likwid/4.1/include -L/opt/apps/x86_64/likwid/4.1/lib -llikwid -lm" )
   ELSE()
    SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Winline" )
   ENDIF()
ENDIF()

IF ( PRINT_RESULTS )
    ADD_DEFINITIONS(-DPRINTRES)
ENDIF()

IF( SHOW_OPTIMIZATION )
    IF( CMAKE_COMPILER_IS_GNUCC)
        SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopt-info" )
    ELSEIF( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
        SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -openmp-simd -Rpass=\"loop|vect\" -Rpass-missed=\"loop|vect\" -Rpass-analysis=\"loop|vect\"" )
    ENDIF()
ENDIF() 


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_COMPILE_FLAGS ${CMAKE_CXX_COMPILE_FLAGS})
set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS})

add_subdirectory(src)
# add_subdirectory(tests)
