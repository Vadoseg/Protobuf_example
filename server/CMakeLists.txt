file(GLOB_RECURSE SOURCES "*.cpp" "*.cc")
file(GLOB_RECURSE HEADERS "*.h")
find_package(PkgConfig REQUIRED)
pkg_check_modules(PROTOBUF REQUIRED protobuf)

add_executable(protobuf_server_example
  ${SOURCES}
  ${HEADERS}
  IServer.h
  ProtoServer.h
  RJServer.h
)

target_link_libraries(protobuf_server_example PRIVATE ${PROTOBUF_LIBRARIES})
target_compile_options(protobuf_server_example PRIVATE ${PROTOBUF_CFLAGS_OTHER})
