SET( SRCS

   Code.cpp
   # more *.cpp files follow here
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.txt ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.txt COPYONLY)

ADD_EXECUTABLE( code.app ${SRCS} )