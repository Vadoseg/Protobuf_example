file(GLOB_RECURSE SOURCES "*.cpp" "*.cc")
file(GLOB_RECURSE HEADERS "*.h")
find_package(PkgConfig REQUIRED)
pkg_check_modules(PROTOBUF REQUIRED protobuf)

add_executable(protobuf_client_example
  ${SOURCES}
  ${HEADERS}
  IClient.h
  ProtoClient.h
  RJClient.h
)

target_link_libraries(protobuf_client_example PRIVATE ${PROTOBUF_LIBRARIES})
target_compile_options(protobuf_client_example PRIVATE
    $<$<CONFIG:Release>:-O3 -march=native -flto>
    $<$<CONFIG:Debug>:-g -O0>
)
