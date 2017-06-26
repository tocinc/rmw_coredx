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

#ifdef CoreDX_GLIBCXX_USE_CXX11_ABI_ZERO
#define _GLIBCXX_USE_CXX11_ABI 0
#endif

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

#include <dds/dds.hh>
#include <dds/dds_builtinDataReader.hh>

#include <rmw/rmw.h>
#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/types.h>
#include <rmw/sanity_checks.h>
#include <rmw/names_and_types.h>
#include <rcutils/strdup.h>
#include <rmw/convert_rcutils_ret_to_rmw_ret.h>

#include <rmw/impl/cpp/macros.hpp>

#include "functions.hpp"
#include "rosidl_typesupport_coredx_c/identifier.h"
#include "rosidl_typesupport_coredx_cpp/identifier.hpp"

// *INDENT-OFF*

#define RMW_COREDX_EXTRACT_MESSAGE_TYPESUPPORT(TYPE_SUPPORTS, TYPE_SUPPORT) \
  do {                                                                  \
    if (!TYPE_SUPPORTS) {                                               \
      RMW_SET_ERROR_MSG("type supports handle is null");                \
      return NULL;                                                      \
    }                                                                   \
    TYPE_SUPPORT =                                                      \
      get_message_typesupport_handle                                    \
      (TYPE_SUPPORTS,                                                   \
       rosidl_typesupport_coredx_c__identifier);                        \
    if (!TYPE_SUPPORT)                                                  \
      TYPE_SUPPORT =                                                    \
        get_message_typesupport_handle                                  \
        (TYPE_SUPPORTS,                                                 \
         rosidl_typesupport_coredx_cpp::typesupport_coredx_identifier); \
    if (!TYPE_SUPPORT) {                                                \
      char __msg[1024];                                                 \
      snprintf(__msg, 1024,                                             \
               "type support handle implementation '%s' (%p) does not match valid type supports " \
               "('%s' (%p), '%s' (%p))",                                \
               TYPE_SUPPORTS->typesupport_identifier,                   \
               static_cast<const void *>(TYPE_SUPPORTS->typesupport_identifier), \
               rosidl_typesupport_coredx_cpp::typesupport_coredx_identifier,   \
               static_cast<const void *>(rosidl_typesupport_coredx_cpp::typesupport_coredx_identifier), \
               rosidl_typesupport_coredx_c__identifier,                 \
               static_cast<const void *>(rosidl_typesupport_coredx_c__identifier)); \
      RMW_SET_ERROR_MSG(__msg);                                         \
      return NULL;                                                      \
    } } while(0)

#define RMW_COREDX_EXTRACT_SERVICE_TYPESUPPORT(TYPE_SUPPORTS, TYPE_SUPPORT) \
  do {                                                                  \
    if (!TYPE_SUPPORTS) {                                               \
      RMW_SET_ERROR_MSG("type supports handle is null");                \
      return NULL;                                                      \
    }                                                                   \
    TYPE_SUPPORT =                                                      \
      get_service_typesupport_handle                                    \
      ( TYPE_SUPPORTS, rosidl_typesupport_coredx_c__identifier);        \
    if (!TYPE_SUPPORT)                                                  \
      TYPE_SUPPORT =                                                    \
        get_service_typesupport_handle                                  \
        (TYPE_SUPPORTS,                                                 \
         rosidl_typesupport_coredx_cpp::typesupport_coredx_identifier); \
    if (!TYPE_SUPPORT) {                                                \
      char __msg[1024];                                                 \
      snprintf(__msg, 1024,                                             \
               "type support handle implementation '%s' (%p) does not match valid type supports " \
               "('%s' (%p), '%s' (%p))",                                \
               TYPE_SUPPORTS->typesupport_identifier,                   \
               static_cast<const void *>(TYPE_SUPPORTS->typesupport_identifier), \
               rosidl_typesupport_coredx_cpp::typesupport_coredx_identifier,   \
               static_cast<const void *>(rosidl_typesupport_coredx_cpp::typesupport_coredx_identifier), \
               rosidl_typesupport_coredx_c__identifier,                 \
               static_cast<const void *>(rosidl_typesupport_coredx_c__identifier)); \
      RMW_SET_ERROR_MSG(__msg);                                         \
      return NULL;                                                      \
    } } while (0)                                                                   
    


const char * toc_coredx_identifier = "rmw_coredx_cpp";

/* ************************************************
 */
inline std::string
_create_type_name(
  const message_type_support_callbacks_t * callbacks,
  const std::string & sep)
{
  return
    std::string(callbacks->package_name) +
    "::" + sep + "::dds_::" + callbacks->message_name + "_";
}

/* ************************************************
 */
static char *
do_strdup( const char *n)
{
  char * retval =
    reinterpret_cast<char *>(rmw_allocate(sizeof(char) * strlen((char*)n) + 1));
  if (retval)
    memcpy(const_cast<char *>(retval), n, strlen(n) + 1);
  return retval;
}


/* ************************************************
 */
void
CustomDataReaderListener::add_information(const DDS::SampleInfo & sample_info,
                                          const std::string & topic_name,
                                          const std::string & type_name)
{
  // store topic name and type name
  auto & topic_types = topic_names_and_types[topic_name];
  topic_types.insert(type_name);
  // store mapping to instance handle
  TopicDescriptor topic_descriptor;
  topic_descriptor.instance_handle = sample_info.instance_handle;
  topic_descriptor.name = topic_name;
  topic_descriptor.type = type_name;
  topic_descriptors.push_back(topic_descriptor);
}

void
CustomDataReaderListener::remove_information(const DDS::SampleInfo & sample_info)
{
  // find entry by instance handle
  for (auto it = topic_descriptors.begin(); it != topic_descriptors.end(); ++it) {
    if (it->instance_handle == sample_info.instance_handle) {
      // remove entries
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
    fprintf(stderr, "failed to trigger graph guard condition: %s\n", rmw_get_error_string_safe());
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
      add_information(*(info_seq[i]), data_seq[i]->topic_name, data_seq[i]->type_name);
    } else {
      remove_information(*(info_seq[i]));
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
      add_information(*(info_seq[i]), data_seq[i]->topic_name, data_seq[i]->type_name);
    } else {
      remove_information(*(info_seq[i]));
    }
  }

  if (data_seq.length() > 0) {
    this->trigger_graph_guard_condition();
  }

  builtin_reader->return_loan(&data_seq, &info_seq);
}

/* ************************************************
 */
const char *
rmw_get_implementation_identifier( )
{
  return toc_coredx_identifier;
}

/* ************************************************
 */
rmw_ret_t
rmw_init ()
{
  rmw_ret_t retval;
  DDS::DomainParticipantFactory * dpf_ = DDS::DomainParticipantFactory::get_instance();
  if (dpf_)
    retval = RMW_RET_OK;
  else {
    RMW_SET_ERROR_MSG("failed to get participant factory");
    retval = RMW_RET_ERROR;
  }
  return retval;
}

/* ************************************************
 */
