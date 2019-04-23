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
#include <rmw/error_handling.h>

#include <dds/dds.hh>

#include "rmw_coredx_types.hpp"

#if defined(__cplusplus)
extern "C" {
#endif

/* ************************************************
 * Retrieve the number of matched subscriptions to a publisher.
 * Query the underlying middleware to determine how many subscriptions are
 * matched to a given publisher.
 *
 * \param[in] publisher the publisher object to inspect
 * \param[out] subscription_count the number of subscriptions matched
 * \return `RMW_RET_OK` if successful, or
 * \return `RMW_RET_INVALID_ARGUMENT` if either argument is null, or
 * \return `RMW_RET_ERROR` if an unexpected error occurs.
 */
rmw_ret_t
rmw_publisher_count_matched_subscriptions( const rmw_publisher_t * publisher,
					   size_t * subscription_count )
{
  if (!publisher) {
    RMW_SET_ERROR_MSG("publisher handle is null");
    return RMW_RET_ERROR;
  }
  if (!subscription_count) {
    RMW_SET_ERROR_MSG("subscription_count out parameter is null");
    return RMW_RET_ERROR;
  }
  CoreDXStaticPublisherInfo * publisher_info =
    static_cast<CoreDXStaticPublisherInfo *>(publisher->data);
  if (!publisher_info) {
    RMW_SET_ERROR_MSG("publisher info handle is null");
    return RMW_RET_ERROR;
  }
  DDS::DataWriter * writer = publisher_info->topic_writer_;
  if (!writer) {
    RMW_SET_ERROR_MSG("writer handle is null");
    return RMW_RET_ERROR;
  }
  
  DDS::ReturnCode_t ret;
  DDS::PublicationMatchedStatus s;
  ret = writer->get_publication_matched_status(&s);
  if (ret != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get publication matched status");
    return RMW_RET_ERROR;
  }
  *subscription_count = s.current_count;
  return RMW_RET_OK;
}

#if defined(__cplusplus)
}
#endif
