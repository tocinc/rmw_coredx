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
rmw_destroy_client( rmw_node_t * node, rmw_client_t * client )
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  if (!client) {
    RMW_SET_ERROR_MSG("client handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client handle,
    client->implementation_identifier, toc_coredx_identifier,
    return RMW_RET_ERROR)

  auto result = RMW_RET_OK;
  CoreDXStaticClientInfo * client_info = static_cast<CoreDXStaticClientInfo *>(client->data);
  if (client_info)
    {
      auto response_datareader = client_info->response_datareader_;
      
      if ( response_datareader && client_info->read_condition_ ) {
        if (response_datareader->delete_readcondition(client_info->read_condition_) != DDS_RETCODE_OK) {
          RMW_SET_ERROR_MSG("failed to delete readcondition");
          result = RMW_RET_ERROR;
        }
      } 
      const service_type_support_callbacks_t * callbacks = client_info->callbacks_;
      if (callbacks && client_info->requester_) {
        callbacks->destroy_requester(client_info->requester_, &rmw_free);
      }
      
      RMW_TRY_DESTRUCTOR(
                         client_info->~CoreDXStaticClientInfo(),
                         CoreDXStaticClientInfo,
                         result = RMW_RET_ERROR)
      rmw_free(client_info);
      client->data = nullptr;
      if (client->service_name) {
        rmw_free(const_cast<char *>(client->service_name));
      }
    }
  rmw_client_free(client);
  return result;
}


#if defined(__cplusplus)
}
#endif

