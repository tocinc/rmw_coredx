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
rmw_publish_serialized_message( const rmw_publisher_t * publisher,
				const rmw_serialized_message_t * serialized_message,
				rmw_publisher_allocation_t * allocation )
{
  if ( !serialized_message ) {
    RMW_SET_ERROR_MSG("serialized message handle is null");
    return RMW_RET_ERROR;
  }

  // --------------------------------------------------------
  // stand-in until we support a write_serialized() API
  CoreDXStaticPublisherInfo * publisher_info =
    static_cast<CoreDXStaticPublisherInfo *>(publisher->data);
  if (!publisher_info) {
    RMW_SET_ERROR_MSG("publisher info handle is null");
    return RMW_RET_ERROR;
  }
  message_type_support_callbacks_t * callbacks = (message_type_support_callbacks_t *)publisher_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }
  //   :: allocate 'ros' message
  void * ros_msg = callbacks->alloc_ros_msg( (rcutils_allocator_t*)&serialized_message->allocator );
  //   :: deserialize into 'ros' message
  callbacks->deserialize( ros_msg, serialized_message );
  //   :: publish ros message
  rmw_ret_t retval =  rmw_publish( publisher, ros_msg, allocation );
  //   :: clean up
  callbacks->free_ros_msg( ros_msg, (rcutils_allocator_t*)&serialized_message->allocator );
  // --------------------------------------------------------

  return retval;
}

#if defined(__cplusplus)
}
#endif
