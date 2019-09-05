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
rmw_publisher_get_actual_qos( const rmw_publisher_t   * publisher,
			            rmw_qos_profile_t * qos )
{
  if (!publisher) {
    RMW_SET_ERROR_MSG("publisher handle is null");
    return RMW_RET_ERROR;
  }
  if (!qos) {
    RMW_SET_ERROR_MSG("qos handle is null");
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

  DDS::DataWriterQos dw_qos;
  writer->get_qos( dw_qos );

  if ( dw_qos.history.kind == DDS::KEEP_LAST_HISTORY_QOS )
    qos->history = RMW_QOS_POLICY_HISTORY_KEEP_LAST;
  else
    qos->history = RMW_QOS_POLICY_HISTORY_KEEP_ALL;
  qos->depth = dw_qos.history.depth;

  if ( dw_qos.reliability.kind == DDS::RELIABLE_RELIABILITY_QOS )
    qos->reliability = RMW_QOS_POLICY_RELIABILITY_RELIABLE;
  else
    qos->reliability = RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT;
  
  if ( dw_qos.durability.kind == DDS::TRANSIENT_LOCAL_DURABILITY_QOS )
    qos->durability = RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL;
  else
    qos->durability = RMW_QOS_POLICY_DURABILITY_VOLATILE;

  qos->deadline.sec  = dw_qos.deadline.period.sec;
  qos->deadline.nsec = dw_qos.deadline.period.nanosec;
  
  qos->lifespan.sec  = dw_qos.lifespan.duration.sec;
  qos->lifespan.nsec = dw_qos.lifespan.duration.nanosec;

  switch ( dw_qos.liveliness.kind )
    {
    case DDS::MANUAL_BY_TOPIC_LIVELINESS_QOS:
      qos->liveliness  = RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_TOPIC;
      break;
    case DDS::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS:
      qos->liveliness  = RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_NODE;
      break;
    case DDS::AUTOMATIC_LIVELINESS_QOS:
    default:
      qos->liveliness  = RMW_QOS_POLICY_LIVELINESS_AUTOMATIC;
      break;
    }
  qos->liveliness_lease_duration.sec = dw_qos.liveliness.lease_duration.sec;
  qos->liveliness_lease_duration.nsec = dw_qos.liveliness.lease_duration.nanosec;
  
  
  return RMW_RET_OK;
}

#if defined(__cplusplus)
}
#endif
