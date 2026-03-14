cmake_minimum_required(VERSION 3.8)
project(autocube_client)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()
set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -O3")

# find dependencies
find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

find_package(Protobuf REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GRPC REQUIRED grpc++)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/UserCommand.msg"
  DEPENDENCIES std_msgs geometry_msgs
)
ament_export_dependencies(rosidl_default_runtime)

include_directories(
  ${GRPC_INCLUDE_DIRS}
  ${Protobuf_INCLUDE_DIRS}
)

link_directories(${GRPC_LIBRARY_DIRS})

# proto 文件目录
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
file(GLOB PROTO_FILES "${PROTO_DIR}/*.proto")

set(GENERATED_SRCS "")
set(GENERATED_HDRS "")

foreach(proto_file ${PROTO_FILES})
  get_filename_component(proto_name ${proto_file} NAME_WE)
  set(pb_src "${CMAKE_CURRENT_BINARY_DIR}/${proto_name}.pb.cc")
  set(pb_hdr "${CMAKE_CURRENT_BINARY_DIR}/${proto_name}.pb.h")
  set(grpc_src "${CMAKE_CURRENT_BINARY_DIR}/${proto_name}.grpc.pb.cc")
  set(grpc_hdr "${CMAKE_CURRENT_BINARY_DIR}/${proto_name}.grpc.pb.h")

  add_custom_command(
    OUTPUT ${pb_src} ${pb_hdr} ${grpc_src} ${grpc_hdr}
    COMMAND protoc
    ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
      --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
      --plugin=protoc-gen-grpc=/usr/bin/grpc_cpp_plugin
      -I ${PROTO_DIR}
      ${proto_file}
    DEPENDS ${proto_file}
  )

  list(APPEND GENERATED_SRCS ${pb_src} ${grpc_src})
  list(APPEND GENERATED_HDRS ${pb_hdr} ${grpc_hdr})
endforeach()

# 包含生成的头文件
include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}
)

# library 和 executable
ament_auto_add_library(${PROJECT_NAME}_lib SHARED
  src/autocube_client_node.cpp
  ${GENERATED_SRCS}      # ✅ 生成的 proto/grpc 文件
)
rosidl_target_interfaces(${PROJECT_NAME}_lib
  ${PROJECT_NAME} "rosidl_typesupport_cpp")
  
ament_auto_add_executable(${PROJECT_NAME}_node
  src/main.cpp
  ${GENERATED_SRCS}      # 如果需要
)

target_link_libraries(${PROJECT_NAME}_node
  ${GRPC_LIBRARIES}
  ${Protobuf_LIBRARIES}
  pthread
)

install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})
install(DIRECTORY config DESTINATION share/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_auto_package()
