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

/* ************************************************
 */
void
CustomDataReaderListener::add_information(const DDS::InstanceHandle_t handle,
                                          const std::string & topic_name,
                                          const std::string & type_name)
{
  // store topic name and type name
  auto & topic_types = topic_names_and_types[topic_name];
  topic_types.insert(type_name);
  // fprintf(stderr, "ADDING TOPICnTYPE( '%s', '%s') \n", topic_name.c_str(), type_name.c_str() );
  // store mapping to instance handle
  TopicDescriptor topic_descriptor;
  topic_descriptor.instance_handle = handle;
  topic_descriptor.name = topic_name;
  topic_descriptor.type = type_name;
  topic_descriptors.push_back(topic_descriptor);
}

void
CustomDataReaderListener::remove_information(const DDS::InstanceHandle_t handle)
{
  // find entry by instance handle
  for (auto it = topic_descriptors.begin(); it != topic_descriptors.end(); ++it) {
    if (it->instance_handle == handle) {
      // remove entries
      // fprintf(stderr, "REMOVING TOPICnTYPE( '%s', '%s') \n", it->name.c_str(), it->type.c_str() );
      auto & topic_types = topic_names_and_types[it->name];
      topic_types.erase(topic_types.find(it->type));
      if (topic_types.empty()) {
        topic_names_and_types.erase(it->name);
      }
      topic_descriptors.erase(it);
      break;
    }
  }
}

void CustomDataReaderListener::trigger_graph_guard_condition()
{
  rmw_ret_t ret = rmw_trigger_guard_condition(graph_guard_condition_);
  if (ret != RMW_RET_OK) {
    rcutils_error_string_t err = rmw_get_error_string();
    fprintf(stderr, "failed to trigger graph guard condition: %s\n", err.str );
  }
}

/* ************************************************
 */
void
CustomPublisherListener::on_data_available(DDS::DataReader * reader)
{
  DDS::DCPSPublicationDataReader * builtin_reader =
    static_cast<DDS::DCPSPublicationDataReader *>(reader);

  DDS::DCPSPublicationPtrSeq data_seq;
  DDS::SampleInfoSeq info_seq;
  DDS::ReturnCode_t retcode = builtin_reader->take(&data_seq,
                                                   &info_seq,
                                                   DDS::LENGTH_UNLIMITED,
                                                   DDS::ANY_SAMPLE_STATE,
                                                   DDS::ANY_VIEW_STATE,
                                                   DDS::ANY_INSTANCE_STATE);

  if (retcode == DDS::RETCODE_NO_DATA) {
    return;
  }
  if (retcode != DDS::RETCODE_OK) {
    fprintf(stderr, "failed to access data from the built-in reader\n");
    return;
  }

  for (uint32_t i = 0; i < data_seq.length(); ++i) {
    if (info_seq[i]->valid_data) {
      auto fqn = std::string("");
      for (uint32_t j = 0; j < data_seq[i]->partition.name.length(); j++) {
        fqn += data_seq[i]->partition.name[j];
        fqn += "/";
      }
      fqn += data_seq[i]->topic_name;
      add_information(info_seq[i]->instance_handle,
                      fqn,
                      data_seq[i]->type_name);
    } else {
      remove_information(info_seq[i]->instance_handle);
    }
  }

  if (data_seq.length() > 0) {
    this->trigger_graph_guard_condition();
  }
  
  builtin_reader->return_loan(&data_seq, &info_seq);
}

/* ************************************************
 */
void
CustomSubscriberListener::on_data_available(DDS::DataReader * reader)
{
  DDS::DCPSSubscriptionDataReader * builtin_reader =
    static_cast<DDS::DCPSSubscriptionDataReader *>(reader);

  DDS::DCPSSubscriptionPtrSeq data_seq;
  DDS::SampleInfoSeq info_seq;
  DDS::ReturnCode_t retcode = builtin_reader->take(&data_seq,
                                                   &info_seq,
                                                   DDS::LENGTH_UNLIMITED,
                                                   DDS::ANY_SAMPLE_STATE,
                                                   DDS::ANY_VIEW_STATE,
                                                   DDS::ANY_INSTANCE_STATE);

  if (retcode == DDS::RETCODE_NO_DATA) {
    return;
  }
  if (retcode != DDS::RETCODE_OK) {
    fprintf(stderr, "failed to access data from the built-in reader\n");
    return;
  }

  for (uint32_t i = 0; i < data_seq.length(); ++i) {
    if (info_seq[i]->valid_data) {
      auto fqn = std::string("");
      for (uint32_t j = 0; j < data_seq[i]->partition.name.length(); j++) {
        fqn += data_seq[i]->partition.name[j];
        fqn += "/";
      }
      fqn += data_seq[i]->topic_name;
      add_information(info_seq[i]->instance_handle,
                      fqn,
                      data_seq[i]->type_name);
    } else {
      remove_information(info_seq[i]->instance_handle);
    }
  }

  if (data_seq.length() > 0) {
    this->trigger_graph_guard_condition();
  }

  builtin_reader->return_loan(&data_seq, &info_seq);
}

