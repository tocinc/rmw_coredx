# Copyright 2014 Open Source Robotics Foundation, Inc.
# Modifications copyright (C) 2017 Twin Oaks Computing, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.5)

project(rmw_coredx_cpp)

if(NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")
endif()

find_package(ament_cmake REQUIRED)

find_package(coredx_cmake_module REQUIRED)
find_package(CoreDX MODULE)
if(NOT CoreDX_FOUND)
  ament_package()
  message(WARNING "Could not find TOC CoreDX DDS - skipping '${PROJECT_NAME}'")
  return()
endif()

find_package(rosidl_typesupport_coredx_c)
if(NOT rosidl_typesupport_coredx_c_FOUND)
  ament_package()
  message(WARNING "Could not find ROSIDL TypeSupport for TOC CoreDX DDS (rosidl_typesupport_coredx_c) - skipping '${PROJECT_NAME}'")
  return()
endif()

find_package(rosidl_typesupport_coredx_cpp)
if(NOT rosidl_typesupport_coredx_cpp_FOUND)
  ament_package()
  message(WARNING "Could not find ROSIDL TypeSupport for TOC CoreDX DDS (rosidl_typesupport_coredx_cpp) - skipping '${PROJECT_NAME}'")
  return()
endif()

find_package(rcutils REQUIRED)
find_package(rmw REQUIRED)
find_package(rosidl_generator_c REQUIRED)
find_package(rosidl_generator_cpp REQUIRED)

include_directories(include)

ament_export_dependencies(
  rcutils
  rmw
  rosidl_generator_c
  rosidl_generator_cpp
  rosidl_typesupport_coredx_c
  rosidl_typesupport_coredx_cpp
)

register_rmw_implementation(
  "c:rosidl_typesupport_c:rosidl_typesupport_coredx_c"
  "cpp:rosidl_typesupport_cpp:rosidl_typesupport_coredx_cpp"
)

add_library(rmw_coredx_cpp SHARED
  src/coredx_identifier.cpp
  src/custom_listeners.cpp
  src/names.cpp
  src/rmw_compare_gids_equal.cpp
  src/rmw_count_publishers.cpp
  src/rmw_count_subscribers.cpp
  src/rmw_create_client.cpp
  src/rmw_create_guard_condition.cpp
  src/rmw_create_node.cpp
  src/rmw_create_publisher.cpp
  src/rmw_create_service.cpp
  src/rmw_create_subscription.cpp
  src/rmw_create_wait_set.cpp
  src/rmw_destroy_client.cpp
  src/rmw_destroy_guard_condition.cpp
  src/rmw_destroy_node.cpp
  src/rmw_destroy_publisher.cpp
  src/rmw_destroy_service.cpp
  src/rmw_destroy_subscription.cpp
  src/rmw_destroy_wait_set.cpp
  src/rmw_get_gid_for_publisher.cpp
  src/rmw_get_implementation_identifier.cpp
  src/rmw_get_node_names.cpp
  src/rmw_get_service_names_and_types.cpp
  src/rmw_get_topic_names_and_types.cpp
  src/rmw_init.cpp
  src/rmw_node_get_graph_guard_condition.cpp
  src/rmw_publish.cpp
  src/rmw_send_request.cpp
  src/rmw_send_response.cpp
  src/rmw_service_server_is_available.cpp
  src/rmw_take.cpp
  src/rmw_take_request.cpp
  src/rmw_take_response.cpp
  src/rmw_take_with_info.cpp
  src/rmw_trigger_guard_condition.cpp
  src/rmw_wait.cpp
  src/util.cpp
  )
ament_target_dependencies(rmw_coredx_cpp
  "rcutils"
  "rmw"
  "rosidl_generator_c"
  "rosidl_generator_cpp"
  "rosidl_typesupport_coredx_c"
  "rosidl_typesupport_coredx_cpp"
  "CoreDX")

if(CoreDX_GLIBCXX_USE_CXX11_ABI_ZERO)
  target_compile_definitions(rmw_coredx_cpp
    PRIVATE CoreDX_GLIBCXX_USE_CXX11_ABI_ZERO)
endif()

ament_export_libraries(rmw_coredx_cpp ${CoreDX_LIBRARIES})

# On Windows this adds the RMW_BUILDING_DLL definition.
# On Unix (GCC or Clang) it hides the symbols with -fvisibility=hidden.
configure_rmw_library(rmw_coredx_cpp)
# Additionally, on Windows, add the ROSIDL_TYPESUPPORT_COREDX_CPP_BUILDING_DLL definition.

if(WIN32)
  # target_compile_definitions(rmw_coredx_cpp
  #   PRIVATE "_CRT_NONSTDC_NO_DEPRECATE")
  target_compile_definitions(rmw_coredx_cpp
    PRIVATE "ROSIDL_TYPESUPPORT_COREDX_CPP_BUILDING_DLL"
            "RMW_COREDX_SHARED_BUILDING_DLL"
            "COREDX_DLL")
endif()

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()

install(
  TARGETS rmw_coredx_cpp
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
