// Copyright 2015 Twin Oaks Computing, Inc.
// Modifications copyright (C) 2017-2018 Twin Oaks Computing, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifdef CoreDX_GLIBCXX_USE_CXX11_ABI_ZERO
#define _GLIBCXX_USE_CXX11_ABI 0
#endif

#include <rmw/rmw.h>
#include <rmw/types.h>
#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/impl/cpp/macros.hpp>
#include <rmw/get_topic_names_and_types.h>
#include <rmw/get_service_names_and_types.h>

#include <rcutils/logging_macros.h>
#include <rcutils/format_string.h>
#include <rcutils/types.h>
#include <rcutils/split.h>

#include <dds/dds.hh>
#include <dds/dds_builtinDataReader.hh>

#include "rmw_coredx_cpp/identifier.hpp"
#include "rmw_coredx_types.hpp"
#include "util.hpp"
#include "names.hpp"

#include "rosidl_typesupport_coredx_c/identifier.h"
#include "rosidl_typesupport_coredx_cpp/identifier.hpp"

/* *******************************************************
 */
// shouldn't these be promoted to rwm 'generic' level?
const char * const ros_topic_prefix = "rt";
const char * const ros_service_requester_prefix = "rq";
const char * const ros_service_response_prefix = "rr";

std::vector<std::string> _ros_prefixes =
{ros_topic_prefix, ros_service_requester_prefix, ros_service_response_prefix};

std::string
_get_ros_prefix_if_exists(const std::string & topic_name)
{
  for (auto prefix : _ros_prefixes) {
    if (topic_name.rfind(std::string(prefix) + "/", 0) == 0) {
      return prefix;
    }
  }
  return "";
}

/* *******************************************************
 */
bool
rmw_coredx_process_service_name(
  const char * service_name,
  bool avoid_ros_namespace_conventions,
  char ** request_topic_str,
  char ** response_topic_str)
{
  bool success = true;
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  const char * request_prefix = "";
  const char * response_prefix = "";
  char * request_concat_str = nullptr;
  char * response_concat_str = nullptr;

  RCUTILS_LOG_DEBUG_NAMED(
                          "rmw_coredx_cpp",
                          "%s[ service_name: '%s' avoid_ros_namespace_conv: %s ]",
                          __FUNCTION__,
                          service_name,
                          avoid_ros_namespace_conventions?"YES":"NO" );
  
  if (!avoid_ros_namespace_conventions) {
    request_prefix  = ros_service_requester_prefix;
    response_prefix = ros_service_response_prefix;
  }
  
  request_concat_str = rcutils_format_string(
    allocator,
    "%s%s%s", request_prefix, service_name, "Request");
  if (!request_concat_str) {
    RMW_SET_ERROR_MSG("could not allocate memory for request topic string");
    success = false;
    goto end;
  }
  
  response_concat_str = rcutils_format_string(
    allocator,
    "%s%s%s", response_prefix, service_name, "Reply");
  if (!response_concat_str) {
    RMW_SET_ERROR_MSG("could not allocate memory for response topic string");
    success = false;
    goto end;
  }
  
  *request_topic_str  = do_strdup(request_concat_str);
  *response_topic_str = do_strdup(response_concat_str);

  RCUTILS_LOG_DEBUG_NAMED(
                          "rmw_coredx_cpp",
                          "%s[ '%s' -> req: '%s' | rep: '%s' ]",
                          __FUNCTION__,
                          service_name, 
                          *request_topic_str,
                          *response_topic_str );
end:
  // free that memory
  if (request_concat_str) {
    allocator.deallocate(request_concat_str, allocator.state);
  }
  if (response_concat_str) {
    allocator.deallocate(response_concat_str, allocator.state);
  }
  return success;
}
