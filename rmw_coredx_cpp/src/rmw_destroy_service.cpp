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

/* ************************************************
 */
rmw_ret_t
rmw_destroy_service( rmw_node_t    * node,
                     rmw_service_t * service )
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  if (!service) {
    RMW_SET_ERROR_MSG("service handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    service handle,
    service->implementation_identifier, toc_coredx_identifier,
    return RMW_RET_ERROR)

  auto result = RMW_RET_OK;
  CoreDXStaticServiceInfo * service_info = static_cast<CoreDXStaticServiceInfo *>(service->data);
  if (service_info)
    {
      auto request_datareader = service_info->request_datareader_;

      if (request_datareader && service_info->read_condition_) {
        if (request_datareader->delete_readcondition(service_info->read_condition_) != DDS_RETCODE_OK) {
          RMW_SET_ERROR_MSG("failed to delete readcondition");
          result = RMW_RET_ERROR;
        }
        service_info->read_condition_ = nullptr;
      } 
      const service_type_support_callbacks_t * callbacks = service_info->callbacks_;
      if ( callbacks && service_info->replier_ ) {
        callbacks->destroy_replier(service_info->replier_, &rmw_free);
      }
      
      RMW_TRY_DESTRUCTOR(
                         service_info->~CoreDXStaticServiceInfo(),
                         CoreDXStaticServiceInfo,
                         result = RMW_RET_ERROR)
      rmw_free(service_info);
      service->data = nullptr;
      if (service->service_name)
        rmw_free(const_cast<char*>(service->service_name));
    }
  rmw_service_free(service);
  return result;
}


#if defined(__cplusplus)
}
#endif

