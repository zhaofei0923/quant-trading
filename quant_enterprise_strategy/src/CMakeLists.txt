cmake_minimum_required(VERSION 3.22.1)
project (QuantStrategyClient)
message("-------------------start cmake-------------------")

add_compile_options(-O2 -std=c++11)
SET( CMAKE_CXX_FLAGS "-std=c++11 -O3 -g")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
message(${CMAKE_RUNTIME_OUTPUT_DIRECTORY})


set(JSON_INCLUDE_DIR ../../other_depend/rapidjson-master/include)
set(LOG_INCLUDE_DIR ../../other_depend/spdlog-1.x/include)

include_directories(${JSON_INCLUDE_DIR}  ${LOG_INCLUDE_DIR}  )

aux_source_directory (. SRC)
add_executable (QuantStrategyClient ${SRC})
target_link_libraries( QuantStrategyClient   pthread  )
