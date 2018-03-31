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

template<typename SubscriberInfo, typename ServiceInfo, typename ClientInfo>
rmw_ret_t
wait(const char * implementation_identifier,
     rmw_subscriptions_t     * subscriptions,
     rmw_guard_conditions_t  * guard_conditions,
     rmw_services_t          * services,
     rmw_clients_t           * clients,
     rmw_wait_set_t          * ros_wait_set,
     const rmw_time_t              * wait_timeout)
{
  if (!ros_wait_set) {
    RMW_SET_ERROR_MSG("wait set handle is null");
    return RMW_RET_ERROR;
  }
  
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    ros_wait_set,
    ros_wait_set->implementation_identifier, implementation_identifier,
    return RMW_RET_ERROR);

  CoreDXWaitSetInfo * wait_set_info = static_cast<CoreDXWaitSetInfo *>(ros_wait_set->data);
  if (!wait_set_info) {
    RMW_SET_ERROR_MSG("Wait set implementation struct is null");
    return RMW_RET_ERROR;
  }

  DDS::WaitSet * dds_wait_set = static_cast<DDS::WaitSet *>(wait_set_info->wait_set);
  if (!dds_wait_set) {
    RMW_SET_ERROR_MSG("DDS wait set handle is null");
    return RMW_RET_ERROR;
  }

  DDS::ConditionSeq active_conditions =
    static_cast<DDS::ConditionSeq>(wait_set_info->active_conditions);
  
  // add a condition for each subscriber
  for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
    SubscriberInfo * subscriber_info =
      static_cast<SubscriberInfo *>(subscriptions->subscribers[i]);
    if (!subscriber_info) {
      RMW_SET_ERROR_MSG("subscriber info handle is null");
      return RMW_RET_ERROR;
    }
    DDS::ReadCondition * read_condition = subscriber_info->read_condition_;
    if (!read_condition) {
      RMW_SET_ERROR_MSG("read condition handle is null");
      return RMW_RET_ERROR;
    }

#if (DO_DEBUG)
    /* DEBUG -- */
    fprintf(stderr, "wait_set.attach( read_cond: %p topic: %s)\n", read_condition,
            subscriber_info->topic_reader_->get_topicdescription()->get_name());
