// Copyright 2019 Twin Oaks Computing, Inc.
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

#include <rmw/error_handling.h>
#include <rmw/rmw.h>
#include <rmw/allocators.h>
#include <rcutils/logging_macros.h>

#include "rmw_coredx_types.hpp"
#include "util.hpp"

extern "C"
{
  rmw_ret_t
  rmw_get_serialized_message_size( const rosidl_message_type_support_t * type_support,
				   const rosidl_message_bounds_t * message_bounds, /* what is this? */
				   size_t * size )
  {
    RMW_SET_ERROR_MSG("notyet");
    return RMW_RET_ERROR; // unsupported
  }


}
