// Copyright 2015 Twin Oaks Computing, Inc.
// Modifications copyright (C) 2017-2019 Twin Oaks Computing, Inc.
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
rmw_publisher_assert_liveliness( const rmw_publisher_t * publisher )
{
  if (!publisher) {
    RMW_SET_ERROR_MSG("publisher handle is null");
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
  DDS::ReturnCode_t dds_retval = writer->assert_liveliness();
  if ( dds_retval != DDS_RETCODE_OK )
    return RMW_RET_ERROR; // too bad there's not another option...
    
  return RMW_RET_OK;
}

#if defined(__cplusplus)
}
#endif