#endif

    rmw_ret_t rmw_status = check_attach_condition_error(
      dds_wait_set->attach_condition(*read_condition));
    if (rmw_status != RMW_RET_OK) {
      return rmw_status;
    }
  }

  // add a condition for each guard condition
  for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i) {
    DDS::GuardCondition * guard_condition =
      static_cast<DDS::GuardCondition *>(guard_conditions->guard_conditions[i]);
    if (!guard_condition) {
      RMW_SET_ERROR_MSG("guard condition handle is null");
      return RMW_RET_ERROR;
    }
#if (DO_DEBUG)
    /* DEBUG -- */
    fprintf(stderr, "wait_set.attach( guard_condition: %p )\n", guard_condition);
#endif
    rmw_ret_t rmw_status = check_attach_condition_error(
      dds_wait_set->attach_condition(*guard_condition));
    if (rmw_status != RMW_RET_OK) {
      return rmw_status;
    }
  }

  // add a condition for each service
  for (size_t i = 0; i < services->service_count; ++i) {
    ServiceInfo * service_info =
      static_cast<ServiceInfo *>(services->services[i]);
    if (!service_info) {
      RMW_SET_ERROR_MSG("service info handle is null");
      return RMW_RET_ERROR;
    }
    DDS::DataReader * request_datareader = service_info->request_datareader_;
    if (!request_datareader) {
      RMW_SET_ERROR_MSG("request datareader handle is null");
      return RMW_RET_ERROR;
    }
    DDS::StatusCondition * condition = request_datareader->get_statuscondition();
    DDS::ReturnCode_t status = condition->set_enabled_statuses(DDS::DATA_AVAILABLE_STATUS);
    if (status != DDS::RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to set enabled statuses");
      return RMW_RET_ERROR;
    }
#if (DO_DEBUG)
    /* DEBUG -- */
    fprintf(stderr, "wait_set.attach( req status_cond: %p )\n", condition);
#endif
    rmw_ret_t rmw_status = check_attach_condition_error(
      dds_wait_set->attach_condition(condition));
    if (rmw_status != RMW_RET_OK) {
      return rmw_status;
    }
  }

  // add a condition for each client
  for (size_t i = 0; i < clients->client_count; ++i) {
    ClientInfo * client_info =
      static_cast<ClientInfo *>(clients->clients[i]);
    if (!client_info) {
      RMW_SET_ERROR_MSG("client info handle is null");
      return RMW_RET_ERROR;
    }
    DDS::DataReader * response_datareader = client_info->response_datareader_;
    if (!response_datareader) {
      RMW_SET_ERROR_MSG("response datareader handle is null");
      return RMW_RET_ERROR;
    }
    DDS::StatusCondition * condition = response_datareader->get_statuscondition();
    DDS::ReturnCode_t status = condition->set_enabled_statuses(DDS::DATA_AVAILABLE_STATUS);
    if (status != DDS::RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to set enabled statuses");
      return RMW_RET_ERROR;
    }
#if (DO_DEBUG)
    /* DEBUG -- */
    fprintf(stderr, "wait_set.attach( rep status_cond: %p )\n", condition);
#endif
    rmw_ret_t rmw_status = check_attach_condition_error(
      dds_wait_set->attach_condition(condition));
    if (rmw_status != RMW_RET_OK) {
      return rmw_status;
    }
  }

  // invoke wait until one of the conditions triggers
  DDS::Duration_t timeout;
  if (!wait_timeout) {
    timeout.sec     = DDS::DURATION_INFINITE_SEC;
    timeout.nanosec = DDS::DURATION_INFINITE_NSEC;
  } else {
    timeout.sec     = static_cast<int32_t>(wait_timeout->sec);
    timeout.nanosec = static_cast<int32_t>(wait_timeout->nsec);
  }

  DDS::ReturnCode_t status = dds_wait_set->wait(active_conditions, timeout);

  if (status == DDS::RETCODE_TIMEOUT) {
    return RMW_RET_TIMEOUT;
  }

  if (status != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to wait on wait set");
    return RMW_RET_ERROR;
  }

  // set subscriber handles to zero for all not triggered conditions
  for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
    SubscriberInfo * subscriber_info =
      static_cast<SubscriberInfo *>(subscriptions->subscribers[i]);
    if (!subscriber_info) {
      RMW_SET_ERROR_MSG("subscriber info handle is null");
      return RMW_RET_ERROR;
    }
    DDS::ReadCondition * read_condition = subscriber_info->read_condition_;
    if (!read_condition) {
      RMW_SET_ERROR_MSG("read condition handle is null");
      return RMW_RET_ERROR;
    }

    // search for subscriber condition in active set
    uint32_t j = 0;
    for (; j < active_conditions.size(); ++j) {
      if (active_conditions[j] == read_condition) {
        break;
      }
    }
    // if subscriber condition is not found in the active set
    // reset the subscriber handle
    if (!(j < active_conditions.size())) {
      subscriptions->subscribers[i] = 0;
    }
#if (DO_DEBUG)
    /* DEBUG -- */
    else
      fprintf(stderr, "wait set: active: read_cond: %p\n", read_condition);
