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
static rmw_ret_t
_publisher_count_matched_subscriptions(DDS::DataWriter * datawriter, size_t * count)
{
  DDS::ReturnCode_t ret;
  DDS::PublicationMatchedStatus s;
  ret = datawriter->get_publication_matched_status(&s);
  if (ret != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get publication matched status");
    return RMW_RET_ERROR;
  }

#ifdef DISCOVERY_DEBUG_LOGGING
  using std::to_string;
  using std::stringstream;
  std::stringstream ss;
  // *INDENT-OFF* (prevent uncrustify from making unnecessary indents here)
  ss << "DDS_PublicationMatchedStatus:\n"
     << "  topic name:               " << datawriter->get_topic()->get_name() << "\n"
     << "  current_count:            " << to_string(s.current_count) << "\n"
     << "  current_count_change:     " << to_string(s.current_count_change) << "\n"
     << "  total_count:              " << to_string(s.total_count) << "\n"
     << "  total_count_change:       " << to_string(s.total_count_change) << "\n"
     << "  last_subscription_handle: " << to_string(s.last_subscription_handle) << "\n";
  // *INDENT-ON*
  printf("%s", ss.str().c_str());
#endif

  *count = s.current_count;
  return RMW_RET_OK;
}

/* ************************************************
 */
static rmw_ret_t
_subscription_count_matched_publishers(DDS::DataReader * datareader, size_t * count)
{
  DDS::ReturnCode_t ret;
  DDS::SubscriptionMatchedStatus s;
  ret = datareader->get_subscription_matched_status(&s);
  if (ret != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get subscription matched status");
    return RMW_RET_ERROR;
  }

#ifdef DISCOVERY_DEBUG_LOGGING
  using std::to_string;
  using std::stringstream;
  std::stringstream ss;
  // *INDENT-OFF* (prevent uncrustify from making unnecessary indents here)
  ss << "DDS_SubscriptionMatchedStatus:\n"
     << "  topic name:               " << datareader->get_topicdescription()->get_name() << "\n"
     << "  current_count:            " << to_string(s.current_count) << "\n"
     << "  current_count_change:     " << to_string(s.current_count_change) << "\n"
     << "  total_count:              " << to_string(s.total_count) << "\n"
     << "  total_count_change:       " << to_string(s.total_count_change) << "\n"
     << "  last_publication_handle:  " << to_string(s.last_publication_handle) << "\n";
  // *INDENT-ON*
  printf("%s", ss.str().c_str());
#endif

  *count = s.current_count;
  return RMW_RET_OK;
}


  
/* ************************************************
 */
rmw_ret_t
rmw_service_server_is_available(
  const rmw_node_t * node,
  const rmw_client_t * client,
  bool * is_available)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, toc_coredx_identifier,
    return RMW_RET_ERROR)
  if (!client) {
    RMW_SET_ERROR_MSG("client handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client handle,
    client->implementation_identifier, toc_coredx_identifier,
    return RMW_RET_ERROR)

  if (!is_available) {
    RMW_SET_ERROR_MSG("is_available is null");
    return RMW_RET_ERROR;
  }

  CoreDXStaticClientInfo * client_info =
    static_cast<CoreDXStaticClientInfo *>(client->data);
  if (!client_info) {
    RMW_SET_ERROR_MSG("client info handle is null");
    return RMW_RET_ERROR;
  }

  const service_type_support_callbacks_t * callbacks = client_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }
  void * requester = client_info->requester_;
  if (!requester) {
    RMW_SET_ERROR_MSG("requester handle is null");
    return RMW_RET_ERROR;
  }
  DDS::DataWriter * request_datawriter =
    static_cast<DDS::DataWriter *>(callbacks->get_request_datawriter(requester));
  const char * request_topic_name = request_datawriter->get_topic()->get_name();
  if (!request_topic_name) {
    RMW_SET_ERROR_MSG("could not get request topic name");
    return RMW_RET_ERROR;
  }

  *is_available = false;
  // In the CoreDX RPC implementation, a server is ready when:
  //   - At least one subscriber is matched to the request publisher.
  //   - At least one publisher is matched to the reponse subscription.
  size_t number_of_request_subscribers = 0;
  rmw_ret_t ret = _publisher_count_matched_subscriptions(
    request_datawriter, &number_of_request_subscribers);
  if (ret != RMW_RET_OK) {
    // error string already set
    return ret;
  }
#ifdef DISCOVERY_DEBUG_LOGGING
  printf("Checking for service server:\n");
  printf(" - %s: %zu\n",
    request_topic_name,
    number_of_request_subscribers);
#endif
  if (number_of_request_subscribers == 0) {
    // not ready
    return RMW_RET_OK;
  }

  size_t number_of_response_publishers = 0;
  ret = _subscription_count_matched_publishers(
    client_info->response_datareader_, &number_of_response_publishers);
  if (ret != RMW_RET_OK) {
    // error string already set
    return ret;
  }
#ifdef DISCOVERY_DEBUG_LOGGING
  printf(" - %s: %zu\n",
    client_info->response_datareader_->get_topicdescription()->get_name(),
    number_of_response_publishers);
#endif
  if (number_of_response_publishers == 0) {
    // not ready
    return RMW_RET_OK;
  }

  // all conditions met, there is a service server available
  *is_available = true;
  return RMW_RET_OK;
}

#if defined(__cplusplus)
}
#endif
