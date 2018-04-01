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

#include <rcutils/logging_macros.h>

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
rmw_destroy_node( rmw_node_t     * node )
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
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(node handle,
                                   node->implementation_identifier, toc_coredx_identifier,
                                   return RMW_RET_ERROR);

  DDS::DomainParticipantFactory * dpf_ = DDS::DomainParticipantFactory::get_instance();
  if (!dpf_) {
    RMW_SET_ERROR_MSG("failed to get participant factory");
    return RMW_RET_ERROR;
  }

  auto node_info = static_cast<CoreDXNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }
  auto participant = static_cast<DDS::DomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
  }
  // This unregisters types and destroys topics which were shared between
  // publishers and subscribers and could not be cleaned up in the delete functions.
  if (participant->delete_contained_entities() != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to delete contained entities of participant");
    return RMW_RET_ERROR;
  }

  DDS::ReturnCode_t ret = dpf_->delete_participant(participant);
  if (ret != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to delete participant");
    return RMW_RET_ERROR;
  }

  if (node_info->publisher_listener) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      node_info->publisher_listener->~CustomPublisherListener(), CustomPublisherListener)
    rmw_free(node_info->publisher_listener);
    node_info->publisher_listener = nullptr;
  }
  if (node_info->subscriber_listener) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      node_info->subscriber_listener->~CustomSubscriberListener(), CustomSubscriberListener)
    rmw_free(node_info->subscriber_listener);
    node_info->subscriber_listener = nullptr;
  }
  if (node_info->graph_guard_condition) {
    rmw_ret_t rmw_ret =
      rmw_destroy_guard_condition(node_info->graph_guard_condition);
    if (rmw_ret != RMW_RET_OK) {
      RMW_SET_ERROR_MSG("failed to delete graph guard condition");
      return RMW_RET_ERROR;
    }
    node_info->graph_guard_condition = nullptr;
  }

  rmw_free(node_info);
  node->data = nullptr;

  if (node->name) {
    rmw_free(const_cast<char *>(node->name));
    node->name = NULL;
  }
  if (node->namespace_) {
    rmw_free(const_cast<char *>(node->namespace_));
    node->namespace_ = NULL;
  }
  rmw_node_free(node);

  return RMW_RET_OK;
}

#if defined(__cplusplus)
}
#endif
