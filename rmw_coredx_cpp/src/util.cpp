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

/* ************************************************
 */
template<typename EntityQos>
bool set_entity_qos_from_profile(const rmw_qos_profile_t * qos_profile,
                                 EntityQos               & entity_qos)
{
  // and, let's just dial down latency.
  entity_qos.latency_budget.duration.sec     = 0;
  entity_qos.latency_budget.duration.nanosec = 0;
  
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


/* ************************************************
 */
rmw_ret_t check_attach_condition_error(DDS::ReturnCode_t retcode)
{
  if (retcode == DDS::RETCODE_OK) {
    return RMW_RET_OK;
  }
  if (retcode == DDS::RETCODE_OUT_OF_RESOURCES) {
    RMW_SET_ERROR_MSG("failed to attach condition to waitset: out of resources");
  } else if (retcode == DDS::RETCODE_BAD_PARAMETER) {
    RMW_SET_ERROR_MSG("failed to attach condition to waitset: condition pointer was invalid");
  } else {
    RMW_SET_ERROR_MSG("failed to attach condition to waitset");
  }
  return RMW_RET_ERROR;
}



/* ************************************************
 */
bool
get_datareader_qos(DDS::DomainParticipant  * participant,
                   const rmw_qos_profile_t * qos_profile,
                   DDS::DataReaderQos      & datareader_qos)
{
  DDS::ReturnCode_t status;
  DDS::Subscriber  *sub;

  sub = participant->create_subscriber(DDS::SUBSCRIBER_QOS_DEFAULT, nullptr, 0);
  if (!sub) {
    RMW_SET_ERROR_MSG("failed to get default datareader qos");
    return false;
  }
  status = sub->get_default_datareader_qos(datareader_qos);
  participant->delete_subscriber(sub);
  if (status != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get default datareader qos");
    return false;
  }
  if (!set_entity_qos_from_profile(qos_profile, datareader_qos)) {
    return false;
  }

  return true;
}

/* ************************************************
 */
bool
get_datawriter_qos(DDS::DomainParticipant  * participant,
                   const rmw_qos_profile_t * qos_profile,
                   DDS::DataWriterQos      & datawriter_qos)
{
  DDS::ReturnCode_t status;
  DDS::Publisher   *pub;

  pub = participant->create_publisher(DDS::PUBLISHER_QOS_DEFAULT, nullptr, 0);
  if (!pub) {
    RMW_SET_ERROR_MSG("failed to get default datareader qos");
    return false;
  }
  status = pub->get_default_datawriter_qos(datawriter_qos);
  participant->delete_publisher(pub);
  if (status != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get default datareader qos");
    return false;
  }
  if (!set_entity_qos_from_profile(qos_profile, datawriter_qos)) {
    return false;
  }

  return true;
}

/* ************************************************
 */
const message_type_support_callbacks_t *
get_callbacks(const rosidl_message_type_support_t * type_supports )
{
  const rosidl_message_type_support_t * ts; 
  RMW_COREDX_EXTRACT_MESSAGE_TYPESUPPORT(type_supports, ts);
  if ( ts )
    return static_cast<const message_type_support_callbacks_t *>(ts->data);
  else
    return NULL;
}
  

