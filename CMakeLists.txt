cmake_minimum_required(VERSION 3.10)
project(sea_battle)
find_package(SQLite3 REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lsqlite3")

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SERVER_CLIENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/client_server)

include_directories(${INCLUDE_DIR})
include_directories(${SQLite3_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES false ${SOURCE_DIR}/*.cpp)

add_library(sea_battle_lib ${SOURCES})
add_executable(client.exe ${SERVER_CLIENT_DIR}/client.cpp)
add_executable(server.exe ${SERVER_CLIENT_DIR}/server.cpp)

target_link_libraries(server.exe PRIVATE ${CMAKE_PROJECT_NAME}_lib ${SQLite3_LIBRARIES})
target_link_libraries(client.exe PRIVATE ${CMAKE_PROJECT_NAME}_lib ${SQLite3_LIBRARIES})
