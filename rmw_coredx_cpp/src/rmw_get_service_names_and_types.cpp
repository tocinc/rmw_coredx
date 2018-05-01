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
#include <algorithm>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include <rmw/rmw.h>
#include <rmw/types.h>
#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/impl/cpp/macros.hpp>
#include <rmw/get_service_names_and_types.h>
#include <rmw/convert_rcutils_ret_to_rmw_ret.h>

#include <rcutils/logging_macros.h>
#include <rcutils/strdup.h>

#include <dds/dds.hh>
#include <dds/dds_builtinDataReader.hh>

#include "rmw_coredx_cpp/identifier.hpp"
#include "rmw_coredx_types.hpp"
#include "util.hpp"
#include "names.hpp"

#if defined(__cplusplus)
extern "C" {
#endif

/* ************************************************
 */
static std::string
_demangle_service_from_topic( const std::string & topic_name )
{
  std::string prefix = _get_ros_prefix_if_exists( topic_name );
  if ( !prefix.length() ) {
    // not a ROS topic or service
    return "";
  }
  std::vector<std::string> prefixes = {
    ros_service_response_prefix,
    ros_service_requester_prefix,
  };
  if (
    std::none_of(
      prefixes.cbegin(), prefixes.cend(),
      [&prefix](std::string x) {
        return prefix == x;
      }))
  {
    // not a ROS service topic
    return "";
  }
  std::map<std::string, std::string> suffixes = {
    {ros_service_response_prefix, "Reply"},
    {ros_service_requester_prefix, "Request"},
  };
  auto suffix = suffixes[prefix];
  size_t suffix_position = topic_name.rfind(suffix);
  if (suffix_position == std::string::npos) {
    RCUTILS_LOG_WARN_NAMED("rmw_coredx_cpp",
      "service topic has prefix but no suffix"
      ", report this: '%s'", topic_name.c_str())
    return "";
  }
  // strip off the suffix first
  std::string service_name = topic_name.substr(0, suffix_position);
  // then the prefix
  size_t start = prefix.length();  // explicitly leave / after prefix
  return service_name.substr(start, service_name.length() - 1 - start);
}

/* ************************************************
 */
static std::string
_demangle_service_type_only(const std::string & dds_type_name)
{
  std::string ns_substring = "::srv::dds_::";
  size_t ns_substring_position = dds_type_name.find(ns_substring);
  if (ns_substring_position == std::string::npos) {
    /* in some cases, we don't register the type with a fully-qualified (scoped) type name */
    /* so, this namespace preifx may not be present: we don't require its presence */
    // return "";
  }
  auto suffixes = {
    std::string("_Response_"),
    std::string("_Request_"),
  };
  std::string found_suffix = "";
  size_t suffix_position = std::string::npos;
  for (auto suffix : suffixes) {
    suffix_position = dds_type_name.rfind(suffix);
    if (suffix_position != std::string::npos) {
      if (dds_type_name.length() - suffix_position - suffix.length() != 0) {
        RCUTILS_LOG_WARN_NAMED("rmw_coredx_cpp",
          "service type contains '::srv::dds_::' and a suffix, but not at the end"
          ", report this: '%s'", dds_type_name.c_str())
        continue;
      }
      found_suffix = suffix;
      break;
    }
  }
  if (suffix_position == std::string::npos) {
    RCUTILS_LOG_WARN_NAMED("rmw_coredx_cpp",
      "service type contains '::srv::dds_::' but does not have a suffix"
      ", report this: '%s'", dds_type_name.c_str())
    return "";
  }
  std::string retval;
  // everything checks out, reformat it from '<pkg>::srv::dds_::<type><suffix>' to '<pkg>/<type>'
  if ( ns_substring_position == std::string::npos ) {
    retval = dds_type_name.substr(0, suffix_position);
  } else {
    std::string pkg = dds_type_name.substr(0, ns_substring_position);
    size_t start = ns_substring_position + ns_substring.length();
    std::string type_name = dds_type_name.substr(start, suffix_position - start);
    retval = pkg + "/" + type_name;
  }
  return retval;
}
  
/* ************************************************
 */
rmw_ret_t
rmw_get_service_names_and_types(
                                const rmw_node_t * node,
                                rcutils_allocator_t * allocator,
                                rmw_names_and_types_t * service_names_and_types)
{
  if (!allocator) {
    RMW_SET_ERROR_MSG("allocator is null")
    return RMW_RET_INVALID_ARGUMENT;
  }
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_INVALID_ARGUMENT;
  }
  if (node->implementation_identifier != toc_coredx_identifier) {
    RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  rmw_ret_t ret = rmw_names_and_types_check_zero(service_names_and_types);
  if ( ret != RMW_RET_OK ){
    return ret;
  }

  auto node_info = static_cast<CoreDXNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }

  if (!node_info->publisher_listener) {
    RMW_SET_ERROR_MSG("publisher listener handle is null");
    return RMW_RET_ERROR;
  }
  if (!node_info->subscriber_listener) {
    RMW_SET_ERROR_MSG("subscriber listener handle is null");
    return RMW_RET_ERROR;
  }

  // combine publisher and subscriber information
  std::map<std::string, std::set<std::string>> services;
  {
    for (auto it : node_info->publisher_listener->topic_names_and_types) {
      std::string service_name = _demangle_service_from_topic(it.first);
      if (!service_name.length()) {
        // not a service
        continue;
      }
      for (auto & itt : it.second) {
        std::string service_type = _demangle_service_type_only(itt);
        if (service_type.length()) {
          services[service_name].insert(service_type);
        }
      }
    }
  }
  {
    for (auto it : node_info->subscriber_listener->topic_names_and_types) {
      std::string service_name = _demangle_service_from_topic(it.first);
      if (!service_name.length()) {
        // not a service
        continue;
      }
      for (auto & itt : it.second) {
        std::string service_type = _demangle_service_type_only(itt);
        if (service_type.length()) {
          services[service_name].insert(service_type);
        }
      }
    }
  }

  // Fill out service_names_and_types
  if (services.size()) {
    // Setup string array to store names
    rmw_ret_t rmw_ret =
      rmw_names_and_types_init(service_names_and_types, services.size(), allocator);
    if (rmw_ret != RMW_RET_OK) {
      return rmw_ret;
    }
    // Setup cleanup function, in case of failure below
    auto fail_cleanup = [&service_names_and_types]() {
        rmw_ret_t rmw_ret = rmw_names_and_types_fini(service_names_and_types);
        if (rmw_ret != RMW_RET_OK) {
          RCUTILS_LOG_ERROR("error during report of error: %s", rmw_get_error_string_safe())
        }
      };
    // For each service, store the name, initialize the string array for types, and store all types
    size_t index = 0;
    for (const auto & service_n_types : services) {
      // Duplicate and store the service_name

      char * service_name = rcutils_strdup(service_n_types.first.c_str(), *allocator);
      if (!service_name) {
        RMW_SET_ERROR_MSG_ALLOC("failed to allocate memory for service name", *allocator);
        fail_cleanup();
        return RMW_RET_BAD_ALLOC;
      }
      service_names_and_types->names.data[index] = service_name;
      // Setup storage for types
      {
        rcutils_ret_t rcutils_ret = rcutils_string_array_init(
          &service_names_and_types->types[index],
          service_n_types.second.size(),
          allocator);
        if (rcutils_ret != RCUTILS_RET_OK) {
          RMW_SET_ERROR_MSG(rcutils_get_error_string_safe())
          fail_cleanup();
          return rmw_convert_rcutils_ret_to_rmw_ret(rcutils_ret);
        }
      }
      // Duplicate and store each type for the service
      size_t type_index = 0;
      for (const auto & type : service_n_types.second) {
        char * type_name = rcutils_strdup(type.c_str(), *allocator);
        if (!type_name) {
          RMW_SET_ERROR_MSG_ALLOC("failed to allocate memory for type name", *allocator)
          fail_cleanup();
          return RMW_RET_BAD_ALLOC;
        }
        service_names_and_types->types[index].data[type_index] = type_name;
        ++type_index;
      }  // for each type
      ++index;
    }  // for each service
  }
  return RMW_RET_OK;
}

#if defined(__cplusplus)
}
#endif
