cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set( CMAKE_CXX_COMPILER "C:/msys64/mingw64/bin/g++.exe" )
set( CMAKE_C_COMPILER "C:/msys64/mingw64/bin/gcc.exe" )

project(TooseChat)

set(SOURCES src/main.cpp src/client.cpp src/server.cpp)
add_executable(TooseChat ${SOURCES})
if(WIN32)
  target_link_libraries(TooseChat wsock32 ws2_32)
endif()

target_include_directories(TooseChat PUBLIC
                           "include"
                           )

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall -g")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall")

