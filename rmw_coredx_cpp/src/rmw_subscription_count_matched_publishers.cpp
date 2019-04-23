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


rmw_ret_t
rmw_subscription_count_matched_publishers( const rmw_subscription_t * subscription,
					   size_t * publisher_count )
{
  if (!subscription) {
    RMW_SET_ERROR_MSG("subscription handle is null");
    return RMW_RET_ERROR;
  }
  if (!publisher_count) {
    RMW_SET_ERROR_MSG("publisher_count out parameter is null");
    return RMW_RET_ERROR;
  }
  CoreDXStaticSubscriberInfo * subscriber_info =
    static_cast<CoreDXStaticSubscriberInfo *>(subscription->data);
  if (!subscriber_info) {
    RMW_SET_ERROR_MSG("subscription info handle is null");
    return RMW_RET_ERROR;
  }
  DDS::DataReader * reader = subscriber_info->topic_reader_;
  if (!reader) {
    RMW_SET_ERROR_MSG("topic reader handle is null");
    return RMW_RET_ERROR;
  }
  
  DDS::ReturnCode_t ret;
  DDS::SubscriptionMatchedStatus s;
  ret = reader->get_subscription_matched_status(&s);
  if (ret != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get subscription matched status");
    return RMW_RET_ERROR;
  }
  *publisher_count = s.current_count;
  return RMW_RET_OK;
}

#if defined(__cplusplus)
}
#endif
