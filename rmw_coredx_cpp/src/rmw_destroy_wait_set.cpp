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
rmw_destroy_wait_set(rmw_wait_set_t * waitset)
{
  RCUTILS_LOG_DEBUG_NAMED(
    "rmw_coredx_cpp",
    "%s[ waitset: %p ]",
    __FUNCTION__,
    waitset );
  if (!waitset) {
    RMW_SET_ERROR_MSG("waitset handle is null");
    return RMW_RET_ERROR;
  }

  auto result = RMW_RET_OK;
  CoreDXWaitSetInfo * waitset_info = static_cast<CoreDXWaitSetInfo *>(waitset->data);

  // Explicitly call destructor since the "placement new" was used
  waitset_info->active_conditions.clear();
  waitset_info->attached_conditions.clear();
  if (waitset_info->wait_set) {
    RMW_TRY_DESTRUCTOR(waitset_info->wait_set->~WaitSet(), WaitSet, result = RMW_RET_ERROR)
    rmw_free(waitset_info->wait_set);
  }
  waitset_info = nullptr;
  if (waitset->data) {
    rmw_free(waitset->data);
    waitset->data = nullptr;
  }
  if (waitset) {
    rmw_wait_set_free(waitset);
  }
  return result;
}


#if defined(__cplusplus)
}
#endif
