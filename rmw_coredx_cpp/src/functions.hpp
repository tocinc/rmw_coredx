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

// *INDENT-OFF*

#include "rosidl_typesupport_coredx_cpp/message_type_support.h"
#include "rosidl_typesupport_coredx_cpp/service_type_support.h"

class CustomDataReaderListener
  : public DDS::DataReaderListener
{
public:
  explicit
  CustomDataReaderListener(
    const char * implementation_identifier,
    rmw_guard_condition_t * graph_guard_condition)
  : graph_guard_condition_(graph_guard_condition),
    implementation_identifier_(implementation_identifier)
  {}

  std::map<std::string, std::multiset<std::string>> topic_names_and_types;
  virtual void trigger_graph_guard_condition();

protected:
  virtual void add_information(const DDS::SampleInfo & sample_info,
                               const std::string & topic_name,
                               const std::string & type_name);

  virtual void remove_information(const DDS::SampleInfo & sample_info);

private:
  struct TopicDescriptor
  {
    DDS::InstanceHandle_t instance_handle;
    std::string name;
    std::string type;
  };
  std::list<TopicDescriptor> topic_descriptors;
  rmw_guard_condition_t * graph_guard_condition_;
  const char * implementation_identifier_;
};

class CustomPublisherListener
  : public CustomDataReaderListener
{
public:
  CustomPublisherListener(
    const char * implementation_identifier,
    rmw_guard_condition_t * graph_guard_condition)
  : CustomDataReaderListener(implementation_identifier, graph_guard_condition)
  {}
  virtual void on_data_available(DDS::DataReader * reader);
};

class CustomSubscriberListener
  : public CustomDataReaderListener
{
public:
  CustomSubscriberListener(
    const char * implementation_identifier,
    rmw_guard_condition_t * graph_guard_condition)
  : CustomDataReaderListener(implementation_identifier, graph_guard_condition)
  {}
  virtual void on_data_available(DDS::DataReader * reader);
};

struct CoreDXNodeInfo
{
  DDS::DomainParticipant                 * participant;
  CustomPublisherListener                * publisher_listener;
  CustomSubscriberListener               * subscriber_listener;
  rmw_guard_condition_t                  * graph_guard_condition;
};

struct CoreDXPublisherGID
{
  DDS::InstanceHandle_t                    publication_handle;
};

struct CoreDXStaticPublisherInfo
{
  DDS::Publisher                         * dds_publisher_;
  DDS::DataWriter                        * topic_writer_;
  const message_type_support_callbacks_t * callbacks_;
  rmw_gid_t                                publisher_gid;
};

struct CoreDXStaticSubscriberInfo
{
  DDS::Subscriber                        * dds_subscriber_;
  DDS::DataReader                        * topic_reader_;
  DDS::ReadCondition                     * read_condition_;
  bool                                     ignore_local_publications;
  const message_type_support_callbacks_t * callbacks_;
};

struct CoreDXStaticClientInfo
{
  void                                   * requester_;
  DDS::DataReader                        * response_datareader_;
  DDS::ReadCondition                     * read_condition_;
  const service_type_support_callbacks_t * callbacks_;
};

struct CoreDXStaticServiceInfo
{
  void                                   * replier_;
  DDS::DataReader                        * request_datareader_;
  DDS::ReadCondition                     * read_condition_;
  const service_type_support_callbacks_t * callbacks_;
};

struct CoreDXWaitSetInfo
{
  DDS::WaitSet      * wait_set;
  DDS::ConditionSeq   active_conditions;
  DDS::ConditionSeq   attached_conditions;
};


bool  get_datareader_qos(  DDS::DomainParticipant  * participant,
                           const rmw_qos_profile_t * qos_profile,
                           DDS::DataReaderQos      & datareader_qos);

bool  get_datawriter_qos(  DDS::DomainParticipant  * participant,
                           const rmw_qos_profile_t * qos_profile,
                           DDS::DataWriterQos      & datawriter_qos);

rmw_ret_t check_attach_condition_error(DDS::ReturnCode_t retcode);

template<typename EntityQos>
bool set_entity_qos_from_profile(const rmw_qos_profile_t * qos_profile,
                                 EntityQos               & entity_qos)
{
  // Read properties from the rmw profile
  switch (qos_profile->history) {
    case RMW_QOS_POLICY_HISTORY_KEEP_LAST:
      entity_qos.history.kind = DDS::KEEP_LAST_HISTORY_QOS;
      break;
    case RMW_QOS_POLICY_HISTORY_KEEP_ALL:
      entity_qos.history.kind = DDS::KEEP_ALL_HISTORY_QOS;
      break;
    case RMW_QOS_POLICY_HISTORY_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS history policy");
      return false;
  }

  switch (qos_profile->reliability) {
    case RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT:
      entity_qos.reliability.kind = DDS::BEST_EFFORT_RELIABILITY_QOS;
      break;
    case RMW_QOS_POLICY_RELIABILITY_RELIABLE:
      entity_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
      break;
    case RMW_QOS_POLICY_RELIABILITY_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS reliability policy");
      return false;
  }

  switch (qos_profile->durability) {
    case RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL:
      entity_qos.durability.kind = DDS::TRANSIENT_LOCAL_DURABILITY_QOS;
      break;
    case RMW_QOS_POLICY_DURABILITY_VOLATILE:
      entity_qos.durability.kind = DDS::VOLATILE_DURABILITY_QOS;
      break;
    case RMW_QOS_POLICY_DURABILITY_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS durability policy");
      return false;
  }
  
  if (qos_profile->depth != RMW_QOS_POLICY_DEPTH_SYSTEM_DEFAULT) {
    entity_qos.history.depth = static_cast<int32_t>(qos_profile->depth);
  }

  // ensure the history depth is at least the requested queue size
  assert(entity_qos.history.depth >= 0);
  if (
    entity_qos.history.kind == DDS::KEEP_LAST_HISTORY_QOS &&
    static_cast<size_t>(entity_qos.history.depth) < qos_profile->depth
  )
  {
    if (qos_profile->depth > (std::numeric_limits<int32_t>::max)()) {
      RMW_SET_ERROR_MSG(
        "failed to set history depth since the requested queue size exceeds the DDS type");
      return false;
    }
    entity_qos.history.depth = static_cast<int32_t>(qos_profile->depth);
  }
  return true;
}


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
// *INDENT-ON*
