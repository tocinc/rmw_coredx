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

#include "take.h" 
 
/* ************************************************
 */
rmw_ret_t
rmw_take_with_info( const rmw_subscription_t * subscription,
                    void                     * ros_message,
                    bool                     * taken,
                    rmw_message_info_t       * message_info,
		    rmw_subscription_allocation_t * allocation )
{
  (void)allocation;
  if (!message_info) {
    RMW_SET_ERROR_MSG("message info is null");
    return RMW_RET_ERROR;
  }
  DDS::InstanceHandle_t sending_publication_handle;
  auto ret = _take(subscription, ros_message, taken, &sending_publication_handle);
  if (ret != RMW_RET_OK) {
    // Error string is already set.
    return RMW_RET_ERROR;
  }

  rmw_gid_t * sender_gid = &message_info->publisher_gid;
  sender_gid->implementation_identifier = toc_coredx_identifier;
  memset(sender_gid->data, 0, RMW_GID_STORAGE_SIZE);
  auto detail = reinterpret_cast<CoreDXPublisherGID *>(sender_gid->data);
  detail->publication_handle = sending_publication_handle;
  return RMW_RET_OK;
}

#if defined(__cplusplus)
}
#endif