rmw_node_t *
rmw_create_node( const char    * name,
                 const char    * namespace_,
                 size_t          domain_id, 
	             const rmw_node_security_options_t * security_options )
{
  DDS::DomainParticipantFactory * dpf_ = DDS::DomainParticipantFactory::get_instance();
  if (!dpf_) {
    RMW_SET_ERROR_MSG("failed to get participant factory");
    return NULL;
  }

  DDS::DomainParticipantQos dp_qos;
  dpf_->get_default_participant_qos(dp_qos);
  strncpy(dp_qos.entity_name.value, name, COREDX_ENTITY_NAME_MAX);
  
  DDS::DomainId_t           domain = static_cast<DDS::DomainId_t>(domain_id);
  DDS::DomainParticipant  * participant =
    dpf_->create_participant(domain,
                             DDS::PARTICIPANT_QOS_DEFAULT,
                             NULL,
                             0);
  if (!participant) {
    RMW_SET_ERROR_MSG("failed to create participant");
    return NULL;
  }

  rmw_node_t               * node_handle = nullptr;
  CoreDXNodeInfo           * node_info   = nullptr;
  rmw_guard_condition_t    * graph_guard_condition = nullptr;
  CustomPublisherListener  * publisher_listener = nullptr;
  CustomSubscriberListener * subscriber_listener = nullptr;
  void                     * buf         = nullptr;
  DDS::DataReader          * data_reader = nullptr;
  DDS::DCPSPublicationDataReader  * builtin_publication_datareader = nullptr;
  DDS::DCPSSubscriptionDataReader * builtin_subscription_datareader = nullptr;
  DDS::Subscriber                 * builtin_subscriber = participant->get_builtin_subscriber();
  if (!builtin_subscriber) {
    RMW_SET_ERROR_MSG("builtin subscriber handle is null");
    goto fail;
  }
  
  // graph guard condition 
  graph_guard_condition = rmw_create_guard_condition( );
  if (!graph_guard_condition) {
    RMW_SET_ERROR_MSG("failed to create graph guard condition");
    goto fail;
  }
  
  // setup publications listener
  data_reader = builtin_subscriber->lookup_datareader("DCPSPublication");
  builtin_publication_datareader =
    static_cast<DDS::DCPSPublicationDataReader *>(data_reader);
  if (!builtin_publication_datareader) {
    RMW_SET_ERROR_MSG("builtin publication datareader handle is null");
    goto fail;
  }

  buf = rmw_allocate(sizeof(CustomPublisherListener));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  RMW_TRY_PLACEMENT_NEW(publisher_listener, buf, goto fail, CustomPublisherListener,
                        toc_coredx_identifier, graph_guard_condition)
  buf = nullptr;
  builtin_publication_datareader->set_listener(publisher_listener, DDS::DATA_AVAILABLE_STATUS);

  // setup subscriber listener
  data_reader = builtin_subscriber->lookup_datareader("DCPSSubscription");
  builtin_subscription_datareader =
    static_cast<DDS::DCPSSubscriptionDataReader *>(data_reader);
  if (!builtin_subscription_datareader) {
    RMW_SET_ERROR_MSG("builtin subscription datareader handle is null");
    goto fail;
  }

  buf = rmw_allocate(sizeof(CustomSubscriberListener));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  RMW_TRY_PLACEMENT_NEW(subscriber_listener, buf, goto fail, CustomSubscriberListener,
                        toc_coredx_identifier, graph_guard_condition)
  buf = nullptr;
  builtin_subscription_datareader->set_listener(subscriber_listener, DDS::DATA_AVAILABLE_STATUS);

  // setup 'node handle'
  node_handle = rmw_node_allocate();
  if (!node_handle) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node handle");
    goto fail;
  }
  node_handle->implementation_identifier = toc_coredx_identifier;
  node_handle->data                      = participant;
  node_handle->name                      = do_strdup(name);
  node_handle->namespace_                = do_strdup(namespace_);
  if (!node_handle->name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node name");
    goto fail;
  }
  
  buf = rmw_allocate(sizeof(CoreDXNodeInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  RMW_TRY_PLACEMENT_NEW(node_info, buf, goto fail, CoreDXNodeInfo)
  buf = nullptr;
  node_info->participant         = participant;
  node_info->publisher_listener  = publisher_listener;
  node_info->subscriber_listener = subscriber_listener;
  node_info->graph_guard_condition = graph_guard_condition;
  
  node_handle->implementation_identifier = toc_coredx_identifier;
  node_handle->data = node_info;
  return node_handle;

fail:
  DDS::ReturnCode_t status = dpf_->delete_participant(participant);
  if (status != DDS::RETCODE_OK) {
    std::stringstream ss;
    ss << "leaking participant while handling failure at " <<
      __FILE__ << ":" << __LINE__;
    (std::cerr << ss.str()).flush();
  }
  if (graph_guard_condition) {
    rmw_ret_t ret = rmw_destroy_guard_condition(graph_guard_condition);
    if (ret != RMW_RET_OK) {
      std::stringstream ss;
      ss << "failed to destroy guard condition while handling failure at " <<
        __FILE__ << ":" << __LINE__;
      (std::cerr << ss.str()).flush();
    }
  }
  if (publisher_listener) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      publisher_listener->~CustomPublisherListener(), CustomPublisherListener)
    rmw_free(publisher_listener);
  }
  if (subscriber_listener) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      subscriber_listener->~CustomSubscriberListener(), CustomSubscriberListener)
    rmw_free(subscriber_listener);
  }
  if (node_handle) {
    if (node_handle->name) {
      rmw_free(const_cast<char *>(node_handle->name));
    }
    if (node_handle->namespace_) {
      rmw_free(const_cast<char *>(node_handle->namespace_));
    }
    rmw_free(node_handle);
  }
  if (node_info) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      node_info->~CoreDXNodeInfo(), CoreDXNodeInfo)
    rmw_free(node_info);
  }
  if (buf) {
    rmw_free(buf);
  }
  return NULL;
}

/* ************************************************
 */
rmw_ret_t
rmw_destroy_node( rmw_node_t     * node )
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(node handle,
                                   node->implementation_identifier, toc_coredx_identifier,
                                   return RMW_RET_ERROR);

  DDS::DomainParticipantFactory * dpf_ = DDS::DomainParticipantFactory::get_instance();
  if (!dpf_) {
    RMW_SET_ERROR_MSG("failed to get participant factory");
    return RMW_RET_ERROR;
  }

  auto node_info = static_cast<CoreDXNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }
  auto participant = static_cast<DDS::DomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
  }
  // This unregisters types and destroys topics which were shared between
  // publishers and subscribers and could not be cleaned up in the delete functions.
  if (participant->delete_contained_entities() != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to delete contained entities of participant");
    return RMW_RET_ERROR;
  }

  DDS::ReturnCode_t ret = dpf_->delete_participant(participant);
  if (ret != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to delete participant");
    return RMW_RET_ERROR;
  }

  if (node_info->publisher_listener) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      node_info->publisher_listener->~CustomPublisherListener(), CustomPublisherListener)
    rmw_free(node_info->publisher_listener);
    node_info->publisher_listener = nullptr;
  }
  if (node_info->subscriber_listener) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      node_info->subscriber_listener->~CustomSubscriberListener(), CustomSubscriberListener)
    rmw_free(node_info->subscriber_listener);
    node_info->subscriber_listener = nullptr;
  }
  if (node_info->graph_guard_condition) {
    rmw_ret_t rmw_ret =
      rmw_destroy_guard_condition(node_info->graph_guard_condition);
    if (rmw_ret != RMW_RET_OK) {
      RMW_SET_ERROR_MSG("failed to delete graph guard condition");
      return RMW_RET_ERROR;
    }
    node_info->graph_guard_condition = nullptr;
  }

  rmw_free(node_info);
  node->data = nullptr;

  if (node->name) {
    rmw_free(const_cast<char *>(node->name));
    node->name = NULL;
  }
  if (node->namespace_) {
    rmw_free(const_cast<char *>(node->namespace_));
    node->namespace_ = NULL;
  }
  rmw_node_free(node);

  return RMW_RET_OK;
}

/* ************************************************
 */
