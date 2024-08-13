cmake_minimum_required(VERSION 3.22.1)
project (QuantMarketServer)
message("-------------------start cmake-------------------")

add_compile_options(-O2 -std=c++11)
SET( CMAKE_CXX_FLAGS "-std=c++11 -O3 -g")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
message(${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

set(JSON_INCLUDE_DIR ../../other_depend/rapidjson-master/include)
set(LOG_INCLUDE_DIR ../../other_depend/spdlog-1.x/include)
set(CTP_INCLUDE_DIR ../../other_depend/ctp/linux64/include)
set(SQLITE_INCLUDE_DIR ../../other_depend/sqlite/include)

set(CTP_LIBRARY  ../../other_depend/ctp/linux64/lib/)
set(SQLITE_LIBRARY  ../../other_depend/sqlite/lib/)

include_directories(${JSON_INCLUDE_DIR}  ${LOG_INCLUDE_DIR}  ${CTP_INCLUDE_DIR}   ${SQLITE_INCLUDE_DIR})
aux_source_directory (. SRC)

link_directories(${CTP_LIBRARY} ${SQLITE_LIBRARY} )
add_executable (QuantMarketServer ${SRC})
target_link_libraries( QuantMarketServer   pthread  libthosttraderapi_se.so  libthostmduserapi_se.so  libLinuxDataCollect.so  libquant_sqlite3.so  )
