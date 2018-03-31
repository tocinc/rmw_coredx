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

#include <rcutils/strdup.h>

#include <dds/dds.hh>
#include <dds/dds_builtinDataReader.hh>

#include "rmw_coredx_cpp/identifier.hpp"
#include "rmw_coredx_types.hpp"
#include "util.hpp"

#if defined(__cplusplus)
extern "C" {
#endif

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
  std::map<std::string, std::set<std::string>> topics_with_multiple_types;
  for (auto it : node_info->publisher_listener->topic_names_and_types) {
    for (auto & jt : it.second) {
      topics_with_multiple_types[it.first].insert(jt);
    }
  }
  for (auto it : node_info->subscriber_listener->topic_names_and_types) {
    for (auto & jt : it.second) {
      topics_with_multiple_types[it.first].insert(jt);
    }
  }

  // ignore inconsistent types
  std::map<std::string, std::string> topics;
  for (auto & it : topics_with_multiple_types) {
    if (it.second.size() != 1) {
      fprintf(stderr, "topic type mismatch - ignoring topic '%s'\n", it.first.c_str());
      continue;
    }
    topics[it.first] = *it.second.begin();
  }

  // reformat type name
  std::string substr = "::msg::dds_::";
  for (auto & it : topics) {
    size_t substr_pos = it.second.find(substr);
    if (it.second[it.second.size() - 1] == '_' && substr_pos != std::string::npos) {
      it.second = it.second.substr(0, substr_pos) + "/" + it.second.substr(
        substr_pos + substr.size(), it.second.size() - substr_pos - substr.size() - 1);
    }
  }

  // copy data into result handle
  if (topics.size() > 0) {
    rmw_ret_t ret = rmw_names_and_types_init(topic_names_and_types, topics.size(), allocator);
    if (ret != RMW_RET_OK) {
      RMW_SET_ERROR_MSG("failed to allocate memory for topic names and types")
      return RMW_RET_ERROR;
    }
    size_t i = 0;
    for (auto it : topics) {
      topic_names_and_types->names.data[i] = rcutils_strdup(it.first.c_str(), *allocator);
      if (!topic_names_and_types->names.data[i]) {
        RMW_SET_ERROR_MSG("failed to allocate memory for topic name")
          goto fail;
      }
      rcutils_ret_t rcutils_ret = rcutils_string_array_init(
                                                            &topic_names_and_types->types[i],
                                                            1,
                                                            allocator);

      if (rcutils_ret != RCUTILS_RET_OK) {
        RMW_SET_ERROR_MSG(rcutils_get_error_string_safe())
          goto fail;
      }

      topic_names_and_types->types[i].data[0] = rcutils_strdup(it.second.c_str(), *allocator);
      
      if (!topic_names_and_types->types[i].data[0]) {
        rmw_free(topic_names_and_types->names.data[i]);
        RMW_SET_ERROR_MSG("failed to allocate memory for type name")
        goto fail;
      }
      ++i;
    }
  }

  return RMW_RET_OK;
fail:
  rmw_ret_t ret = rmw_names_and_types_fini(topic_names_and_types);
  (void)ret;
  return RMW_RET_ERROR;
}


#if defined(__cplusplus)
}
#endif
