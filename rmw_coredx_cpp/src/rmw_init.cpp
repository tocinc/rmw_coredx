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
#include <rmw/error_handling.h>

#include <dds/dds.hh>
#include <dds/dds_builtinDataReader.hh>

#if defined(__cplusplus)
extern "C" {
#endif

/* ************************************************
 */
rmw_ret_t
rmw_init ()
{
  rmw_ret_t retval;
  DDS::DomainParticipantFactory * dpf_ = DDS::DomainParticipantFactory::get_instance();
  if (dpf_)
    retval = RMW_RET_OK;
  else {
    RMW_SET_ERROR_MSG("failed to get participant factory");
    retval = RMW_RET_ERROR;
  }
  return retval;
}

#if defined(__cplusplus)
}  // extern "C"
#endif
