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

#include <vector>

#include <rmw/rmw.h>
#include <rmw/types.h>
#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/impl/cpp/macros.hpp>
#include <rmw/sanity_checks.h>
#include <rmw/convert_rcutils_ret_to_rmw_ret.h>
#include <rmw/impl/cpp/key_value.hpp>

#include <rcutils/logging_macros.h>
#include <rcutils/allocator.h>
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
rmw_get_node_names(
  const rmw_node_t       * node,
  rcutils_string_array_t * node_names,
  rcutils_string_array_t * node_namespaces)
{
  RCUTILS_LOG_DEBUG_NAMED(
    "rmw_coredx_cpp",
    "%s[ node: %p ]",
    __FUNCTION__,
    node );

  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  if (node->implementation_identifier != toc_coredx_identifier) {
    RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  if (rmw_check_zero_rmw_string_array(node_names) != RMW_RET_OK) {
    return RMW_RET_ERROR;
  }
  if (rmw_check_zero_rmw_string_array(node_namespaces) != RMW_RET_OK ) {
    return RMW_RET_ERROR;
  } 
    
  DDS::DomainParticipant * participant = static_cast<CoreDXNodeInfo *>(node->data)->participant;
  DDS::InstanceHandleSeq handles;
  if (participant->get_discovered_participants(&handles) != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("unable to fetch discovered participants.");
    return RMW_RET_ERROR;
  }
  uint32_t length = handles.length();

  RCUTILS_LOG_DEBUG_NAMED(
    "rmw_coredx_cpp",
    "%s[ length: %d ]",
    __FUNCTION__,
    length );
  
  node_names->size = length;
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_ret_t rcutils_ret = rcutils_string_array_init(node_names, length, &allocator);
  if (rcutils_ret != RCUTILS_RET_OK) {
    RMW_SET_ERROR_MSG(rcutils_get_error_string().str);
    return rmw_convert_rcutils_ret_to_rmw_ret(rcutils_ret);
  }
  rcutils_ret = rcutils_string_array_init(node_namespaces, length, &allocator);
  if (rcutils_ret != RCUTILS_RET_OK) {
    RMW_SET_ERROR_MSG(rcutils_get_error_string().str);
    return rmw_convert_rcutils_ret_to_rmw_ret(rcutils_ret);
  }

  for (uint32_t i = 0; i < length; ++i) {
    DDS::ParticipantBuiltinTopicData pbtd;
    auto dds_ret = participant->get_discovered_participant_data(&pbtd, handles[i]);
    std::string name;
    std::string name_space;
    if ( dds_ret == DDS::RETCODE_OK )
      {
	// first, try to parse user_data as a set of "key=value;" pairs
	auto data = static_cast<const unsigned char *>(pbtd.user_data.value.buffer());
	std::vector<uint8_t> kv(data, data + pbtd.user_data.value.length());
	auto map = rmw::impl::cpp::parse_key_value(kv);
	auto name_found = map.find("name");
	auto ns_found = map.find("namespace");

	if (name_found != map.end()) {
	  name = std::string(name_found->second.begin(), name_found->second.end());
	}
	if (name_found != map.end()) {
	  name_space = std::string(ns_found->second.begin(), ns_found->second.end());
	}
	
	// if that doesn't work, try pbtd.entity_name
	if (name.empty()) {
	  if (pbtd.entity_name) {
	    name = pbtd.entity_name;
	  }
	}
      }
    
    if (name.empty()) {
      // ignore discovered participants without a name
      node_names->data[i]      = nullptr;
      node_namespaces->data[i] = nullptr;
      continue;
    }
    
    node_names->data[i] = rcutils_strdup(name.c_str(), allocator);
    if (!node_names->data[i]) {
      RMW_SET_ERROR_MSG("could not allocate memory for node name");
      goto fail;
    }

    node_namespaces->data[i] = rcutils_strdup(name_space.c_str(), allocator);
    if (!node_namespaces->data[i]) {
      RMW_SET_ERROR_MSG("could not allocate memory for node namespace");
      goto fail;
    }
    
    RCUTILS_LOG_DEBUG_NAMED(
                            "rmw_coredx_cpp",
                            "%s[ name: '%s' namespace: '%s']",
                            __FUNCTION__,
                            name.c_str(),
			    name_space.c_str() );
  }

  return RMW_RET_OK;

fail:
  if (node_names) {
    rcutils_ret = rcutils_string_array_fini(node_names);
    if (rcutils_ret != RCUTILS_RET_OK) {
      RCUTILS_LOG_ERROR_NAMED(
        "rmw_coredx_cpp",
        "failed to cleanup during error handling: %s", rcutils_get_error_string().str);
      rcutils_reset_error();
    }
  }
  if (node_namespaces) {
    rcutils_ret = rcutils_string_array_fini(node_namespaces);
    if (rcutils_ret != RCUTILS_RET_OK) {
      RCUTILS_LOG_ERROR_NAMED(
        "rmw_coredx_cpp",
        "failed to cleanup during error handling: %s", rcutils_get_error_string().str);
      rcutils_reset_error();
    }
  }
  return RMW_RET_BAD_ALLOC;
  
}

#if defined(__cplusplus)
}
#endif

