aux_source_directory(./LimitOrders SRC_LIST)
add_library(${PROJECT_NAME} SHARED ${SRC_LIST})