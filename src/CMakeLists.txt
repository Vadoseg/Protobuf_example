file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.h")
find_package(PkgConfig REQUIRED)
pkg_check_modules(PROTOBUF REQUIRED protobuf)

set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
set(GEN_DIR "${PROTO_DIR}/gen")
file(GLOB_RECURSE PROTO_FILES "${PROTO_DIR}/*.proto")

file(MAKE_DIRECTORY "${GEN_DIR}")
execute_process(
  COMMAND protoc --cpp_out=${GEN_DIR} -I${PROTO_DIR} ${PROTO_FILES}
  RESULT_VARIABLE PROTOC_RESULT
  ERROR_VARIABLE PROTOC_ERROR
)

if(NOT PROTOC_RESULT EQUAL 0)
  message(FATAL_ERROR "Protobuf pre-build generation failed: ${PROTOC_ERROR}")
endif()

file(GLOB_RECURSE PROTO_SOURCES "*.pb.cc")
file(GLOB_RECURSE PROTO_HEADERS "*.pb.h")

source_group("Gen Protos" FILES ${PROTO_SOURCES} ${PROTO_HEADERS})

add_executable(protobuf_examples
  ${SOURCES}
  ${HEADERS}
  ${PROTO_SOURCES}
  ${PROTO_HEADERS}
)

set(TEXT_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/config_dsp_test_sweep.textproto")
set(BIN_CONFIG  "${CMAKE_CURRENT_BINARY_DIR}/config_dsp_test_sweep.pb")
set(PROTO_MESSAGE "MainConfig")

target_include_directories(protobuf_examples PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/common
  ${CMAKE_CURRENT_SOURCE_DIR}/server
  ${CMAKE_CURRENT_SOURCE_DIR}/proto
)

target_link_libraries(protobuf_examples PRIVATE ${PROTOBUF_LIBRARIES})
target_compile_options(protobuf_examples PRIVATE ${PROTOBUF_CFLAGS_OTHER})

include(GNUInstallDirs)
install(TARGETS protobuf_examples
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


