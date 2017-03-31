# Copyright 2014-2015 Open Source Robotics Foundation, Inc.
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

###############################################################################
#
# CMake module for finding TOC CoreDX DDS.
#
# Input variables:
#
# - COREDX_TOP, COREDX_TARGET, COREDX_HOST:
#   When specified, header files, libraries, and tools
#   will be searched for in `${COREDX_TOP}/target/include`,
#   and `${COREDX_TOP}/target/${COREDX_TARGET}/lib`, and
#   `${COREDX_TOP}/host/${COREDX_HOST}/bin` respectively.
#
# Output variables:
#
# - CoreDX_FOUND: flag indicating if the package was found
# - CoreDX_INCLUDE_DIRS: Paths to the header files
# - CoreDX_HOME: Root directory for the NDDS install.
# - CoreDX_LIBRARIES: Name to the C++ libraries including the path
# - CoreDX_LIBRARY_DIRS: Paths to the libraries
# - CoreDX_LIBRARY_DIR: Path to libraries; guaranteed to be a single path
# - CoreDX_DEFINITIONS: Definitions to be passed on
# - CoreDX_DDSGEN: Path to the idl code generator
#
# Example usage:
#
#   find_package(coredx_cmake_module REQUIRED)
#   find_package(CoreDX MODULE)
#   # use CoreDX_* variables
#
###############################################################################

# lint_cmake: -convention/filename, -package/stdargs
#message("READING coredx_cmake_modules/cmake/Modules/FindCoreDX.cmake....")

set(CoreDX_FOUND FALSE)

# check if all libraries with an expected name have been found
function(_find_coredx_libraries var_count var_found expected_library_names libraries)
  set(found_libraries "")
  set(found_all_libraries TRUE)
  foreach(library_path ${libraries})
    get_filename_component(library_name "${library_path}" NAME)
    list(GET expected_library_names "${library_name}" index)
    if(index EQUAL -1)
      set(found_all_libraries FALSE)
    else()
      list(APPEND found_libraries "${library_path}")
    endif()
  endforeach()
  list(LENGTH found_libraries length)
  set(${var_count} ${length} PARENT_SCOPE)
  set(${var_found} ${found_all_libraries} PARENT_SCOPE)
endfunction()

set(_expected_library_base_names
  "dds_rpc_cpp"
  "dds_cpp_cf"
  "dds_cf"
)

set(_expected_library_names "")
foreach(_base_name IN LISTS _expected_library_base_names)
  if(WIN32)
    list(APPEND _expected_library_names "${_base_name}.lib")
  elseif(APPLE)
    list(APPEND _expected_library_names "lib${_base_name}.dylib")
  else()
    list(APPEND _expected_library_names "lib${_base_name}.so")
  endif()
endforeach()

file(TO_CMAKE_PATH "$ENV{COREDX_TOP}"    _COREDX_TOP)
file(TO_CMAKE_PATH "$ENV{COREDX_TARGET}" _COREDX_TARGET)
file(TO_CMAKE_PATH "$ENV{COREDX_HOST}"   _COREDX_HOST)

if(NOT "${_COREDX_TOP} " STREQUAL " ")
  # look inside of COREDX_HOME if defined
  message(STATUS "Found CoreDX DDS: ${_COREDX_TOP}")
  set(CoreDX_HOME "${_COREDX_TOP}")
  set(CoreDX_INCLUDE_DIRS "${_COREDX_TOP}/target/include")

  set(_lib_path "${_COREDX_TOP}/target/${_COREDX_TARGET}/lib")

  set(_search_library_paths "")
  foreach(_library_name ${_expected_library_names})
    list(APPEND _search_library_paths "${_lib_path}/${_library_name}")
  endforeach()

  # find libraries
  file(GLOB_RECURSE _libs
    RELATIVE "${_lib_path}"
    ${_search_library_paths}
    )
  
  _find_coredx_libraries(_length _found_all_libraries "${_expected_library_names}" "${_libs}")
  if(NOT _found_all_libraries)
    message(FATAL_ERROR "COREDX_TOP set to '${_COREDX_TOP}' but could not find all libraries '${_expected_library_names}' under '${_lib_path}': ${_libs}")
  endif()

  list(LENGTH _expected_library_names _expected_length)
  if(_length GREATER _expected_length)
    message(FATAL_ERROR "COREDX_TOP set to '${_COREDX_TOP}' but found multiple files named '${_expected_library_names}' under '${_lib_path}': ${_libs}")
  endif()

  set(CoreDX_LIBRARIES "")
  foreach(_lib IN LISTS _libs)
    list(APPEND CoreDX_LIBRARIES "${_lib_path}/${_lib}")
  endforeach()
  list(GET _libs 0 _first_lib)
  get_filename_component(CoreDX_LIBRARY_DIRS "${_lib_path}/${_first_lib}" DIRECTORY)
  # Since we know CoreDX_LIBRARY_DIRS is a single path, just alias it.
  set(CoreDX_LIBRARY_DIR "${CoreDX_LIBRARY_DIRS}")

  set(CoreDX_DEFINITIONS "COREDX_DDS")

  if(WIN32)
    set(CoreDX_DDSGEN "${_COREDX_TOP}/host/${_COREDX_HOST}/bin/coredx_ddl.exe")
  else()
    set(CoreDX_DDSGEN "${_COREDX_TOP}/host/${_COREDX_HOST}/bin/coredx_ddl")
  endif()

  if(NOT EXISTS "${CoreDX_DDSGEN}")
    message(FATAL_ERROR "Could not find executable 'coredx_ddl' at ${CoreDX_DDSGEN}")
  endif()

  set(CoreDX_FOUND TRUE)
endif()

if(CoreDX_FOUND AND NOT WIN32)
  list(APPEND CoreDX_LIBRARIES "pthread")
endif()

include(FindPackageHandleStandardArgs)

# CoreDX_HOME, CoreDX_LIBRARY_DIRS, and CoreDX_LIBRARY_DIR are not always set, depending on the source of CoreDX.
find_package_handle_standard_args(CoreDX
  FOUND_VAR CoreDX_FOUND
  REQUIRED_VARS
    CoreDX_INCLUDE_DIRS
    CoreDX_LIBRARIES
    CoreDX_DEFINITIONS
    CoreDX_DDSGEN
)
