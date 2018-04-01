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
rmw_wait_set_t *
rmw_create_wait_set(size_t max_conditions)
{
  rmw_wait_set_t    * waitset = rmw_wait_set_allocate();
  CoreDXWaitSetInfo * waitset_info = nullptr;
  
  RCUTILS_LOG_DEBUG_NAMED(
    "rmw_coredx_cpp",
    "%s[ begin ]",
    __FUNCTION__ );

  // From here onward, error results in unrolling in the goto fail block.
  if (!waitset) {
    RMW_SET_ERROR_MSG("failed to allocate waitset");
    goto fail;
  }
  waitset->implementation_identifier = toc_coredx_identifier;
  waitset->data = rmw_allocate(sizeof(CoreDXWaitSetInfo));
  waitset_info = static_cast<CoreDXWaitSetInfo *>(waitset->data);

  if (!waitset_info) {
    RMW_SET_ERROR_MSG("failed to allocate waitset");
    goto fail;
  }

  waitset_info->wait_set = static_cast<DDS::WaitSet *>(rmw_allocate(sizeof(DDS::WaitSet)));
  if (!waitset_info->wait_set) {
    RMW_SET_ERROR_MSG("failed to allocate waitset");
    goto fail;
  }

  RMW_TRY_PLACEMENT_NEW( waitset_info->wait_set, waitset_info->wait_set, goto fail, DDS::WaitSet, )

  // If max_conditions is greater than zero, re-allocate both ConditionSeqs to max_conditions
  if (max_conditions > 0) {
    waitset_info->active_conditions.resize(max_conditions);
    waitset_info->attached_conditions.resize(max_conditions);
  } 

  RCUTILS_LOG_DEBUG_NAMED(
    "rmw_coredx_cpp",
    "%s[ ret: %p ]",
    __FUNCTION__, waitset );
  
  return waitset;

fail:
  if (waitset_info) {
    if (waitset_info->wait_set) {
      RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(waitset_info->wait_set->~WaitSet(), DDS::WaitSet)
      rmw_free(waitset_info->wait_set);
    }
    waitset_info = nullptr;
  }
  if (waitset) {
    if (waitset->data) {
      rmw_free(waitset->data);
    }
    rmw_wait_set_free(waitset);
  }
  RCUTILS_LOG_DEBUG_NAMED(
    "rmw_coredx_cpp",
    "%s[ FAILED ]",
    __FUNCTION__ );
  return nullptr;
}
  
#if defined(__cplusplus)
}
#endif
