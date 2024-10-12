cmake_minimum_required(VERSION 3.15)
project(learn_socket CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(socketpp)


add_executable( server
  src/server.cpp
)
target_link_libraries(server
  socketpp
)

add_executable( client 
  src/client.cpp
)
target_link_libraries( client
  socketpp
)