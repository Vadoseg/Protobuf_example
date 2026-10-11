cmake_minimum_required(VERSION 3.16)

project(protobuf_examples LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_subdirectory(src)
add_subdirectory(server)
add_subdirectory(client)
