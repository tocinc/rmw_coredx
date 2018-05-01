// Copyright (C) 2015-2018 Twin Oaks Computing, Inc.
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

#ifndef RMW_COREDX_CPP__TYPES_HPP_
#define RMW_COREDX_CPP__TYPES_HPP_

#include <cassert>
#include <exception>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

#include <rmw/rmw.h>

#include "rosidl_typesupport_coredx_cpp/message_type_support.h"
#include "rosidl_typesupport_coredx_cpp/service_type_support.h"

#include <dds/dds.hh>
#include <dds/dds_builtinDataReader.hh>

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
  virtual void add_information(const DDS::InstanceHandle_t handle,
                               const std::string & topic_name,
                               const std::string & type_name);

  virtual void remove_information(const DDS::InstanceHandle_t handle);

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


#endif // RMW_COREDX_CPP__TYPES_HPP_