rmw_publisher_t *
rmw_create_publisher( const rmw_node_t        * node,
                      const rosidl_message_type_support_t * type_supports,
                      const char              * topic_name,
                      const rmw_qos_profile_t * qos_policies )
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return NULL;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, toc_coredx_identifier,
    return NULL);

  const rosidl_message_type_support_t * type_support; 
  RMW_COREDX_EXTRACT_MESSAGE_TYPESUPPORT(type_supports, type_support);
  
  if (!qos_policies) {
    RMW_SET_ERROR_MSG("qos_policies is null");
    return nullptr;
  }

  auto node_info = static_cast<CoreDXNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return NULL;
  }
  auto participant = static_cast<DDS::DomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return NULL;
  }

  const message_type_support_callbacks_t * callbacks =
    static_cast<const message_type_support_callbacks_t *>(type_support->data);
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return NULL;
  }
  std::string type_name = _create_type_name(callbacks, "msg");
  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_publisher_t * publisher = nullptr;
  bool registered;
  DDS::PublisherQos publisher_qos;
  DDS::ReturnCode_t status;
  DDS::Publisher * dds_publisher = nullptr;
  DDS::Topic * topic;
  DDS::TopicDescription * topic_description = nullptr;
  DDS::DataWriterQos datawriter_qos;
  DDS::DataWriter * topic_writer = nullptr;
  void * buf = nullptr;
  CoreDXStaticPublisherInfo * publisher_info = nullptr;
  // Begin initializing elements
  publisher = rmw_publisher_allocate();
  if (!publisher) {
    RMW_SET_ERROR_MSG("failed to allocate publisher");
    goto fail;
  }

  registered = callbacks->register_type(participant, type_name.c_str());
  if (!registered) {
    RMW_SET_ERROR_MSG("failed to register type");
    goto fail;
  }

  status = participant->get_default_publisher_qos(publisher_qos);
  if (status != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get default publisher qos");
    goto fail;
  }

  dds_publisher = participant->create_publisher(
    publisher_qos, NULL, 0);
  if (!dds_publisher) {
    RMW_SET_ERROR_MSG("failed to create publisher");
    goto fail;
  }

  topic_description = participant->lookup_topicdescription(topic_name);
  if (!topic_description) {
    DDS::TopicQos default_topic_qos;
    status = participant->get_default_topic_qos(default_topic_qos);
    if (status != DDS::RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to get default topic qos");
      goto fail;
    }

    topic = participant->create_topic(
      topic_name, type_name.c_str(), default_topic_qos, NULL, 0);
    if (!topic) {
      RMW_SET_ERROR_MSG("failed to create topic");
      goto fail;
    }
  } else {
    DDS::Duration_t timeout;
    timeout.sec = 0;
    timeout.nanosec = 0;
    topic = participant->find_topic(topic_name, timeout);
    if (!topic) {
      RMW_SET_ERROR_MSG("failed to find topic");
      goto fail;
    }
  }

  if (!get_datawriter_qos(participant, qos_policies, datawriter_qos)) {
    // error string was set within the function
    goto fail;
  }

  topic_writer = dds_publisher->create_datawriter(
    topic, datawriter_qos, NULL, 0);
  if (!topic_writer) {
    RMW_SET_ERROR_MSG("failed to create datawriter");
    goto fail;
  }

  // Allocate memory for the CoreDXStaticPublisherInfo object.
  buf = rmw_allocate(sizeof(CoreDXStaticPublisherInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the CoreDXStaticPublisherInfo in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(publisher_info, buf, goto fail, CoreDXStaticPublisherInfo)
  buf = nullptr;  // Only free the publisher_info pointer; don't need the buf pointer anymore.
  publisher_info->dds_publisher_ = dds_publisher;
  publisher_info->topic_writer_ = topic_writer;
  publisher_info->callbacks_ = callbacks;
  publisher_info->publisher_gid.implementation_identifier = toc_coredx_identifier;
  static_assert(
    sizeof(CoreDXPublisherGID) <= RMW_GID_STORAGE_SIZE,
    "RMW_GID_STORAGE_SIZE insufficient to store the rmw_coredx_cpp GID implemenation."
  );
  // Zero the data memory.
  memset(publisher_info->publisher_gid.data, 0, RMW_GID_STORAGE_SIZE);
  {
    auto publisher_gid =
      reinterpret_cast<CoreDXPublisherGID *>(publisher_info->publisher_gid.data);
    publisher_gid->publication_handle = topic_writer->get_instance_handle();
  }
  publisher_info->publisher_gid.implementation_identifier = toc_coredx_identifier;

  publisher->implementation_identifier = toc_coredx_identifier;
  publisher->data = publisher_info;
  publisher->topic_name = do_strdup(topic_name);
  if (!publisher->topic_name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node name");
    goto fail;
  }

  node_info->publisher_listener->trigger_graph_guard_condition();
  
  return publisher;

 fail:
  if (publisher) {
    rmw_publisher_free(publisher);
  }
  // Assumption: participant is valid.
  if (dds_publisher) {
    if (topic_writer) {
      if (dds_publisher->delete_datawriter(topic_writer) != DDS::RETCODE_OK) {
        std::stringstream ss;
        ss << "leaking datawriter while handling failure at " <<
          __FILE__ << ":" << __LINE__ << '\n';
        (std::cerr << ss.str()).flush();
      }
    }
    if (participant->delete_publisher(dds_publisher) != DDS::RETCODE_OK) {
      std::stringstream ss;
      ss << "leaking publisher while handling failure at " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (publisher_info) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      publisher_info->~CoreDXStaticPublisherInfo(), CoreDXStaticPublisherInfo)
    rmw_free(publisher_info);
  }
  if (buf) {
    rmw_free(buf);
  }
  return NULL;
}

/* ************************************************
 */
rmw_ret_t
rmw_destroy_publisher( rmw_node_t      * node,
                       rmw_publisher_t * publisher)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, toc_coredx_identifier,
    return RMW_RET_ERROR);

  if (!publisher) {
    RMW_SET_ERROR_MSG("publisher handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    publisher handle,
    publisher->implementation_identifier, toc_coredx_identifier,
    return RMW_RET_ERROR);

  auto node_info = static_cast<CoreDXNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }
  auto participant = static_cast<DDS::DomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return RMW_RET_ERROR;
  }
  // we don't need to unregister types with the participant.
  CoreDXStaticPublisherInfo * publisher_info = (CoreDXStaticPublisherInfo *)publisher->data;
  if (publisher_info) {
    DDS::Publisher * dds_publisher = publisher_info->dds_publisher_;
    if (dds_publisher) {
      if (publisher_info->topic_writer_) {
        if (dds_publisher->delete_datawriter(publisher_info->topic_writer_) != DDS::RETCODE_OK) {
          RMW_SET_ERROR_MSG("failed to delete datawriter");
          return RMW_RET_ERROR;
        }
        publisher_info->topic_writer_ = nullptr;
      }
      if (participant->delete_publisher(dds_publisher) != DDS::RETCODE_OK) {
        RMW_SET_ERROR_MSG("failed to delete publisher");
        return RMW_RET_ERROR;
      }
      publisher_info->dds_publisher_ = nullptr;
    } else if (publisher_info->topic_writer_) {
      RMW_SET_ERROR_MSG("cannot delete datawriter because the publisher is null");
      return RMW_RET_ERROR;
    }
    RMW_TRY_DESTRUCTOR(
      publisher_info->~CoreDXStaticPublisherInfo(),
      CoreDXStaticPublisherInfo, return RMW_RET_ERROR)
    rmw_free(publisher_info);
    publisher->data = nullptr;
  }
  if (publisher->topic_name) {
    rmw_free(const_cast<char *>(publisher->topic_name));
  }
  rmw_publisher_free(publisher);

  return RMW_RET_OK;
}

/* ************************************************
 */
