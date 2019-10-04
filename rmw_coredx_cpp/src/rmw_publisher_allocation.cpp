// Copyright 2019 Twin Oaks Computing, Inc.
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

#include <rmw/rmw.h>
#include "rmw/error_handling.h"


#if defined(__cplusplus)
extern "C" {
#endif

  /* ************************************************
   */
  rmw_ret_t
  rmw_init_publisher_allocation(  const rosidl_message_type_support_t * type_support,
				  const rosidl_message_bounds_t * message_bounds,
				  rmw_publisher_allocation_t * allocation )
  {
    (void)type_support;
    (void)message_bounds;
    (void)allocation;
    RMW_SET_ERROR_MSG("notyet");
    return RMW_RET_ERROR;
  }

  rmw_ret_t
  rmw_fini_publisher_allocation( rmw_publisher_allocation_t * allocation )
  {
    (void)allocation;
    RMW_SET_ERROR_MSG("notyet");
    return RMW_RET_ERROR;
  }

#if defined(__cplusplus)
}  // extern "C"
#endif
