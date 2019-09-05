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

#include <unordered_map>
#include <unordered_set>

#include <rmw/rmw.h>
#include <rmw/event.h>
#include <rmw/types.h>
#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/impl/cpp/macros.hpp>

#include <dds/dds.hh>
#include <dds/dds_builtinDataReader.hh>

#include "rmw_coredx_cpp/identifier.hpp"
#include "rmw_coredx_types.hpp"
#include "util.hpp"

static bool is_event_supported( rmw_event_type_t event_t )
{
  bool retval = false;
  switch ( event_t ) {
  case RMW_EVENT_LIVELINESS_CHANGED:
  case RMW_EVENT_REQUESTED_DEADLINE_MISSED:
  case RMW_EVENT_LIVELINESS_LOST:
  case RMW_EVENT_OFFERED_DEADLINE_MISSED:
    retval = true;
  case RMW_EVENT_INVALID:
    break;
  }
  return retval;
}

static bool is_event_publisher( rmw_event_type_t event_t )
{
  bool retval = false;
  switch ( event_t ) {
  case RMW_EVENT_LIVELINESS_CHANGED:
  case RMW_EVENT_REQUESTED_DEADLINE_MISSED:
    break;
  case RMW_EVENT_LIVELINESS_LOST:
  case RMW_EVENT_OFFERED_DEADLINE_MISSED:
    retval = true;
    break;
  case RMW_EVENT_INVALID:
    break;
  }
  return retval;
}
static DDS::StatusMask rmw_event_type_to_status_mask( rmw_event_type_t event_t )
{
  DDS::StatusMask retval = 0;
  switch ( event_t ) {
  case RMW_EVENT_LIVELINESS_CHANGED:
    retval = DDS::LIVELINESS_CHANGED_STATUS;
    break;
  case RMW_EVENT_REQUESTED_DEADLINE_MISSED:
    retval = DDS::REQUESTED_DEADLINE_MISSED_STATUS;
    break;
  case RMW_EVENT_LIVELINESS_LOST:
    retval = DDS::LIVELINESS_LOST_STATUS;
    break;
  case RMW_EVENT_OFFERED_DEADLINE_MISSED:
    retval = DDS::OFFERED_DEADLINE_MISSED_STATUS;
    break;
  case RMW_EVENT_INVALID:
    break;
  }
  return retval;
}

static rmw_ret_t
__gather_event_conditions(
  rmw_events_t * events,
  std::unordered_set<DDS::StatusCondition *> & status_conditions)
{
  RMW_CHECK_ARGUMENT_FOR_NULL(events, RMW_RET_INVALID_ARGUMENT);
  std::unordered_map<DDS::StatusCondition *, DDS::StatusMask> status_mask_map;
  // gather all status conditions and masks
  for (size_t i = 0; i < events->event_count; ++i) {
    auto current_event = static_cast<rmw_event_t *>(events->events[i]);
    RMW_CHECK_ARGUMENT_FOR_NULL(current_event->data, RMW_RET_INVALID_ARGUMENT);
    
    if (is_event_supported(current_event->event_type)) {
      DDS::StatusCondition * status_condition = nullptr;
      bool is_publisher = is_event_publisher( current_event->event_type );
    
      if ( is_publisher ) {
	struct CoreDXStaticPublisherInfo * pub_info = static_cast<CoreDXStaticPublisherInfo*>(current_event->data);
	status_condition = pub_info->topic_writer_->get_statuscondition();
      } else {
	struct CoreDXStaticSubscriberInfo * sub_info = static_cast<CoreDXStaticSubscriberInfo*>(current_event->data);
	status_condition = sub_info->topic_reader_->get_statuscondition();
      }
      
      if (!status_condition) {
	RMW_SET_ERROR_MSG("status condition handle is null");
	return RMW_RET_ERROR;
      }
      auto map_pair =
        status_mask_map.insert(std::pair<DDS::StatusCondition *, DDS::StatusMask>(status_condition,
										  DDS::STATUS_MASK_NONE));
      auto iterator = map_pair.first;
      status_mask_map[status_condition] = rmw_event_type_to_status_mask(current_event->event_type) |
        (*iterator).second;

    } else {
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING("event %d not supported", current_event->event_type);
    }
  }
  for (auto & pair : status_mask_map) {
    // set the status condition's mask with the supported type
    pair.first->set_enabled_statuses(pair.second);
    status_conditions.insert(pair.first);
  }
  return RMW_RET_OK;
}

