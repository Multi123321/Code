SET( SRCS

   Code.cpp
   CoastlineTrader.cpp
   FXrateTrading.cpp
   HelpClass.cpp
   LocalLiquidity.cpp
   NbDcCounter.cpp
   PriceFeedData.cpp
   Prices.cpp
   Runner.cpp
   CSVReader.cpp
   helper/UsefullFunctions.cpp
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.txt ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.txt COPYONLY)

ADD_EXECUTABLE( code.app ${SRCS} )