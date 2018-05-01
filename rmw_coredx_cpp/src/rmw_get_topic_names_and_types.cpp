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

static std::string
_demangle_if_ros_topic(const std::string & topic_name)
{
  std::string prefix = _get_ros_prefix_if_exists(topic_name);
  if ( prefix.length() ) {
    return topic_name.substr(strlen(ros_topic_prefix));
  }
  return topic_name;
}

static std::string
_demangle_if_ros_type(const std::string & dds_type_string)
{
  std::string substring = "::msg::dds_::";
  size_t substring_position = dds_type_string.find(substring);
  if (
    dds_type_string[dds_type_string.size() - 1] == '_' &&
    substring_position != std::string::npos )
  {
    std::string pkg = dds_type_string.substr( 0, substring_position );
    size_t start = substring_position + substring.size();
    std::string type_name = dds_type_string.substr( start, dds_type_string.length() - 1 - start );
    return pkg + "/" + type_name;
  }
  // not a ROS type
  return dds_type_string;
}
  
/* ************************************************
 */
rmw_ret_t
rmw_get_topic_names_and_types(
  const rmw_node_t * node,
  rcutils_allocator_t * allocator,
  bool no_demangle,
  rmw_names_and_types_t * topic_names_and_types)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  if (node->implementation_identifier != toc_coredx_identifier) {
    RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  if (rmw_names_and_types_check_zero(topic_names_and_types) != RMW_RET_OK)
  {
    RMW_SET_ERROR_MSG("topic names and types is not initialized");
    return RMW_RET_ERROR;
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
  std::map<std::string, std::set<std::string>> topics;
  for (auto it : node_info->publisher_listener->topic_names_and_types) {
    if ( !no_demangle && _get_ros_prefix_if_exists( it.first ) != ros_topic_prefix ) {
      continue; /* not a plain topic if not prefixed with "rt/" */
    }
    for (auto & jt : it.second) {
      topics[it.first].insert(jt);
    }
  }
  for (auto it : node_info->subscriber_listener->topic_names_and_types) {
    if ( !no_demangle && _get_ros_prefix_if_exists( it.first ) != ros_topic_prefix ) {
      continue; /* not a plain topic if not prefixed with "rt/" */
    }
    for (auto & jt : it.second) {
      topics[it.first].insert(jt);
    }
  }

  if ( topics.size() > 0 ) {
    // Setup string array to store names
    rmw_ret_t rmw_ret = rmw_names_and_types_init(topic_names_and_types, topics.size(), allocator);
    if (rmw_ret != RMW_RET_OK) {
      return rmw_ret;
    }
    // Setup cleanup function, in case of failure below
    auto fail_cleanup = [&topic_names_and_types]() {
        rmw_ret_t rmw_ret = rmw_names_and_types_fini(topic_names_and_types);
        if (rmw_ret != RMW_RET_OK) {
          RCUTILS_LOG_ERROR("error during report of error: %s", rmw_get_error_string_safe())
        }
      };
    // Setup demangling functions based on no_demangle option
    auto demangle_topic = _demangle_if_ros_topic;
    auto demangle_type = _demangle_if_ros_type;
    if (no_demangle) {
      auto noop = [](const std::string & in) {
          return in;
        };
      demangle_topic = noop;
      demangle_type = noop;
    }
    // For each topic, store the name, initialize the string array for types, and store all types
    size_t index = 0;
    for (const auto & topic_n_types : topics) {
      // Duplicate and store the topic_name
      char * topic_name = rcutils_strdup( demangle_topic(topic_n_types.first).c_str(), *allocator );
      if ( !topic_name ) {
        RMW_SET_ERROR_MSG_ALLOC("failed to allocate memory for topic name", *allocator);
        fail_cleanup();
        return RMW_RET_BAD_ALLOC;
      }
      topic_names_and_types->names.data[index] = topic_name;
      // Setup storage for types
      {
        rcutils_ret_t rcutils_ret = rcutils_string_array_init(
          &topic_names_and_types->types[index],
          topic_n_types.second.size(),
          allocator);
        if ( rcutils_ret != RCUTILS_RET_OK ) {
          RMW_SET_ERROR_MSG( rcutils_get_error_string_safe() )
          fail_cleanup();
          return rmw_convert_rcutils_ret_to_rmw_ret(rcutils_ret);
        }
      }
      // Duplicate and store each type for the topic
      size_t type_index = 0;
      for ( const auto & type : topic_n_types.second ) {
        char * type_name = rcutils_strdup( demangle_type(type).c_str(), *allocator );
        if (!type_name) {
          RMW_SET_ERROR_MSG_ALLOC("failed to allocate memory for type name", *allocator)
          fail_cleanup();
          return RMW_RET_BAD_ALLOC;
        }
        topic_names_and_types->types[index].data[type_index] = type_name;
        ++type_index;
      }  // for each type
      ++index;
    }  // for each topic
  }
  return RMW_RET_OK;
}


#if defined(__cplusplus)
}
#endif
