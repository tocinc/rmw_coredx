// Copyright 2015 Twin Oaks Computing, Inc.
// Modifications copyright (C) 2017 Twin Oaks Computing, Inc.
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

#include <rcutils/get_env.h>
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
static void
set_log_level()
{
  static int    done = 0;
  if ( !done )
    {
      const char * env = nullptr;
      const char * err_str = nullptr;
      err_str = rcutils_get_env("RMW_COREDX_LOG_LEVEL", &env);
      if ( !err_str && env && (strlen(env) > 0) )
        {
          rcutils_ret_t ret = RCUTILS_RET_INVALID_ARGUMENT;
          if (strcmp( env, "DEBUG" ) == 0 )
            ret = rcutils_logging_set_logger_level("rmw_coredx_cpp", RCUTILS_LOG_SEVERITY_DEBUG);
          else if (strcmp( env, "INFO" ) == 0 )
            ret = rcutils_logging_set_logger_level("rmw_coredx_cpp", RCUTILS_LOG_SEVERITY_INFO);
          else if (strcmp( env, "WARN" ) == 0 )
            ret = rcutils_logging_set_logger_level("rmw_coredx_cpp", RCUTILS_LOG_SEVERITY_WARN);
          else if (strcmp( env, "ERROR" ) == 0 )
            ret = rcutils_logging_set_logger_level("rmw_coredx_cpp", RCUTILS_LOG_SEVERITY_ERROR);
          else if (strcmp( env, "FATAL" ) == 0 )
            ret = rcutils_logging_set_logger_level("rmw_coredx_cpp", RCUTILS_LOG_SEVERITY_FATAL);
          if ( ret != RCUTILS_RET_OK)
            fprintf(stderr, "RMW_COREDX: Failed to set logging level to '%s'\n", env );
        }
    }
}
   
/* ************************************************
 */
