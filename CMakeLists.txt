cmake_minimum_required(VERSION 3.10)
project(sea_battle)
find_package(SQLite3 REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lsqlite3")

set(SOURCE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CLIENT_DIR ${INCLUDE_DIR}/client)
set(SERVER_DIR ${INCLUDE_DIR}/server)
set(BATTLEFIELD_DIR ${INCLUDE_DIR}/battlefield)
set(MESSAGE_DIR ${INCLUDE_DIR}/message)
set(START_DIR ${CMAKE_CURRENT_SOURCE_DIR}/start)

include_directories(${BATTLEFIELD_DIR})
include_directories(${SERVER_DIR})
include_directories(${MESSAGE_DIR})
include_directories(${CLIENT_DIR})
include_directories(${SQLite3_INCLUDE_DIRS})

add_library(server_lib ${SOURCE_DIR}/server.cpp)
add_library(battlefield_dir ${SOURCE_DIR}/battlefield.cpp)
add_library(message_dir ${SOURCE_DIR}/message.cpp)
add_library(client_lib ${SOURCE_DIR}/client.cpp ${SOURCE_DIR}/text_art.cpp)

add_executable(client.exe ${START_DIR}/client_start.cpp)
add_executable(server.exe ${START_DIR}/server_start.cpp)

target_link_libraries(server.exe PRIVATE server_lib battlefield_dir message_dir ${SQLite3_LIBRARIES})
target_link_libraries(client.exe PRIVATE client_lib battlefield_dir message_dir ${SQLite3_LIBRARIES})
