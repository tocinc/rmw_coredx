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
rmw_client_t *
rmw_create_client( const rmw_node_t                   * node,
                  const rosidl_service_type_support_t * type_supports,
                  const char                          * service_name,
                  const rmw_qos_profile_t             * qos_profile )
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return NULL;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, toc_coredx_identifier,
    return NULL)

  const rosidl_service_type_support_t * type_support; 
  RMW_COREDX_EXTRACT_SERVICE_TYPESUPPORT(type_supports, type_support);

  if (!qos_profile) {
    RMW_SET_ERROR_MSG("qos_profile is null");
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

  const service_type_support_callbacks_t * callbacks =
    static_cast<const service_type_support_callbacks_t *>(type_support->data);
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return NULL;
  }
  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_client_t * client = nullptr;
  DDS::DataReaderQos datareader_qos;
  DDS::DataWriterQos datawriter_qos;
  DDS::DataReader * response_datareader = nullptr;
  DDS::ReadCondition * read_condition = nullptr;
  void * requester = nullptr;
  void * buf = nullptr;
  CoreDXStaticClientInfo * client_info = nullptr;

  // Begin inializing elements.
  client = rmw_client_allocate();
  if (!client) {
    RMW_SET_ERROR_MSG("failed to allocate client");
    goto fail;
  }

  if (!get_datareader_qos(participant, qos_profile, datareader_qos)) {
    // error string was set within the function
    goto fail;
  }

  if (!get_datawriter_qos(participant, qos_profile, datawriter_qos)) {
    // error string was set within the function
    goto fail;
  }

  requester = callbacks->create_requester(
    participant, service_name, &datareader_qos, &datawriter_qos,
    reinterpret_cast<void **>(&response_datareader), &rmw_allocate);
  if (!requester) {
    RMW_SET_ERROR_MSG("failed to create requester");
    goto fail;
  }
  if (!response_datareader) {
    RMW_SET_ERROR_MSG("data reader handle is null");
    goto fail;
  }

  read_condition = response_datareader->create_readcondition(
     DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
  if (!read_condition) {
    RMW_SET_ERROR_MSG("failed to create read condition");
    goto fail;
  }

  buf = rmw_allocate(sizeof(CoreDXStaticClientInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the CoreDXStaticClientInfo in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(client_info, buf, goto fail, CoreDXStaticClientInfo)
  buf = nullptr;  // Only free the client_info pointer; don't need the buf pointer anymore.
  client_info->requester_ = requester;
  client_info->callbacks_ = callbacks;
  client_info->response_datareader_ = response_datareader;
  client_info->read_condition_ = read_condition;

  client->implementation_identifier = toc_coredx_identifier;
  client->data = client_info;
  client->service_name = do_strdup( service_name );
  if (!client->service_name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node name");
    goto fail;
  }
  
  return client;
fail:
  if (client_info) {
    if (response_datareader && client_info->read_condition_ ) {
      response_datareader->delete_readcondition(client_info->read_condition_);
    }
    
    // deallocate requester (currently allocated with new elsewhere)
    if (callbacks && client_info->requester_) {
      callbacks->destroy_requester(client_info->requester_, &rmw_free);
    }
    
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
                                           client_info->~CoreDXStaticClientInfo(), CoreDXStaticClientInfo)
      rmw_free(client_info);
  }
  
  if (client) {
    if (client->service_name) {
      rmw_free(const_cast<char *>(client->service_name));
    }
    rmw_client_free(client);
  }
  
  if (buf) {
    rmw_free(buf);
  }
  return NULL;
}


#if defined(__cplusplus)
}
#endif
  
