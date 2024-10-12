cmake_minimum_required(VERSION 3.15)
project(socketpp CXX)

set(SOURCES
  src/socket.cpp
  src/serversocket.cpp
)

add_library(socketpp STATIC)

target_sources(socketpp
  PRIVATE ${SOURCES}
)

target_include_directories(socketpp
  PRIVATE   include/socketpp/
  INTERFACE include/
)

target_compile_features(socketpp
  INTERFACE cxx_std_23 
)