rmw_ret_t
rmw_publish( const rmw_publisher_t * publisher,
             const void            * ros_message )
{
  if (!publisher) {
    RMW_SET_ERROR_MSG("publisher handle is null");
    return RMW_RET_ERROR;
  }
  if (publisher->implementation_identifier != toc_coredx_identifier) {
    RMW_SET_ERROR_MSG("publisher handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  if (!ros_message) {
    RMW_SET_ERROR_MSG("ros message handle is null");
    return RMW_RET_ERROR;
  }

  CoreDXStaticPublisherInfo * publisher_info =
    static_cast<CoreDXStaticPublisherInfo *>(publisher->data);
  if (!publisher_info) {
    RMW_SET_ERROR_MSG("publisher info handle is null");
    return RMW_RET_ERROR;
  }
  const message_type_support_callbacks_t * callbacks = publisher_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }
  DDS::DataWriter * topic_writer = publisher_info->topic_writer_;
  if (!topic_writer) {
    RMW_SET_ERROR_MSG("topic writer handle is null");
    return RMW_RET_ERROR;
  }

  bool published = callbacks->publish(topic_writer, ros_message);
  if (!published) {
    RMW_SET_ERROR_MSG("failed to publish message");
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

/* ************************************************
 */
rmw_subscription_t *
rmw_create_subscription( const rmw_node_t        * node,
                         const rosidl_message_type_support_t * type_supports,
                         const char              * topic_name,
                         const rmw_qos_profile_t * qos_policies,
                         bool                      ignore_local_publications )
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return NULL;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
     node handle,
     node->implementation_identifier, toc_coredx_identifier,
     return NULL)
    
  const rosidl_message_type_support_t * type_support; 
  RMW_COREDX_EXTRACT_MESSAGE_TYPESUPPORT(type_supports, type_support);
    
  if (!qos_policies) {
    RMW_SET_ERROR_MSG("qos_policies is null");
    return nullptr;
  }

  auto node_info = static_cast<CoreDXNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return NULL;
  }
  auto participant = static_cast<DDS::DomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return NULL;
  }

  const message_type_support_callbacks_t * callbacks =
    static_cast<const message_type_support_callbacks_t *>(type_support->data);
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return NULL;
  }
  std::string type_name = _create_type_name(callbacks, "msg");
  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_subscription_t * subscription = nullptr;
  bool registered;
  DDS::SubscriberQos subscriber_qos;
  DDS::ReturnCode_t status;
  DDS::Subscriber * dds_subscriber = nullptr;
  DDS::Topic * topic;
  DDS::TopicDescription * topic_description = nullptr;
  DDS::DataReaderQos datareader_qos;
  DDS::DataReader * topic_reader = nullptr;
  DDS::ReadCondition * read_condition = nullptr;
  void * buf = nullptr;
  CoreDXStaticSubscriberInfo * subscriber_info = nullptr;
  // Begin initializing elements.
  subscription = rmw_subscription_allocate();
  if (!subscription) {
    RMW_SET_ERROR_MSG("failed to allocate subscription");
    goto fail;
  }

  registered = callbacks->register_type(participant, type_name.c_str());
  if (!registered) {
    RMW_SET_ERROR_MSG("failed to register type");
    goto fail;
  }

  status = participant->get_default_subscriber_qos(subscriber_qos);
  if (status != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get default subscriber qos");
    goto fail;
  }

  dds_subscriber = participant->create_subscriber(subscriber_qos, NULL, 0);
  if (!dds_subscriber) {
    RMW_SET_ERROR_MSG("failed to create subscriber");
    goto fail;
  }

  topic_description = participant->lookup_topicdescription(topic_name);
  if (!topic_description) {
    DDS::TopicQos default_topic_qos;
    status = participant->get_default_topic_qos(default_topic_qos);
    if (status != DDS::RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to get default topic qos");
      goto fail;
    }

    topic = participant->create_topic(
      topic_name, type_name.c_str(), default_topic_qos, NULL, 0);
    if (!topic) {
      RMW_SET_ERROR_MSG("failed to create topic");
      goto fail;
    }
  } else {
    DDS::Duration_t timeout(0,0);
    topic = participant->find_topic(topic_name, timeout);
    if (!topic) {
      RMW_SET_ERROR_MSG("failed to find topic");
      goto fail;
    }
  }

  if (!get_datareader_qos(participant, qos_policies, datareader_qos)) {
    // error string was set within the function
    goto fail;
  }

  topic_reader = dds_subscriber->create_datareader(
    topic, datareader_qos,
    NULL, 0);
  if (!topic_reader) {
    RMW_SET_ERROR_MSG("failed to create datareader");
    goto fail;
  }

  read_condition = topic_reader->create_readcondition(
    DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
  if (!read_condition) {
    RMW_SET_ERROR_MSG("failed to create read condition");
    goto fail;
  }

  // Allocate memory for the CoreDXStaticSubscriberInfo object.
  buf = rmw_allocate(sizeof(CoreDXStaticSubscriberInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the CoreDXStaticSubscriberInfo in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(subscriber_info, buf, goto fail, CoreDXStaticSubscriberInfo)
  buf = nullptr;  // Only free the subscriber_info pointer; don't need the buf pointer anymore.
  subscriber_info->dds_subscriber_ = dds_subscriber;
  subscriber_info->topic_reader_ = topic_reader;
  subscriber_info->read_condition_ = read_condition;
  subscriber_info->callbacks_ = callbacks;
  subscriber_info->ignore_local_publications = ignore_local_publications;

  subscription->implementation_identifier = toc_coredx_identifier;
  subscription->data = subscriber_info;
  subscription->topic_name = do_strdup(topic_name);
  if (!subscription->topic_name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for topic name");
    goto fail;
  }

  return subscription;
fail:
  if (subscription) {
    rmw_subscription_free(subscription);
  }
  // Assumption: participant is valid.
  if (dds_subscriber) {
    if (topic_reader) {
      if (read_condition) {
        if (topic_reader->delete_readcondition(read_condition) != DDS::RETCODE_OK) {
          std::stringstream ss;
          ss << "leaking readcondition while handling failure at " <<
            __FILE__ << ":" << __LINE__ << '\n';
          (std::cerr << ss.str()).flush();
        }
      }
      if (dds_subscriber->delete_datareader(topic_reader) != DDS::RETCODE_OK) {
        std::stringstream ss;
        ss << "leaking datareader while handling failure at " <<
          __FILE__ << ":" << __LINE__ << '\n';
        (std::cerr << ss.str()).flush();
      }
    }
    if (participant->delete_subscriber(dds_subscriber) != DDS::RETCODE_OK) {
      std::stringstream ss;
      std::cerr << "leaking subscriber while handling failure at " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (subscriber_info) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      subscriber_info->~CoreDXStaticSubscriberInfo(), CoreDXStaticSubscriberInfo)
    rmw_free(subscriber_info);
  }
  if (buf) {
    rmw_free(buf);
  }
  return NULL;
}

/* ************************************************
 */
rmw_ret_t
rmw_destroy_subscription( rmw_node_t         * node,
                         rmw_subscription_t * subscription )
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, toc_coredx_identifier,
    return RMW_RET_ERROR)

  if (!subscription) {
    RMW_SET_ERROR_MSG("subscription handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    subscription handle,
    subscription->implementation_identifier, toc_coredx_identifier,
    return RMW_RET_ERROR)

  auto node_info = static_cast<CoreDXNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }
  auto participant = static_cast<DDS::DomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return RMW_RET_ERROR;
  }

  CoreDXStaticSubscriberInfo * subscriber_info =
    (CoreDXStaticSubscriberInfo *)subscription->data;
  if (subscriber_info) {
    auto dds_subscriber = subscriber_info->dds_subscriber_;
    if (dds_subscriber) {
      auto topic_reader = subscriber_info->topic_reader_;
      if (topic_reader) {
        auto read_condition = subscriber_info->read_condition_;
        if (read_condition) {
          if (topic_reader->delete_readcondition(read_condition) != DDS::RETCODE_OK) {
            RMW_SET_ERROR_MSG("failed to delete readcondition");
            return RMW_RET_ERROR;
          }
          subscriber_info->read_condition_ = nullptr;
        }
        if (dds_subscriber->delete_datareader(topic_reader) != DDS::RETCODE_OK) {
          RMW_SET_ERROR_MSG("failed to delete datareader");
          return RMW_RET_ERROR;
        }
        subscriber_info->topic_reader_ = nullptr;
      } else if (subscriber_info->read_condition_) {
        RMW_SET_ERROR_MSG("cannot delete readcondition because the datareader is null");
        return RMW_RET_ERROR;
      }
      if (participant->delete_subscriber(dds_subscriber) != DDS::RETCODE_OK) {
        RMW_SET_ERROR_MSG("failed to delete subscriber");
        return RMW_RET_ERROR;
      }
      subscriber_info->dds_subscriber_ = nullptr;
    } else if (subscriber_info->topic_reader_) {
      RMW_SET_ERROR_MSG("cannot delete datareader because the subscriber is null");
      return RMW_RET_ERROR;
    }
    RMW_TRY_DESTRUCTOR(
      subscriber_info->~CoreDXStaticSubscriberInfo(),
      CoreDXStaticSubscriberInfo, return RMW_RET_ERROR)
    rmw_free(subscriber_info);
    subscription->data = nullptr;
  }
  if (subscription->topic_name) {
    rmw_free(const_cast<char *>(subscription->topic_name));
  }
  rmw_subscription_free(subscription);

  return RMW_RET_OK;
}

/* ************************************************
 */
static rmw_ret_t
_take( const rmw_subscription_t * subscription,
       void                     * ros_message,
       bool                     * taken,
       DDS::InstanceHandle_t    * sending_publication_handle )
{
  if (!subscription) {
    RMW_SET_ERROR_MSG("subscription handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    subscription handle,
    subscription->implementation_identifier, toc_coredx_identifier,
    return RMW_RET_ERROR)

  if (!ros_message) {
    RMW_SET_ERROR_MSG("ros message handle is null");
    return RMW_RET_ERROR;
  }
  if (!taken) {
    RMW_SET_ERROR_MSG("taken handle is null");
    return RMW_RET_ERROR;
  }

  CoreDXStaticSubscriberInfo * subscriber_info =
    static_cast<CoreDXStaticSubscriberInfo *>(subscription->data);
  if (!subscriber_info) {
    RMW_SET_ERROR_MSG("subscriber info handle is null");
    return RMW_RET_ERROR;
  }
  DDS::DataReader * topic_reader = subscriber_info->topic_reader_;
  if (!topic_reader) {
    RMW_SET_ERROR_MSG("topic reader handle is null");
    return RMW_RET_ERROR;
  }
  const message_type_support_callbacks_t * callbacks = subscriber_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }

  bool success = callbacks->take(
    topic_reader, subscriber_info->ignore_local_publications, ros_message, taken,
    sending_publication_handle);

  return success ? RMW_RET_OK : RMW_RET_ERROR;
}

/* ************************************************
 */
rmw_ret_t
rmw_take( const rmw_subscription_t * subscription,
          void                     * ros_message,
          bool                     * taken )
{
  return _take(subscription, ros_message, taken, nullptr);
}

/* ************************************************
 */
rmw_ret_t
rmw_take_with_info( const rmw_subscription_t * subscription,
                    void                     * ros_message,
                    bool                     * taken,
                    rmw_message_info_t       * message_info )
{
  if (!message_info) {
    RMW_SET_ERROR_MSG("message info is null");
    return RMW_RET_ERROR;
  }
  DDS::InstanceHandle_t sending_publication_handle;
  auto ret = _take(subscription, ros_message, taken, &sending_publication_handle);
  if (ret != RMW_RET_OK) {
    // Error string is already set.
    return RMW_RET_ERROR;
  }

  rmw_gid_t * sender_gid = &message_info->publisher_gid;
  sender_gid->implementation_identifier = toc_coredx_identifier;
  memset(sender_gid->data, 0, RMW_GID_STORAGE_SIZE);
  auto detail = reinterpret_cast<CoreDXPublisherGID *>(sender_gid->data);
  detail->publication_handle = sending_publication_handle;
  return RMW_RET_OK;
}


/* ************************************************
 * ************************************************
 * ************************************************/