#endif
    DDS_ReturnCode_t retcode = dds_wait_set->detach_condition(*read_condition);
    if (retcode != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("Failed to get detach condition from wait set");
    }
  }

  // set guard condition handles to zero for all not triggered conditions
  if (guard_conditions)
    {
      for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i) {
        DDS::GuardCondition * guard_condition =
          static_cast<DDS::GuardCondition *>(guard_conditions->guard_conditions[i]);
        if (!guard_condition) {
          RMW_SET_ERROR_MSG("condition handle is null");
          return RMW_RET_ERROR;
        }

        // search for guard condition in active set
        uint32_t j = 0;
        for (; j < active_conditions.size(); ++j) {
          if (active_conditions[j] == guard_condition) {
#if (DO_DEBUG)
            /* DEBUG -- */
            fprintf(stderr, "wait set: active: guard_cond: %p (clearing it)\n", guard_condition);
#endif
            guard_condition->set_trigger_value(0);
            break;
          }
        }
        
        // if the guard condition was not triggered
        // reset the guard condition handle
        if (!(j < active_conditions.size())) {
          guard_conditions->guard_conditions[i] = 0;
        } 
        DDS_ReturnCode_t retcode = dds_wait_set->detach_condition(*guard_condition);
        if (retcode != DDS_RETCODE_OK) {
          RMW_SET_ERROR_MSG("Failed to get detach condition from wait set");
        }
      }
    }
  
  // set service handles to zero for all not triggered conditions
  for (size_t i = 0; i < services->service_count; ++i) {
    ServiceInfo * service_info =
      static_cast<ServiceInfo *>(services->services[i]);
    if (!service_info) {
      RMW_SET_ERROR_MSG("service info handle is null");
      return RMW_RET_ERROR;
    }
    DDS::DataReader * request_datareader = service_info->request_datareader_;
    if (!request_datareader) {
      RMW_SET_ERROR_MSG("request datareader handle is null");
      return RMW_RET_ERROR;
    }
    DDS::StatusCondition * condition = request_datareader->get_statuscondition();

    // search for service condition in active set
    uint32_t j = 0;
    for (; j < active_conditions.size(); ++j) {
      if (active_conditions[j] == condition) {
        break;
      }
    }
    // if service condition is not found in the active set
    // reset the subscriber handle
    if (!(j < active_conditions.size())) {
      services->services[i] = 0;
    }
#if (DO_DEBUG)
    /* DEBUG -- */
    else
      fprintf(stderr, "wait set: active: req status_cond: %p\n", condition);
#endif
    DDS_ReturnCode_t retcode = dds_wait_set->detach_condition(*condition);
    if (retcode != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("Failed to get detach condition from wait set");
    }
  }

  // set client handles to zero for all not triggered conditions
  for (size_t i = 0; i < clients->client_count; ++i) {
    ClientInfo * client_info =
      static_cast<ClientInfo *>(clients->clients[i]);
    if (!client_info) {
      RMW_SET_ERROR_MSG("client info handle is null");
      return RMW_RET_ERROR;
    }
    DDS::DataReader * response_datareader = client_info->response_datareader_;
    if (!response_datareader) {
      RMW_SET_ERROR_MSG("response datareader handle is null");
      return RMW_RET_ERROR;
    }
    DDS::StatusCondition * condition = response_datareader->get_statuscondition();

    // search for service condition in active set
    uint32_t j = 0;
    for (; j < active_conditions.size(); ++j) {
      if (active_conditions[j] == condition) {
        break;
      }
    }
    // if client condition is not found in the active set
    // reset the subscriber handle
    if (!(j < active_conditions.size())) {
      clients->clients[i] = 0;
    }
#if (DO_DEBUG)
    /* DEBUG -- */
    else
      {
        fprintf(stderr, "wait set: active: rep status_cond: %p (flag: %s) (enabled: 0x%0x) (status: 0x%0x)\n",
                condition,
                condition->get_trigger_value()?"set":"clear",
                condition->get_enabled_statuses(),
                response_datareader->get_status_changes());
      }
#endif
    DDS_ReturnCode_t retcode = dds_wait_set->detach_condition(*condition);
    if (retcode != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("Failed to get detach condition from wait set");
    }
  }
  return RMW_RET_OK;
}
