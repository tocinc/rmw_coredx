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
rmw_destroy_publisher( rmw_node_t      * node,
                       rmw_publisher_t * publisher)
{
  RCUTILS_LOG_DEBUG_NAMED(
    "rmw_coredx_cpp",
    "%s[ node: %p publisher: %p ]",
    __FUNCTION__,
    node, publisher );
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, toc_coredx_identifier,
    return RMW_RET_ERROR);

  if (!publisher) {
    RMW_SET_ERROR_MSG("publisher handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    publisher handle,
    publisher->implementation_identifier, toc_coredx_identifier,
    return RMW_RET_ERROR);

  auto node_info = static_cast<CoreDXNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }
  auto participant = static_cast<DDS::DomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return RMW_RET_ERROR;
  }
  // we don't need to unregister types with the participant.
  CoreDXStaticPublisherInfo * publisher_info = (CoreDXStaticPublisherInfo *)publisher->data;
  if (publisher_info) {
    DDS::Publisher * dds_publisher = publisher_info->dds_publisher_;
    if (dds_publisher) {
      if (publisher_info->topic_writer_) {
        if (dds_publisher->delete_datawriter(publisher_info->topic_writer_) != DDS::RETCODE_OK) {
          RMW_SET_ERROR_MSG("failed to delete datawriter");
          return RMW_RET_ERROR;
        }
        publisher_info->topic_writer_ = nullptr;
      }
      if (participant->delete_publisher(dds_publisher) != DDS::RETCODE_OK) {
        RMW_SET_ERROR_MSG("failed to delete publisher");
        return RMW_RET_ERROR;
      }
      publisher_info->dds_publisher_ = nullptr;
    } else if (publisher_info->topic_writer_) {
      RMW_SET_ERROR_MSG("cannot delete datawriter because the publisher is null");
      return RMW_RET_ERROR;
    }
    RMW_TRY_DESTRUCTOR(
      publisher_info->~CoreDXStaticPublisherInfo(),
      CoreDXStaticPublisherInfo, return RMW_RET_ERROR)
    rmw_free(publisher_info);
    publisher->data = nullptr;
  }
  if (publisher->topic_name) {
    rmw_free(const_cast<char *>(publisher->topic_name));
  }
  rmw_publisher_free(publisher);

  return RMW_RET_OK;
}

#if defined(__cplusplus)
}
#endif