/* ************************************************
 */
rmw_client_t *
rmw_create_client( const rmw_node_t                   * node,
                  const rosidl_service_type_support_t * type_supports,
                  const char                          * service_name,
                  const rmw_qos_profile_t             * qos_profile )
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return NULL;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, toc_coredx_identifier,
    return NULL)

  const rosidl_service_type_support_t * type_support; 
  RMW_COREDX_EXTRACT_SERVICE_TYPESUPPORT(type_supports, type_support);

  if (!qos_profile) {
    RMW_SET_ERROR_MSG("qos_profile is null");
    return nullptr;
  }

  auto node_info = static_cast<CoreDXNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return NULL;
  }
  auto participant = static_cast<DDS::DomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return NULL;
  }

  const service_type_support_callbacks_t * callbacks =
    static_cast<const service_type_support_callbacks_t *>(type_support->data);
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return NULL;
  }
  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_client_t * client = nullptr;
  DDS::DataReaderQos datareader_qos;
  DDS::DataWriterQos datawriter_qos;
  DDS::DataReader * response_datareader = nullptr;
  DDS::ReadCondition * read_condition = nullptr;
  void * requester = nullptr;
  void * buf = nullptr;
  CoreDXStaticClientInfo * client_info = nullptr;

  // Begin inializing elements.
  client = rmw_client_allocate();
  if (!client) {
    RMW_SET_ERROR_MSG("failed to allocate client");
    goto fail;
  }

  if (!get_datareader_qos(participant, qos_profile, datareader_qos)) {
    // error string was set within the function
    goto fail;
  }

  if (!get_datawriter_qos(participant, qos_profile, datawriter_qos)) {
    // error string was set within the function
    goto fail;
  }

  requester = callbacks->create_requester(
    participant, service_name, &datareader_qos, &datawriter_qos,
    reinterpret_cast<void **>(&response_datareader), &rmw_allocate);
  if (!requester) {
    RMW_SET_ERROR_MSG("failed to create requester");
    goto fail;
  }
  if (!response_datareader) {
    RMW_SET_ERROR_MSG("data reader handle is null");
    goto fail;
  }

  read_condition = response_datareader->create_readcondition(
     DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
  if (!read_condition) {
    RMW_SET_ERROR_MSG("failed to create read condition");
    goto fail;
  }

  buf = rmw_allocate(sizeof(CoreDXStaticClientInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the CoreDXStaticClientInfo in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(client_info, buf, goto fail, CoreDXStaticClientInfo)
  buf = nullptr;  // Only free the client_info pointer; don't need the buf pointer anymore.
  client_info->requester_ = requester;
  client_info->callbacks_ = callbacks;
  client_info->response_datareader_ = response_datareader;
  client_info->read_condition_ = read_condition;

  client->implementation_identifier = toc_coredx_identifier;
  client->data = client_info;
  client->service_name = do_strdup( service_name );
  if (!client->service_name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node name");
    goto fail;
  }
  
  return client;
fail:
  if (client_info) {
    if (response_datareader && client_info->read_condition_ ) {
      response_datareader->delete_readcondition(client_info->read_condition_);
    }
    
    // deallocate requester (currently allocated with new elsewhere)
    if (callbacks && client_info->requester_) {
      callbacks->destroy_requester(client_info->requester_, &rmw_free);
    }
    
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
                                           client_info->~CoreDXStaticClientInfo(), CoreDXStaticClientInfo)
      rmw_free(client_info);
  }
  
  if (client) {
    if (client->service_name) {
      rmw_free(const_cast<char *>(client->service_name));
    }
    rmw_client_free(client);
  }
  
  if (buf) {
    rmw_free(buf);
  }
  return NULL;
}

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

/* ************************************************
 */
rmw_ret_t
rmw_send_request( const rmw_client_t * client,
                  const void         * ros_request,
                  int64_t            * sequence_id )
{
  if (!client) {
    RMW_SET_ERROR_MSG("client handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client handle,
    client->implementation_identifier, toc_coredx_identifier,
    return RMW_RET_ERROR)

  if (!ros_request) {
    RMW_SET_ERROR_MSG("ros request handle is null");
    return RMW_RET_ERROR;
  }

  CoreDXStaticClientInfo * client_info = static_cast<CoreDXStaticClientInfo *>(client->data);
  if (!client_info) {
    RMW_SET_ERROR_MSG("client info handle is null");
    return RMW_RET_ERROR;
  }
  const service_type_support_callbacks_t * callbacks = client_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }
  void * requester = client_info->requester_;
  if (!requester) {
    RMW_SET_ERROR_MSG("requester handle is null");
    return RMW_RET_ERROR;
  }

  *sequence_id = callbacks->send_request(requester, ros_request);
  return RMW_RET_OK;
}

/* ************************************************
 */
rmw_ret_t
rmw_take_response( const rmw_client_t * client,
                   rmw_request_id_t   * ros_request_header,
                  void               * ros_response,
                  bool               * taken )
{
  if (!client) {
    RMW_SET_ERROR_MSG("client handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client handle,
    client->implementation_identifier, toc_coredx_identifier,
    return RMW_RET_ERROR)

  if (!ros_request_header) {
    RMW_SET_ERROR_MSG("ros request header handle is null");
    return RMW_RET_ERROR;
  }
  if (!ros_response) {
    RMW_SET_ERROR_MSG("ros response handle is null");
    return RMW_RET_ERROR;
  }
  if (!taken) {
    RMW_SET_ERROR_MSG("taken handle is null");
    return RMW_RET_ERROR;
  }

  CoreDXStaticClientInfo * client_info =
    static_cast<CoreDXStaticClientInfo *>(client->data);
  if (!client_info) {
    RMW_SET_ERROR_MSG("client info handle is null");
    return RMW_RET_ERROR;
  }

  void * requester = client_info->requester_;
  if (!requester) {
    RMW_SET_ERROR_MSG("requester handle is null");
    return RMW_RET_ERROR;
  }

  const service_type_support_callbacks_t * callbacks = client_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }

  *taken = callbacks->take_response(requester, ros_request_header, ros_response);

  return RMW_RET_OK;
}

/* ************************************************
 */
rmw_service_t *
rmw_create_service( const rmw_node_t                    * node,
                    const rosidl_service_type_support_t * type_supports,
                    const char                          * service_name,
                    const rmw_qos_profile_t             * qos_profile )
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return NULL;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, toc_coredx_identifier,
    return NULL)

  const rosidl_service_type_support_t * type_support; 
  RMW_COREDX_EXTRACT_SERVICE_TYPESUPPORT(type_supports, type_support);

  if (!qos_profile) {
    RMW_SET_ERROR_MSG("qos_profile is null");
    return nullptr;
  }

  auto node_info = static_cast<CoreDXNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return NULL;
  }
  auto participant = static_cast<DDS::DomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return NULL;
  }

  const service_type_support_callbacks_t * callbacks =
    static_cast<const service_type_support_callbacks_t *>(type_support->data);
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return NULL;
  }

  // Past this point, a failure results in unrolling code in the goto fail block.
  DDS::DataReader * request_datareader = nullptr;
  DDS::ReadCondition * read_condition = nullptr;
  DDS::DataReaderQos datareader_qos;
  DDS::DataWriterQos datawriter_qos;
  void * replier = nullptr;
  void * buf = nullptr;
  CoreDXStaticServiceInfo * service_info = nullptr;
  rmw_service_t * service = nullptr;
  // Begin initializing elements.
  service = rmw_service_allocate();
  if (!service) {
    RMW_SET_ERROR_MSG("service handle is null");
    goto fail;
  }

  if (!get_datareader_qos(participant, qos_profile, datareader_qos)) {
    // error string was set within the function
    goto fail;
  }

  if (!get_datawriter_qos(participant, qos_profile, datawriter_qos)) {
    // error string was set within the function
    goto fail;
  }

  replier = callbacks->create_replier(
    participant, service_name, &datareader_qos, &datawriter_qos,
    reinterpret_cast<void **>(&request_datareader), &rmw_allocate);
  if (!replier) {
    RMW_SET_ERROR_MSG("failed to create replier");
    goto fail;
  }
  if (!request_datareader) {
    RMW_SET_ERROR_MSG("data reader handle is null");
    goto fail;
  }

  read_condition = request_datareader->create_readcondition(
     DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
  if (!read_condition) {
    RMW_SET_ERROR_MSG("failed to create read condition");
    goto fail;
  }

  buf = rmw_allocate(sizeof(CoreDXStaticServiceInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the CoreDXStaticServiceInfo in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(service_info, buf, goto fail, CoreDXStaticServiceInfo)
  buf = nullptr;  // Only free the service_info pointer; don't need the buf pointer anymore.
  service_info->replier_ = replier;
  service_info->callbacks_ = callbacks;
  service_info->request_datareader_ = request_datareader;
  service_info->read_condition_ = read_condition;
  service->implementation_identifier = toc_coredx_identifier;
  service->data = service_info;
  service->service_name = do_strdup( service_name );
  if (!service->service_name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node name");
    goto fail;
  }
  
  return service;
fail:
  if (service) {
    rmw_service_free(service);
  }
  if (request_datareader) {
    if (read_condition) {
      if (request_datareader->delete_readcondition(read_condition) != DDS_RETCODE_OK) {
        std::stringstream ss;
        ss << "leaking readcondition while handling failure at " <<
          __FILE__ << ":" << __LINE__ << '\n';
        (std::cerr << ss.str()).flush();
      }
    }
    DDS::Subscriber * sub = request_datareader->get_subscriber();
    if ((!sub) || (sub->delete_datareader(request_datareader) != RMW_RET_OK)) {
      std::stringstream ss;
      ss << "leaking datareader while handling failure at " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (service_info) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      service_info->~CoreDXStaticServiceInfo(), CoreDXStaticServiceInfo)
    rmw_free(service_info);
  }
  if (buf) {
    rmw_free(buf);
  }
  return NULL;
}

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

/* ************************************************
 */
rmw_ret_t
rmw_take_request( const rmw_service_t * service,
                  rmw_request_id_t    * ros_request_header,
                  void                * ros_request,
                  bool                * taken )
{
  if (!service) {
    RMW_SET_ERROR_MSG("service handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    service handle,
    service->implementation_identifier, toc_coredx_identifier,
    return RMW_RET_ERROR)

  if (!ros_request_header) {
    RMW_SET_ERROR_MSG("ros request header handle is null");
    return RMW_RET_ERROR;
  }
  if (!ros_request) {
    RMW_SET_ERROR_MSG("ros request handle is null");
    return RMW_RET_ERROR;
  }
  if (!taken) {
    RMW_SET_ERROR_MSG("taken handle is null");
    return RMW_RET_ERROR;
  }

  CoreDXStaticServiceInfo * service_info =
    static_cast<CoreDXStaticServiceInfo *>(service->data);
  if (!service_info) {
    RMW_SET_ERROR_MSG("service info handle is null");
    return RMW_RET_ERROR;
  }

  void * replier = service_info->replier_;
  if (!replier) {
    RMW_SET_ERROR_MSG("replier handle is null");
    return RMW_RET_ERROR;
  }

  const service_type_support_callbacks_t * callbacks = service_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }
  *taken = callbacks->take_request(replier, ros_request_header, ros_request);

  return RMW_RET_OK;
}

/* ************************************************
 */
rmw_ret_t
rmw_send_response( const rmw_service_t * service,
                  rmw_request_id_t    * ros_request_header,
                  void                * ros_response )
{
  if (!service) {
    RMW_SET_ERROR_MSG("service handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    service handle,
    service->implementation_identifier, toc_coredx_identifier,
    return RMW_RET_ERROR)

  if (!ros_request_header) {
    RMW_SET_ERROR_MSG("ros request header handle is null");
    return RMW_RET_ERROR;
  }
  if (!ros_response) {
    RMW_SET_ERROR_MSG("ros response handle is null");
    return RMW_RET_ERROR;
  }

  CoreDXStaticServiceInfo * service_info =
    static_cast<CoreDXStaticServiceInfo *>(service->data);
  if (!service_info) {
    RMW_SET_ERROR_MSG("service info handle is null");
    return RMW_RET_ERROR;
  }

  void * replier = service_info->replier_;
  if (!replier) {
    RMW_SET_ERROR_MSG("replier handle is null");
    return RMW_RET_ERROR;
  }

  const service_type_support_callbacks_t * callbacks = service_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }

  callbacks->send_response(replier, ros_request_header, ros_response);

  return RMW_RET_OK;
}

/* ************************************************
 */
rmw_guard_condition_t *
rmw_create_guard_condition( )
{
  rmw_guard_condition_t * guard_condition = rmw_guard_condition_allocate();
  if (!guard_condition) {
    RMW_SET_ERROR_MSG("failed to allocate guard condition");
    return NULL;
  }
  // Allocate memory for the DDS::GuardCondition object.
  DDS::GuardCondition * dds_guard_condition = nullptr;
  void * buf = rmw_allocate(sizeof(DDS::GuardCondition));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the DDS::GuardCondition in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(dds_guard_condition, buf, goto fail, DDS::GuardCondition)
  buf = nullptr;  // Only free the dds_guard_condition pointer; don't need the buf pointer anymore.
  guard_condition->implementation_identifier = toc_coredx_identifier;
  guard_condition->data = dds_guard_condition;
  return guard_condition;
fail:
  if (guard_condition) {
    rmw_guard_condition_free(guard_condition);
  }
  if (buf) {
    rmw_free(buf);
  }
  return NULL;
}

/* ************************************************
 */
rmw_ret_t
rmw_destroy_guard_condition( rmw_guard_condition_t * guard_condition )
{
  if (!guard_condition) {
    RMW_SET_ERROR_MSG("guard condition handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    guard condition handle,
    guard_condition->implementation_identifier, toc_coredx_identifier,
    return RMW_RET_ERROR)

  auto result = RMW_RET_OK;
  RMW_TRY_DESTRUCTOR( ((DDS::GuardCondition *)guard_condition->data)->~GuardCondition(),
                      DDS::GuardCondition, result = RMW_RET_ERROR)
  rmw_free(guard_condition->data);
  rmw_guard_condition_free(guard_condition);
  return result;
}

/* ************************************************
 */
rmw_ret_t
rmw_trigger_guard_condition( const rmw_guard_condition_t * guard_condition_handle )
{
  if (!guard_condition_handle) {
    RMW_SET_ERROR_MSG("guard condition handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    guard condition handle,
    guard_condition_handle->implementation_identifier, toc_coredx_identifier,
    return RMW_RET_ERROR);

  DDS::GuardCondition * guard_condition =
    static_cast<DDS::GuardCondition *>(guard_condition_handle->data);

  if (!guard_condition) {
    RMW_SET_ERROR_MSG("guard condition is null");
    return RMW_RET_ERROR;
  }
  DDS::ReturnCode_t status = guard_condition->set_trigger_value(1);
  if (status != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to set trigger value");
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
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
rmw_ret_t
rmw_wait( rmw_subscriptions_t    * subscriptions,
          rmw_guard_conditions_t * guard_conditions,
          rmw_services_t         * services,
          rmw_clients_t          * clients,
          rmw_waitset_t          * waitset,
          const rmw_time_t             * wait_timeout )
{
  return wait<CoreDXStaticSubscriberInfo, CoreDXStaticServiceInfo, CoreDXStaticClientInfo>
    (toc_coredx_identifier, subscriptions, guard_conditions, services, clients, waitset, wait_timeout);
}

/* ************************************************
 */
rmw_waitset_t *
rmw_create_waitset(size_t max_conditions)
{
  rmw_waitset_t     * waitset = rmw_waitset_allocate();
  CoreDXWaitSetInfo * waitset_info = nullptr;

  // From here onward, error results in unrolling in the goto fail block.
  if (!waitset) {
    RMW_SET_ERROR_MSG("failed to allocate waitset");
    goto fail;
  }
  waitset->implementation_identifier = toc_coredx_identifier;
  waitset->data = rmw_allocate(sizeof(CoreDXWaitSetInfo));
  waitset_info = static_cast<CoreDXWaitSetInfo *>(waitset->data);

  if (!waitset_info) {
    RMW_SET_ERROR_MSG("failed to allocate waitset");
    goto fail;
  }

  waitset_info->waitset = static_cast<DDS::WaitSet *>(rmw_allocate(sizeof(DDS::WaitSet)));
  if (!waitset_info->waitset) {
    RMW_SET_ERROR_MSG("failed to allocate waitset");
    goto fail;
  }

  RMW_TRY_PLACEMENT_NEW( waitset_info->waitset, waitset_info->waitset, goto fail, DDS::WaitSet, )

  // If max_conditions is greater than zero, re-allocate both ConditionSeqs to max_conditions
  if (max_conditions > 0) {
    waitset_info->active_conditions.resize(max_conditions);
    waitset_info->attached_conditions.resize(max_conditions);
  } 

  return waitset;

fail:
  if (waitset_info) {
    if (waitset_info->waitset) {
      RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(waitset_info->waitset->~WaitSet(), DDS::WaitSet)
      rmw_free(waitset_info->waitset);
    }
    waitset_info = nullptr;
  }
  if (waitset) {
    if (waitset->data) {
      rmw_free(waitset->data);
    }
    rmw_waitset_free(waitset);
  }
  return nullptr;
}

/* ************************************************
 */
rmw_ret_t
rmw_destroy_waitset(rmw_waitset_t * waitset)
{
  if (!waitset) {
    RMW_SET_ERROR_MSG("waitset handle is null");
    return RMW_RET_ERROR;
  }

  auto result = RMW_RET_OK;
  CoreDXWaitSetInfo * waitset_info = static_cast<CoreDXWaitSetInfo *>(waitset->data);

  // Explicitly call destructor since the "placement new" was used
  waitset_info->active_conditions.clear();
  waitset_info->attached_conditions.clear();
  if (waitset_info->waitset) {
    RMW_TRY_DESTRUCTOR(waitset_info->waitset->~WaitSet(), WaitSet, result = RMW_RET_ERROR)
    rmw_free(waitset_info->waitset);
  }
  waitset_info = nullptr;
  if (waitset->data) {
    rmw_free(waitset->data);
    waitset->data = nullptr;
  }
  if (waitset) {
    rmw_waitset_free(waitset);
  }
  return result;
}

/* ************************************************
 */
rmw_ret_t
rmw_get_topic_names_and_types(
	const rmw_node_t * node,
	rcutils_allocator_t * allocator,
	bool no_demangle,
	rmw_names_and_types_t * topic_names_and_types)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  if (node->implementation_identifier != toc_coredx_identifier) {
    RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }

  if (rmw_names_and_types_check_zero(topic_names_and_types) != RMW_RET_OK)
  {
    RMW_SET_ERROR_MSG("topic names and types is initialized");
    return RMW_RET_ERROR;
  }
  
  auto node_info = static_cast<CoreDXNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }
  if (!node_info->publisher_listener) {
    RMW_SET_ERROR_MSG("publisher listener handle is null");
    return RMW_RET_ERROR;
  }
  if (!node_info->subscriber_listener) {
    RMW_SET_ERROR_MSG("subscriber listener handle is null");
    return RMW_RET_ERROR;
  }

  // combine publisher and subscriber information
  std::map<std::string, std::set<std::string>> topics_with_multiple_types;
  for (auto it : node_info->publisher_listener->topic_names_and_types) {
    for (auto & jt : it.second) {
      topics_with_multiple_types[it.first].insert(jt);
    }
  }
  for (auto it : node_info->subscriber_listener->topic_names_and_types) {
    for (auto & jt : it.second) {
      topics_with_multiple_types[it.first].insert(jt);
    }
  }

  // ignore inconsistent types
  std::map<std::string, std::string> topics;
  for (auto & it : topics_with_multiple_types) {
    if (it.second.size() != 1) {
      fprintf(stderr, "topic type mismatch - ignoring topic '%s'\n", it.first.c_str());
      continue;
    }
    topics[it.first] = *it.second.begin();
  }

  // reformat type name
  std::string substr = "::msg::dds_::";
  for (auto & it : topics) {
    size_t substr_pos = it.second.find(substr);
    if (it.second[it.second.size() - 1] == '_' && substr_pos != std::string::npos) {
      it.second = it.second.substr(0, substr_pos) + "/" + it.second.substr(
        substr_pos + substr.size(), it.second.size() - substr_pos - substr.size() - 1);
    }
  }

  // copy data into result handle
  if (topics.size() > 0) {
	rmw_ret_t ret = rmw_names_and_types_init(topic_names_and_types, sizeof(char *) * topics.size(), allocator);
    if (ret != RMW_RET_OK) {
      RMW_SET_ERROR_MSG("failed to allocate memory for topic names and types")
      return RMW_RET_ERROR;
    }

	size_t i = 0;
    for (auto it : topics) {
      topic_names_and_types->names.data[i] = rcutils_strdup(it.first.c_str(), *allocator);
	  if (!topic_names_and_types->names.data[i]) {
		  RMW_SET_ERROR_MSG("failed to allocate memory for topic name")
			  goto fail;
	  }
	  rcutils_ret_t rcutils_ret = rcutils_string_array_init(
		  &topic_names_and_types->types[i],
		  it.second.size(),
		  allocator);

	  if (rcutils_ret != RCUTILS_RET_OK) {
		  RMW_SET_ERROR_MSG(rcutils_get_error_string_safe())
			  goto fail;
	  }

      topic_names_and_types->types[i].data[0] = rcutils_strdup(it.second.c_str(), *allocator);
     
      
      if (!topic_names_and_types->types[i].data[0]) {
        rmw_free(topic_names_and_types->names.data[i]);
        RMW_SET_ERROR_MSG("failed to allocate memory for type name")
        goto fail;
      }
      ++i;
    }
  }

  return RMW_RET_OK;
fail:
  rmw_ret_t ret = rmw_names_and_types_fini(topic_names_and_types);
  (void)ret;
  return RMW_RET_ERROR;
}

/* ************************************************
*/
rmw_ret_t
rmw_get_service_names_and_types(
	    const rmw_node_t * node,
		rcutils_allocator_t * allocator,
		rmw_names_and_types_t * service_names_and_types)
{
	if (!node) {
		RMW_SET_ERROR_MSG("node handle is null");
		return RMW_RET_ERROR;
	}
	if (node->implementation_identifier != toc_coredx_identifier) {
		RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
		return RMW_RET_ERROR;
	}

	if (rmw_names_and_types_check_zero(service_names_and_types) != RMW_RET_OK)
	{
		RMW_SET_ERROR_MSG("service names and types is initialized");
		return RMW_RET_ERROR;
	}

	auto node_info = static_cast<CoreDXNodeInfo *>(node->data);
	if (!node_info) {
		RMW_SET_ERROR_MSG("node info handle is null");
		return RMW_RET_ERROR;
	}
	
	RMW_SET_ERROR_MSG("get service names and types is not implemented!");

	return RMW_RET_ERROR;
}

/* ************************************************
 */
rmw_ret_t
rmw_get_node_names(
  const rmw_node_t * node,
  rcutils_string_array_t * node_names)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  if (node->implementation_identifier != toc_coredx_identifier) {
    RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  if (rmw_check_zero_rmw_string_array(node_names) != RMW_RET_OK) {
    return RMW_RET_ERROR;
  }
  
  DDS::DomainParticipant * participant = static_cast<CoreDXNodeInfo *>(node->data)->participant;
  DDS::InstanceHandleSeq handles;
  if (participant->get_discovered_participants(&handles) != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("unable to fetch discovered participants.");
    return RMW_RET_ERROR;
  }
  uint32_t length = handles.length() + 1;  // add myself
  node_names->size = length;
  node_names->data = static_cast<char **>(rmw_allocate(length * sizeof(char *)));

  DDS::DomainParticipantQos participant_qos;
  DDS::ReturnCode_t status = participant->get_qos(participant_qos);
  if (status != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get default participant qos");
    return RMW_RET_ERROR;
  }
  auto participant_name_length = strlen(participant_qos.entity_name.value) + 1;
  node_names->data[0] =
    static_cast<char *>(rmw_allocate(participant_name_length * sizeof(char)));
  snprintf(node_names->data[0], participant_name_length, "%s",
           participant_qos.entity_name.value);

  for (uint32_t i = 1; i < length; ++i) {
    DDS::ParticipantBuiltinTopicData pbtd;
    auto dds_ret = participant->get_discovered_participant_data(&pbtd, handles[i - 1]);
    char * name = pbtd.entity_name;
    if (!name || dds_ret != DDS_RETCODE_OK) {
      name = const_cast<char *>("(no name)");
    }
    size_t name_length = strlen(name) + 1;
    node_names->data[i] = static_cast<char *>(rmw_allocate(name_length * sizeof(char)));
    snprintf(node_names->data[i], name_length, "%s", name);
  }

  return RMW_RET_OK;
}

/* ************************************************
 */
rmw_ret_t
rmw_count_publishers( const rmw_node_t * node,
                      const char       * topic_name,
                      size_t           * count )
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  if (node->implementation_identifier != toc_coredx_identifier) {
    RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  if (!topic_name) {
    RMW_SET_ERROR_MSG("topic name is null");
    return RMW_RET_ERROR;
  }
  if (!count) {
    RMW_SET_ERROR_MSG("count handle is null");
    return RMW_RET_ERROR;
  }

  auto node_info = static_cast<CoreDXNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }
  if (!node_info->publisher_listener) {
    RMW_SET_ERROR_MSG("publisher listener handle is null");
    return RMW_RET_ERROR;
  }

  const auto & topic_names_and_types = node_info->publisher_listener->topic_names_and_types;
  auto it = topic_names_and_types.find(topic_name);
  if (it == topic_names_and_types.end()) {
    *count = 0;
  } else {
    *count = it->second.size();
  }
  return RMW_RET_OK;
}

/* ************************************************
 */
rmw_ret_t
rmw_count_subscribers( const rmw_node_t * node,
                       const char       * topic_name,
                       size_t           * count)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  if (node->implementation_identifier != toc_coredx_identifier) {
    RMW_SET_ERROR_MSG("node handle is not from this rmw implementation");
    return RMW_RET_ERROR;
  }
  if (!topic_name) {
    RMW_SET_ERROR_MSG("topic name is null");
    return RMW_RET_ERROR;
  }
  if (!count) {
    RMW_SET_ERROR_MSG("count handle is null");
    return RMW_RET_ERROR;
  }

  auto node_info = static_cast<CoreDXNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }
  if (!node_info->subscriber_listener) {
    RMW_SET_ERROR_MSG("subscriber listener handle is null");
    return RMW_RET_ERROR;
  }

  const auto & topic_names_and_types = node_info->subscriber_listener->topic_names_and_types;
  auto it = topic_names_and_types.find(topic_name);
  if (it == topic_names_and_types.end()) {
    *count = 0;
  } else {
    *count = it->second.size();
  }
  return RMW_RET_OK;
}

/* ************************************************
 */
rmw_ret_t
rmw_get_gid_for_publisher( const rmw_publisher_t * publisher,
                           rmw_gid_t             * gid )
{
  if (!publisher) {
    RMW_SET_ERROR_MSG("publisher is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    publisher handle,
    publisher->implementation_identifier,
    toc_coredx_identifier,
    return RMW_RET_ERROR)
  if (!gid) {
    RMW_SET_ERROR_MSG("gid is null");
    return RMW_RET_ERROR;
  }

  const CoreDXStaticPublisherInfo * publisher_info =
    static_cast<const CoreDXStaticPublisherInfo *>(publisher->data);
  if (!publisher_info) {
    RMW_SET_ERROR_MSG("publisher info handle is null");
    return RMW_RET_ERROR;
  }

  *gid = publisher_info->publisher_gid;
  return RMW_RET_OK;
}

/* ************************************************
 */
rmw_ret_t
rmw_compare_gids_equal( const rmw_gid_t * gid1,
                        const rmw_gid_t * gid2,
                        bool            * result )
{
  if (!gid1) {
    RMW_SET_ERROR_MSG("gid1 is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    gid1,
    gid1->implementation_identifier,
    toc_coredx_identifier,
    return RMW_RET_ERROR)
  if (!gid2) {
    RMW_SET_ERROR_MSG("gid2 is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    gid2,
    gid2->implementation_identifier,
    toc_coredx_identifier,
    return RMW_RET_ERROR)
  if (!result) {
    RMW_SET_ERROR_MSG("result is null");
    return RMW_RET_ERROR;
  }
  auto detail1 = reinterpret_cast<const CoreDXPublisherGID *>(gid1->data);
  if (!detail1) {
    RMW_SET_ERROR_MSG("gid1 is invalid");
    return RMW_RET_ERROR;
  }
  auto detail2 = reinterpret_cast<const CoreDXPublisherGID *>(gid2->data);
  if (!detail2) {
    RMW_SET_ERROR_MSG("gid2 is invalid");
    return RMW_RET_ERROR;
  }
  *result =  &detail1->publication_handle == &detail2->publication_handle;
  return RMW_RET_OK;
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
rmw_ret_t
_publisher_count_matched_subscriptions(DDS::DataWriter * datawriter, size_t * count)
{
  DDS::ReturnCode_t ret;
  DDS::PublicationMatchedStatus s;
  ret = datawriter->get_publication_matched_status(&s);
  if (ret != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get publication matched status");
    return RMW_RET_ERROR;
  }

#ifdef DISCOVERY_DEBUG_LOGGING
  using std::to_string;
  using std::stringstream;
  std::stringstream ss;
  // *INDENT-OFF* (prevent uncrustify from making unnecessary indents here)
  ss << "DDS_PublicationMatchedStatus:\n"
     << "  topic name:               " << datawriter->get_topic()->get_name() << "\n"
     << "  current_count:            " << to_string(s.current_count) << "\n"
     << "  current_count_change:     " << to_string(s.current_count_change) << "\n"
     << "  current_count_peak:       " << to_string(s.current_count_peak) << "\n"
     << "  total_count:              " << to_string(s.total_count) << "\n"
     << "  total_count_change:       " << to_string(s.total_count_change) << "\n"
     << "  last_subscription_handle: " << to_string(s.last_subscription_handle) << "\n";
  // *INDENT-ON*
  printf("%s", ss.str().c_str());
#endif

  *count = s.current_count;
  return RMW_RET_OK;
}

/* ************************************************
 */
rmw_ret_t
_subscription_count_matched_publishers(DDS::DataReader * datareader, size_t * count)
{
  DDS::ReturnCode_t ret;
  DDS::SubscriptionMatchedStatus s;
  ret = datareader->get_subscription_matched_status(&s);
  if (ret != DDS_RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get subscription matched status");
    return RMW_RET_ERROR;
  }

#ifdef DISCOVERY_DEBUG_LOGGING
  using std::to_string;
  using std::stringstream;
  std::stringstream ss;
  // *INDENT-OFF* (prevent uncrustify from making unnecessary indents here)
  ss << "DDS_SubscriptionMatchedStatus:\n"
     << "  topic name:               " << datareader->get_topicdescription()->get_name() << "\n"
     << "  current_count:            " << to_string(s.current_count) << "\n"
     << "  current_count_change:     " << to_string(s.current_count_change) << "\n"
     << "  current_count_peak:       " << to_string(s.current_count_peak) << "\n"
     << "  total_count:              " << to_string(s.total_count) << "\n"
     << "  total_count_change:       " << to_string(s.total_count_change) << "\n"
     << "  last_publication_handle:  " << to_string(s.last_publication_handle) << "\n";
  // *INDENT-ON*
  printf("%s", ss.str().c_str());
#endif

  *count = s.current_count;
  return RMW_RET_OK;
}

/* ************************************************
 */
rmw_ret_t
rmw_service_server_is_available(
  const rmw_node_t * node,
  const rmw_client_t * client,
  bool * is_available)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, toc_coredx_identifier,
    return RMW_RET_ERROR)
  if (!client) {
    RMW_SET_ERROR_MSG("client handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client handle,
    client->implementation_identifier, toc_coredx_identifier,
    return RMW_RET_ERROR)

  if (!is_available) {
    RMW_SET_ERROR_MSG("is_available is null");
    return RMW_RET_ERROR;
  }

  CoreDXStaticClientInfo * client_info =
    static_cast<CoreDXStaticClientInfo *>(client->data);
  if (!client_info) {
    RMW_SET_ERROR_MSG("client info handle is null");
    return RMW_RET_ERROR;
  }

  const service_type_support_callbacks_t * callbacks = client_info->callbacks_;
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return RMW_RET_ERROR;
  }
  void * requester = client_info->requester_;
  if (!requester) {
    RMW_SET_ERROR_MSG("requester handle is null");
    return RMW_RET_ERROR;
  }
  DDS::DataWriter * request_datawriter =
    static_cast<DDS::DataWriter *>(callbacks->get_request_datawriter(requester));
  const char * request_topic_name = request_datawriter->get_topic()->get_name();
  if (!request_topic_name) {
    RMW_SET_ERROR_MSG("could not get request topic name");
    return RMW_RET_ERROR;
  }

  *is_available = false;
  // In the CoreDX RPC implementation, a server is ready when:
  //   - At least one subscriber is matched to the request publisher.
  //   - At least one publisher is matched to the reponse subscription.
  size_t number_of_request_subscribers = 0;
  rmw_ret_t ret = _publisher_count_matched_subscriptions(
    request_datawriter, &number_of_request_subscribers);
  if (ret != RMW_RET_OK) {
    // error string already set
    return ret;
  }
#ifdef DISCOVERY_DEBUG_LOGGING
  printf("Checking for service server:\n");
  printf(" - %s: %zu\n",
    request_topic_name,
    number_of_request_subscribers);
#endif
  if (number_of_request_subscribers == 0) {
    // not ready
    return RMW_RET_OK;
  }

  size_t number_of_response_publishers = 0;
  ret = _subscription_count_matched_publishers(
    client_info->response_datareader_, &number_of_response_publishers);
  if (ret != RMW_RET_OK) {
    // error string already set
    return ret;
  }
#ifdef DISCOVERY_DEBUG_LOGGING
  printf(" - %s: %zu\n",
    client_info->response_datareader_->get_topicdescription()->get_name(),
    number_of_response_publishers);
#endif
  if (number_of_response_publishers == 0) {
    // not ready
    return RMW_RET_OK;
  }

  // all conditions met, there is a service server available
  *is_available = true;
  return RMW_RET_OK;
}

const rmw_guard_condition_t *
rmw_node_get_graph_guard_condition(const rmw_node_t * node)
{
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return nullptr;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, toc_coredx_identifier,
    return nullptr)

  CoreDXNodeInfo * node_info = static_cast<CoreDXNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return nullptr;
  }

  return node_info->graph_guard_condition;
}


// *INDENT-ON*