static rmw_ret_t __handle_active_event_conditions(rmw_events_t * events)
{
  // enable a status condition for each event
  if (events) {
    for (size_t i = 0; i < events->event_count; ++i) {
      auto current_event = static_cast<rmw_event_t *>(events->events[i]);
      RMW_CHECK_ARGUMENT_FOR_NULL(current_event->data, RMW_RET_INVALID_ARGUMENT);
      bool is_active = false;
      if (is_event_supported(current_event->event_type)) {
	bool is_publisher = is_event_publisher( current_event->event_type );
	DDS::StatusMask status_mask = 0;
	if ( is_publisher ) {
	  struct CoreDXStaticPublisherInfo * pub_info = static_cast<CoreDXStaticPublisherInfo*>(current_event->data);
	  status_mask = pub_info->topic_writer_->get_status_changes();
	} else {
	  struct CoreDXStaticSubscriberInfo * sub_info = static_cast<CoreDXStaticSubscriberInfo*>(current_event->data);
	  status_mask = sub_info->topic_reader_->get_status_changes();
	}
	is_active = static_cast<bool>(status_mask &
				      rmw_event_type_to_status_mask( current_event->event_type) );
      }
      // if status condition is not found in the active set
      // reset the subscriber handle
      if (!is_active) {
        events->events[i] = nullptr;
      }
    }
  }
  return RMW_RET_OK;
}

template<typename SubscriberInfo, typename ServiceInfo, typename ClientInfo>
rmw_ret_t
wait(const char * implementation_identifier,
     rmw_subscriptions_t     * subscriptions,
     rmw_guard_conditions_t  * guard_conditions,
     rmw_services_t          * services,
     rmw_clients_t           * clients,
     rmw_events_t            * events,
     rmw_wait_set_t          * ros_wait_set,
     const rmw_time_t        * wait_timeout)
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

  
  // add a condition for each 'event'
  std::unordered_set<DDS::StatusCondition *> status_conditions;
  // gather all status conditions with set masks
  rmw_ret_t ret_code = __gather_event_conditions(events, status_conditions);
  if (ret_code != RMW_RET_OK) {
    return ret_code;
  }
  // enable a status condition for each event
  for (auto status_condition : status_conditions) {
    rmw_ret_t rmw_status = check_attach_condition_error(
      dds_wait_set->attach_condition(status_condition));
#if (DO_DEBUG)
    /* DEBUG -- */
    fprintf(stderr, "wait_set.attach( status_condition: %p )\n", status_condition);
#endif
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

  // add a read condition for each service
  for (size_t i = 0; i < services->service_count; ++i) {
    ServiceInfo * service_info =
      static_cast<ServiceInfo *>(services->services[i]);
    if (!service_info) {
      RMW_SET_ERROR_MSG("service info handle is null");
      return RMW_RET_ERROR;
    }
    /*
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
    */
    DDS::ReadCondition * condition = service_info->read_condition_;

#if (DO_DEBUG)
    /* DEBUG -- */
    fprintf(stderr, "wait_set.attach( req read_cond: %p )\n", condition);
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
    /*
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
    */
    DDS::ReadCondition * condition = client_info->read_condition_;
#if (DO_DEBUG)
    /* DEBUG -- */
    fprintf(stderr, "wait_set.attach( rep read_cond: %p )\n", condition);
#endif
    rmw_ret_t rmw_status = check_attach_condition_error(
      dds_wait_set->attach_condition(condition));
    if (rmw_status != RMW_RET_OK) {
      return rmw_status;
    }
  }


  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  
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

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------

  if (status == DDS::RETCODE_TIMEOUT) {
    //return RMW_RET_TIMEOUT; <--- rcl does not like this! so we'll just say OK
  } else if (status != DDS::RETCODE_OK) {
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
    else {
      /* DEBUG -- */
      fprintf(stderr, "wait set: active: read_cond: %p\n", read_condition);
    }
#endif
    DDS_ReturnCode_t retcode = dds_wait_set->detach_condition(*read_condition);
    if (retcode != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("Failed to get detach condition from wait set");
    }
  }

  // sete event handles to null for all not triggered events
  {
    rmw_ret_t rmw_ret_code = __handle_active_event_conditions(events);
    if (rmw_ret_code != RMW_RET_OK) {
      return rmw_ret_code;
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
            fprintf(stderr, "wait set: active: guard_cond: %p (clearing it)\n",
		    guard_condition);
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
    /*
    DDS::DataReader * request_datareader = service_info->request_datareader_;
    if (!request_datareader) {
      RMW_SET_ERROR_MSG("request datareader handle is null");
      return RMW_RET_ERROR;
    }
    DDS::StatusCondition * condition = request_datareader->get_statuscondition();
    */
    DDS::ReadCondition * condition = service_info->read_condition_;
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
    else {
      /* DEBUG -- */
      fprintf(stderr, "wait set: active: req read_cond: %p\n", condition);
    }
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
    /*
    DDS::DataReader * response_datareader = client_info->response_datareader_;
    if (!response_datareader) {
      RMW_SET_ERROR_MSG("response datareader handle is null");
      return RMW_RET_ERROR;
    }
    DDS::StatusCondition * condition = response_datareader->get_statuscondition();
    */
    DDS::ReadCondition * condition = client_info->read_condition_;
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
    else {
      /* DEBUG -- */
      fprintf(stderr, "wait set: active: rep read_cond: %p\n", condition);
    }
#endif
    DDS_ReturnCode_t retcode = dds_wait_set->detach_condition(*condition);
    if (retcode != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("Failed to get detach condition from wait set");
    }
  }
  return RMW_RET_OK;
}
