# Copyright 2016 Open Source Robotics Foundation, Inc.
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

project(rosidl_typesupport_coredx_c)

if(NOT WIN32)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wpedantic")
endif()

find_package(ament_cmake REQUIRED)

find_package(coredx_cmake_module REQUIRED)
find_package(CoreDX MODULE)
if(NOT CoreDX_FOUND)
  ament_package()
  message(WARNING "Could not find TOC CoreDX DDS - skipping '${PROJECT_NAME}'")
  return()
endif()

find_package(ament_cmake_python REQUIRED)
find_package(rosidl_typesupport_coredx_cpp REQUIRED)

ament_export_dependencies(rmw)
ament_export_dependencies(rosidl_cmake)
ament_export_dependencies(rosidl_generator_c)
ament_export_dependencies(rosidl_generator_dds_idl)
ament_export_dependencies(rosidl_typesupport_coredx_cpp)
ament_export_include_directories(include)

ament_python_install_package(${PROJECT_NAME})

add_library(${PROJECT_NAME} SHARED src/identifier.cpp)
if(WIN32)
  target_compile_definitions(${PROJECT_NAME}
    PRIVATE "ROSIDL_TYPESUPPORT_COREDX_C_BUILDING_DLL")
endif()
target_include_directories(${PROJECT_NAME} PUBLIC include)
ament_target_dependencies(${PROJECT_NAME} "rosidl_typesupport_coredx_cpp")
ament_export_libraries(${PROJECT_NAME})

ament_index_register_resource("rosidl_typesupport_c")

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package(
  CONFIG_EXTRAS "rosidl_typesupport_coredx_c-extras.cmake.in"
)

install(
  PROGRAMS bin/rosidl_typesupport_coredx_c
  DESTINATION lib/rosidl_typesupport_coredx_c
)

install(
  DIRECTORY cmake resource
  DESTINATION share/${PROJECT_NAME}
)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
