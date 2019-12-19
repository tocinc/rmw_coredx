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

#include <dds/dds.hh>
#include <dds/dds_builtinDataReader.hh>

#include "rmw_coredx_cpp/identifier.hpp"
#include "rmw_coredx_types.hpp"
#include "util.hpp"
#include "names.hpp"

#include "rosidl_typesupport_coredx_c/identifier.h"
#include "rosidl_typesupport_coredx_cpp/identifier.hpp"

#if defined(__cplusplus)
extern "C" {
#endif

/* ************************************************
 */
rmw_publisher_t *
rmw_create_publisher( const rmw_node_t        * node,
                      const rosidl_message_type_support_t * type_supports,
                      const char              * topic_name,
                      const rmw_qos_profile_t * qos_policies )
{
  RCUTILS_LOG_DEBUG_NAMED(
    "rmw_coredx_cpp",
    "%s[ node: %p topic_name: '%s' ]",
    __FUNCTION__,
    node, topic_name );

  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return NULL;
  }
  
  if (node->implementation_identifier != toc_coredx_identifier) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
    return nullptr;
  }
  
  if (!topic_name || strlen(topic_name) == 0) {
    RMW_SET_ERROR_MSG("publisher topic is null or empty string");
    return nullptr;
  }
  
  if (!qos_policies) {
    RMW_SET_ERROR_MSG("qos_policies is null");
    return nullptr;
  }

  auto node_info = static_cast<CoreDXNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return NULL;
  }
  auto participant = static_cast<DDS::DomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return NULL;
  }

  const rosidl_message_type_support_t * type_support; 
  RMW_COREDX_EXTRACT_MESSAGE_TYPESUPPORT(type_supports, type_support);
  
  const message_type_support_callbacks_t * callbacks =
    static_cast<const message_type_support_callbacks_t *>(type_support->data);
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return NULL;
  }
  
  std::string type_name = _create_type_name(callbacks, "");
  std::string tmp_topic_name;
  if ( !qos_policies->avoid_ros_namespace_conventions ) {
    tmp_topic_name = std::string(ros_topic_prefix) + topic_name;
  } else {
    tmp_topic_name = std::string(topic_name);
  }
  
  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_publisher_t * publisher = nullptr;
  bool registered;
  DDS::PublisherQos publisher_qos;
  DDS::ReturnCode_t status;
  DDS::Publisher * dds_publisher = nullptr;
  DDS::Topic * topic;
  DDS::TopicDescription * topic_description = nullptr;
  DDS::DataWriterQos datawriter_qos;
  DDS::DataWriter * topic_writer = nullptr;
  void * buf = nullptr;
  CoreDXStaticPublisherInfo * publisher_info = nullptr;
  // Begin initializing elements
  publisher = rmw_publisher_allocate();
  if (!publisher) {
    RMW_SET_ERROR_MSG("failed to allocate publisher");
    goto fail;
  }

  registered = callbacks->register_type(participant, type_name.c_str());
  if (!registered) {
    RMW_SET_ERROR_MSG("failed to register type");
    goto fail;
  }

  status = participant->get_default_publisher_qos(publisher_qos);
  if (status != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get default publisher qos");
    goto fail;
  }
  
  dds_publisher = participant->create_publisher(publisher_qos, NULL, 0);
  if (!dds_publisher) {
    RMW_SET_ERROR_MSG("failed to create publisher");
    goto fail;
  }
  
  topic_description = participant->lookup_topicdescription( tmp_topic_name.c_str() );
  if (!topic_description) {
    DDS::TopicQos default_topic_qos;
    status = participant->get_default_topic_qos(default_topic_qos);
    if (status != DDS::RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to get default topic qos");
      goto fail;
    }

    topic = participant->create_topic(
      tmp_topic_name.c_str(), type_name.c_str(), default_topic_qos, NULL, 0);
    if (!topic) {
      RMW_SET_ERROR_MSG("failed to create topic");
      goto fail;
    }
  } else {
    DDS::Duration_t timeout;
    timeout.sec = 0;
    timeout.nanosec = 0;
    topic = participant->find_topic(tmp_topic_name.c_str(), timeout);
    if (!topic) {
      RMW_SET_ERROR_MSG("failed to find topic");
      goto fail;
    }
  }

  if (!get_datawriter_qos(participant, qos_policies, datawriter_qos)) {
    // error string was set within the function
    goto fail;
  }

  topic_writer = dds_publisher->create_datawriter(
    topic, datawriter_qos, NULL, 0);
  if (!topic_writer) {
    RMW_SET_ERROR_MSG("failed to create datawriter");
    goto fail;
  }

  // let discovery complete
  {
    DDS::Duration_t timeout(1,0);
    status = participant->builtin_wait_for_acknowledgments( timeout );
  }
  
  // Allocate memory for the CoreDXStaticPublisherInfo object.
  buf = rmw_allocate(sizeof(CoreDXStaticPublisherInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }

  // Use a placement new to construct the CoreDXStaticPublisherInfo in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(publisher_info, buf, goto fail, CoreDXStaticPublisherInfo)
  buf = nullptr;  // Only free the publisher_info pointer; don't need the buf pointer anymore.
  publisher_info->dds_publisher_ = dds_publisher;
  publisher_info->topic_writer_ = topic_writer;
  publisher_info->callbacks_ = callbacks;
  publisher_info->publisher_gid.implementation_identifier = toc_coredx_identifier;
  static_assert(
    sizeof(CoreDXPublisherGID) <= RMW_GID_STORAGE_SIZE,
    "RMW_GID_STORAGE_SIZE insufficient to store the rmw_coredx_cpp GID implemenation."
  );
  // Zero the data memory.
  memset(publisher_info->publisher_gid.data, 0, RMW_GID_STORAGE_SIZE);
  {
    auto publisher_gid =
      reinterpret_cast<CoreDXPublisherGID *>(publisher_info->publisher_gid.data);
    publisher_gid->publication_handle = topic_writer->get_instance_handle();
  }
  publisher_info->publisher_gid.implementation_identifier = toc_coredx_identifier;

  publisher->implementation_identifier = toc_coredx_identifier;
  publisher->data = publisher_info;
  publisher->topic_name = do_strdup(topic_name);
  if (!publisher->topic_name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node name");
    goto fail;
  }

  RCUTILS_LOG_DEBUG_NAMED(
    "rmw_coredx_cpp",
    "%s[ node: %p ret: %p ]",
    __FUNCTION__,
    node, publisher );
  
  return publisher;

 fail:
  
  if (publisher) {
    rmw_publisher_free(publisher);
  }
  // Assumption: participant is valid.
  if (dds_publisher) {
    if (topic_writer) {
      if (dds_publisher->delete_datawriter(topic_writer) != DDS::RETCODE_OK) {
        std::stringstream ss;
        ss << "leaking datawriter while handling failure at " <<
          __FILE__ << ":" << __LINE__ << '\n';
        (std::cerr << ss.str()).flush();
      }
    }
    if (participant->delete_publisher(dds_publisher) != DDS::RETCODE_OK) {
      std::stringstream ss;
      ss << "leaking publisher while handling failure at " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (publisher_info) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      publisher_info->~CoreDXStaticPublisherInfo(), CoreDXStaticPublisherInfo)
    rmw_free(publisher_info);
  }
  if (buf) {
    rmw_free(buf);
  }
  RCUTILS_LOG_DEBUG_NAMED(
    "rmw_coredx_cpp",
    "%s[ FAILED ]",
    __FUNCTION__ );
  return NULL;
}

#if defined(__cplusplus)
}
#endif