rmw_node_t *
rmw_create_node (
  const char * name,
  const char * namespace_,
  size_t       domain_id,
  const rmw_node_security_options_t * security_options )
{
  (void)security_options; /* todo ... */

  DDS::DomainParticipantFactory * dpf_ = DDS::DomainParticipantFactory::get_instance();
  if (!dpf_) {
    RMW_SET_ERROR_MSG("failed to get participant factory");
    return NULL;
  }

  set_log_level();

  RCUTILS_LOG_DEBUG_NAMED(
    "rmw_coredx_cpp",
    "%s[ name: '%s' namespace: '%s' domain_id: %d ]",
    __FUNCTION__,
    name, namespace_, (int)domain_id);

  DDS::DomainParticipantQos dp_qos;
  dpf_->get_default_participant_qos(dp_qos);
  size_t name_len = strlen(name);
  if ( name_len > COREDX_ENTITY_NAME_MAX) name_len = COREDX_ENTITY_NAME_MAX;
  memcpy( dp_qos.entity_name.value, name, name_len );
  dp_qos.entity_name.value[COREDX_ENTITY_NAME_MAX-1] = '\0';
  DDS::DomainId_t           domain = static_cast<DDS::DomainId_t>(domain_id);
  DDS::DomainParticipant  * participant =
    dpf_->create_participant(domain,
                             dp_qos,
                             NULL,
                             0);
  if (!participant) {
    RMW_SET_ERROR_MSG("failed to create participant");
    return NULL;
  }

  rmw_node_t               * node_handle = nullptr;
  CoreDXNodeInfo           * node_info   = nullptr;
  rmw_guard_condition_t    * graph_guard_condition = nullptr;
  CustomPublisherListener  * publisher_listener = nullptr;
  CustomSubscriberListener * subscriber_listener = nullptr;
  void                     * buf         = nullptr;
  DDS::DataReader          * data_reader = nullptr;
  DDS::DCPSPublicationDataReader  * builtin_publication_datareader = nullptr;
  DDS::DCPSSubscriptionDataReader * builtin_subscription_datareader = nullptr;
  DDS::Subscriber                 * builtin_subscriber = participant->get_builtin_subscriber();
  if (!builtin_subscriber) {
    RMW_SET_ERROR_MSG("builtin subscriber handle is null");
    goto fail;
  }
  
  // graph guard condition 
  graph_guard_condition = rmw_create_guard_condition( );
  if (!graph_guard_condition) {
    RMW_SET_ERROR_MSG("failed to create graph guard condition");
    goto fail;
  }
  
  // setup publications listener
  data_reader = builtin_subscriber->lookup_datareader("DCPSPublication");
  builtin_publication_datareader =
    static_cast<DDS::DCPSPublicationDataReader *>(data_reader);
  if (!builtin_publication_datareader) {
    RMW_SET_ERROR_MSG("builtin publication datareader handle is null");
    goto fail;
  }

  buf = rmw_allocate(sizeof(CustomPublisherListener));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  RMW_TRY_PLACEMENT_NEW(publisher_listener, buf, goto fail, CustomPublisherListener,
                        toc_coredx_identifier, graph_guard_condition)
  buf = nullptr;
  builtin_publication_datareader->set_listener(publisher_listener, DDS::DATA_AVAILABLE_STATUS);

  // setup subscriber listener
  data_reader = builtin_subscriber->lookup_datareader("DCPSSubscription");
  builtin_subscription_datareader =
    static_cast<DDS::DCPSSubscriptionDataReader *>(data_reader);
  if (!builtin_subscription_datareader) {
    RMW_SET_ERROR_MSG("builtin subscription datareader handle is null");
    goto fail;
  }

  buf = rmw_allocate(sizeof(CustomSubscriberListener));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  RMW_TRY_PLACEMENT_NEW(subscriber_listener, buf, goto fail, CustomSubscriberListener,
                        toc_coredx_identifier, graph_guard_condition)
  buf = nullptr;
  builtin_subscription_datareader->set_listener(subscriber_listener, DDS::DATA_AVAILABLE_STATUS);

  // setup 'node handle'
  node_handle = rmw_node_allocate();
  if (!node_handle) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node handle");
    goto fail;
  }
  node_handle->implementation_identifier = toc_coredx_identifier;
  node_handle->data                      = participant;
  node_handle->name                      = do_strdup(name);
  node_handle->namespace_                = do_strdup(namespace_);
  if (!node_handle->name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node name");
    goto fail;
  }
  
  buf = rmw_allocate(sizeof(CoreDXNodeInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  RMW_TRY_PLACEMENT_NEW(node_info, buf, goto fail, CoreDXNodeInfo)
  buf = nullptr;
  node_info->participant         = participant;
  node_info->publisher_listener  = publisher_listener;
  node_info->subscriber_listener = subscriber_listener;
  node_info->graph_guard_condition = graph_guard_condition;
  
  node_handle->implementation_identifier = toc_coredx_identifier;
  node_handle->data = node_info;

  RCUTILS_LOG_DEBUG_NAMED(
    "rmw_coredx_cpp",
    "%s[ ret: %p ]",
    __FUNCTION__,
    node_handle );
  
  return node_handle;

fail:
  DDS::ReturnCode_t status = dpf_->delete_participant(participant);
  if (status != DDS::RETCODE_OK) {
    std::stringstream ss;
    ss << "leaking participant while handling failure at " <<
      __FILE__ << ":" << __LINE__;
    (std::cerr << ss.str()).flush();
  }
  if (graph_guard_condition) {
    rmw_ret_t ret = rmw_destroy_guard_condition(graph_guard_condition);
    if (ret != RMW_RET_OK) {
      std::stringstream ss;
      ss << "failed to destroy guard condition while handling failure at " <<
        __FILE__ << ":" << __LINE__;
      (std::cerr << ss.str()).flush();
    }
  }
  if (publisher_listener) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      publisher_listener->~CustomPublisherListener(), CustomPublisherListener)
    rmw_free(publisher_listener);
  }
  if (subscriber_listener) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      subscriber_listener->~CustomSubscriberListener(), CustomSubscriberListener)
    rmw_free(subscriber_listener);
  }
  if (node_handle) {
    if (node_handle->name) {
      rmw_free(const_cast<char *>(node_handle->name));
    }
    if (node_handle->namespace_) {
      rmw_free(const_cast<char *>(node_handle->namespace_));
    }
    rmw_free(node_handle);
  }
  if (node_info) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      node_info->~CoreDXNodeInfo(), CoreDXNodeInfo)
    rmw_free(node_info);
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
