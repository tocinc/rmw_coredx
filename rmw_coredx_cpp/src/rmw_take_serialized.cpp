// Copyright 2018 Twin Oaks Computing, Inc.
// Modifications copyright (C) 2018 Twin Oaks Computing, Inc.
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

#include "take.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* ************************************************
 */
rmw_ret_t
rmw_take_serialized_message( const rmw_subscription_t * subscription,
			     rmw_serialized_message_t * serialized_message,
			     bool                     * taken )
{
  rmw_ret_t retval = RMW_RET_ERROR;
  // --------------------------------------------------------
  // stand-in until we support a take_serialized() API
  //   :: allocate 'ros' message
  CoreDXStaticSubscriberInfo * subscriber_info =
    static_cast<CoreDXStaticSubscriberInfo *>(subscription->data);
  if (!subscriber_info) {
    RMW_SET_ERROR_MSG("subscriber info handle is null");
    return RMW_RET_ERROR;
  }
  const message_type_support_callbacks_t * callbacks = subscriber_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }
  void * ros_msg = callbacks->alloc_ros_msg( (rcutils_allocator_t*)&serialized_message->allocator );
  //   :: take next message
  if ( ros_msg ) {
    retval = _take(subscription, ros_msg, taken, nullptr);
    //   :: serialize ros_msg to serialized_messsage
    if ( retval == RMW_RET_OK ) {
      callbacks->serialize( ros_msg, serialized_message );
    }
    //   :: clean up
    callbacks->free_ros_msg( ros_msg, (rcutils_allocator_t*)&serialized_message->allocator );
  }
  // --------------------------------------------------------
  return retval;
}

/* ************************************************
 */
rmw_ret_t
rmw_take_serialized_message_with_info( const rmw_subscription_t * subscription,
				       rmw_serialized_message_t * serialized_message,
				       bool                     * taken,
				       rmw_message_info_t       * message_info )
{
  rmw_ret_t retval = RMW_RET_ERROR;
  // --------------------------------------------------------
  // stand-in until we support a take_serialized() API
  CoreDXStaticSubscriberInfo * subscriber_info =
    static_cast<CoreDXStaticSubscriberInfo *>(subscription->data);
  if (!subscriber_info) {
    RMW_SET_ERROR_MSG("subscriber info handle is null");
    return RMW_RET_ERROR;
  }
  const message_type_support_callbacks_t * callbacks = subscriber_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }
  //   :: allocate 'ros' message
  void * ros_msg = callbacks->alloc_ros_msg( (rcutils_allocator_t*)&serialized_message->allocator );
  //   :: take next message
  if ( ros_msg ) {
    retval = rmw_take_with_info(subscription, ros_msg, taken, message_info);
    //   :: serialize ros_msg to serialized_messsage
    if ( retval == RMW_RET_OK ) {
      callbacks->serialize( ros_msg, serialized_message );
    }
    //   :: clean up
    callbacks->free_ros_msg( ros_msg, (rcutils_allocator_t*)&serialized_message->allocator );
  }
  // --------------------------------------------------------
  return retval;
}

#if defined(__cplusplus)
}
#endif
