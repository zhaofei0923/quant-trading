cmake_minimum_required(VERSION 3.22.1)
project (QuantTradeServer)
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

# Resolve absolute library directories for robust linking
get_filename_component(CTP_LIB_DIR "${CMAKE_SOURCE_DIR}/${CTP_LIBRARY}" REALPATH)
get_filename_component(SQLITE_LIB_DIR "${CMAKE_SOURCE_DIR}/${SQLITE_LIBRARY}" REALPATH)

include_directories(${JSON_INCLUDE_DIR}  ${LOG_INCLUDE_DIR}  ${CTP_INCLUDE_DIR}   ${SQLITE_INCLUDE_DIR})
aux_source_directory (. SRC)

link_directories(${CTP_LIBRARY} ${SQLITE_LIBRARY})

# Define imported shared/static libs to avoid -l resolution hitting broken placeholders
add_library(thosttraderapi_se SHARED IMPORTED GLOBAL)
set_target_properties(thosttraderapi_se PROPERTIES IMPORTED_LOCATION "${CTP_LIB_DIR}/thosttraderapi_se.so")

add_library(thostmduserapi_se SHARED IMPORTED GLOBAL)
set_target_properties(thostmduserapi_se PROPERTIES IMPORTED_LOCATION "${CTP_LIB_DIR}/thostmduserapi_se.so")

add_library(LinuxDataCollect SHARED IMPORTED GLOBAL)
set_target_properties(LinuxDataCollect PROPERTIES IMPORTED_LOCATION "${CTP_LIB_DIR}/LinuxDataCollect.so")

add_library(sqlite3_external STATIC IMPORTED GLOBAL)
set_target_properties(sqlite3_external PROPERTIES IMPORTED_LOCATION "${SQLITE_LIB_DIR}/libsqlite3.a")
add_executable(QuantTradeServer ${SRC})
# Link against real ELF .so files present in ctp/sqlite folders.
# Some "lib*.so" files in repo are text placeholders (broken symlinks).
target_link_libraries(QuantTradeServer
    pthread
    thosttraderapi_se
    thostmduserapi_se
    LinuxDataCollect
    sqlite3_external
)
