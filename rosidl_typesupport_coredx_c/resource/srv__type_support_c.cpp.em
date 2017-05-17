// generated from rosidl_typesupport_coredx_c/resource/srv__type_support_c.cpp.em
// generated code does not contain a copyright notice

@#######################################################################
@# EmPy template for generating <srv>__type_support_c.cpp files
@#
@# Context:
@#  - spec (rosidl_parser.ServiceSpecification)
@#    Parsed specification of the .srv file
@#  - get_header_filename_from_msg_name (function)
@#######################################################################
@

#ifdef CoreDX_GLIBCXX_USE_CXX11_ABI_ZERO
#define _GLIBCXX_USE_CXX11_ABI 0
#endif

#include "@(spec.pkg_name)/srv/@(get_header_filename_from_msg_name(spec.srv_name))__rosidl_typesupport_coredx_c.h"

#include <dds/dds.hh>

#include <rosidl_typesupport_coredx_cpp/service_type_support.h>
#include <rosidl_typesupport_coredx_cpp/message_type_support.h>

#include "rmw/rmw.h"
#include "rmw/error_handling.h"
#include "rosidl_typesupport_cpp/service_type_support.hpp"
#include "rosidl_typesupport_coredx_c/identifier.h"

#include "@(spec.pkg_name)/msg/rosidl_typesupport_coredx_c__visibility_control.h"
@{req_header_file_name = get_header_filename_from_msg_name(spec.srv_name + '__request')}@
@{res_header_file_name = get_header_filename_from_msg_name(spec.srv_name + '__response')}@
#include "@(spec.pkg_name)/srv/@(req_header_file_name).h"
#include "@(spec.pkg_name)/srv/@(res_header_file_name).h"

#include "@(spec.pkg_name)/srv/dds_coredx/@(spec.srv_name)_Request_TypeSupport.hh"
#include "@(spec.pkg_name)/srv/@(get_header_filename_from_msg_name(spec.srv_name + '_Request'))__rosidl_typesupport_coredx_c.h"
#include "@(spec.pkg_name)/srv/dds_coredx/@(spec.srv_name)_Response_TypeSupport.hh"
#include "@(spec.pkg_name)/srv/@(get_header_filename_from_msg_name(spec.srv_name + '_Response'))__rosidl_typesupport_coredx_c.h"

// Re-use most of the functions from C++ typesupport
#include "@(spec.pkg_name)/srv/@(get_header_filename_from_msg_name(spec.srv_name))__rosidl_typesupport_coredx_cpp.hpp"

#if defined(__cplusplus)
extern "C"
{
#endif

// forward declare type support functions
const rosidl_message_type_support_t *
  ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_coredx_c, @(spec.pkg_name), srv, @(spec.srv_name)_Request)();
const rosidl_message_type_support_t *
  ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_coredx_c, @(spec.pkg_name), srv, @(spec.srv_name)_Response)();

void * create_requester__@(spec.srv_name)(
  void * untyped_participant,
  const char * service_name,
  const void * untyped_datareader_qos,
  const void * untyped_datawriter_qos,
  void ** untyped_reader,
  void * (*allocator)(size_t))
{
  return @(spec.pkg_name)::srv::typesupport_coredx_cpp::create_requester__@(spec.srv_name)(
    untyped_participant,
    service_name,
    untyped_datareader_qos,
    untyped_datawriter_qos,
    untyped_reader,
    allocator);
}
const char * destroy_requester__@(spec.srv_name)(
  void * untyped_requester,
  void (* deallocator)(void *))
{
  return @(spec.pkg_name)::srv::typesupport_coredx_cpp::destroy_requester__@(spec.srv_name)(
    untyped_requester, deallocator);
}

int64_t send_request__@(spec.srv_name)(
  void * untyped_requester,
  const void * untyped_ros_request)
{
  using RequesterType = DDS::rpc::Requester<
      @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
      @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  DDS::WriteSample<
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_> request;
  const rosidl_message_type_support_t * ts =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_coredx_c, @(spec.pkg_name), srv, @(spec.srv_name)_Request)();
  const message_type_support_callbacks_t * callbacks =
    static_cast<const message_type_support_callbacks_t *>(ts->data);
  bool converted = callbacks->convert_ros_to_dds(
    untyped_ros_request, static_cast<void *>(&request.data()));
  if (!converted) {
    fprintf(stderr, "Unable to convert request!\n");
    return -1;
  }

  RequesterType * requester = reinterpret_cast<RequesterType *>(untyped_requester);

  requester->send_request(request);
  int64_t sequence_number = ((int64_t)request.identity().seqnum.high) << 32 |
    request.identity().seqnum.low;
  return sequence_number;
}

void * create_replier__@(spec.srv_name)(
  void * untyped_participant,
  const char * service_name,
  const void * untyped_datareader_qos,
  const void * untyped_datawriter_qos,
  void ** untyped_reader,
  void * (*allocator)(size_t))
{
  return @(spec.pkg_name)::srv::typesupport_coredx_cpp::create_replier__@(spec.srv_name)(
    untyped_participant,
    service_name,
    untyped_datareader_qos,
    untyped_datawriter_qos,
    untyped_reader,
    allocator);
}

const char * destroy_replier__@(spec.srv_name)(
  void * untyped_replier,
  void (* deallocator)(void *))
{
  return @(spec.pkg_name)::srv::typesupport_coredx_cpp::destroy_replier__@(spec.srv_name)(
    untyped_replier, deallocator);
}

bool take_request__@(spec.srv_name)(
  void * untyped_replier,
  rmw_request_id_t * request_header,
  void * untyped_ros_request)
{
  using ReplierType = DDS::rpc::Replier<
      @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
      @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  if (!untyped_replier || !request_header || !untyped_ros_request) {
    return false;
  }

  ReplierType * replier = reinterpret_cast<ReplierType *>(untyped_replier);

  DDS::Sample<@(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_> request;
  bool taken = replier->take_request(request);
  if (!taken) {
    return false;
  }
  if (!request.info().valid_data) {
    return false;
  }

  const rosidl_message_type_support_t * ts =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_coredx_c, @(spec.pkg_name), srv, @(spec.srv_name)_Request)();
  const message_type_support_callbacks_t * callbacks =
    static_cast<const message_type_support_callbacks_t *>(ts->data);
  bool converted = callbacks->convert_dds_to_ros(
    static_cast<const void *>(&request.data()), untyped_ros_request);
  if (!converted) {
    return false;
  }

  size_t SAMPLE_IDENTITY_SIZE = 16;
  memcpy(&(request_header->writer_guid[0]), request.identity().guid.value, SAMPLE_IDENTITY_SIZE);

  request_header->sequence_number = ((int64_t)request.identity().seqnum.high) << 32 | request.identity().seqnum.low;
  return true;
}

bool take_response__@(spec.srv_name)(
  void * untyped_requester,
  rmw_request_id_t * request_header,
  void * untyped_ros_response)
{
  using RequesterType = DDS::rpc::Requester<@(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_, @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  if (!untyped_requester || !request_header || !untyped_ros_response) {
    return false;
  }

  RequesterType * requester = reinterpret_cast<RequesterType *>(untyped_requester);

  DDS::Sample<@(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_> response;
  bool received = requester->take_reply(response);
  if (!received) {
    return false;
  }
  if (!response.info().valid_data) {
    return false;
  }

  int64_t sequence_number =
    (((int64_t)response.related_identity().seqnum.high) << 32) |
    response.related_identity().seqnum.low;
  request_header->sequence_number = sequence_number;

  const rosidl_message_type_support_t * ts =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_coredx_c, @(spec.pkg_name), srv, @(spec.srv_name)_Response)();
  const message_type_support_callbacks_t * callbacks =
    static_cast<const message_type_support_callbacks_t *>(ts->data);
  bool converted = callbacks->convert_dds_to_ros(
    static_cast<const void *>(&response.data()), untyped_ros_response);
  return converted;
}

bool send_response__@(spec.srv_name)(
  void * untyped_replier,
  const rmw_request_id_t * request_header,
  const void * untyped_ros_response)
{
  using ReplierType = DDS::rpc::Replier<@(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_, @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  if (!untyped_replier || !request_header || !untyped_ros_response) {
    return false;
  }

  DDS::WriteSample<@(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_> response;
  const rosidl_message_type_support_t * ts =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_coredx_c, @(spec.pkg_name), srv, @(spec.srv_name)_Response)();
  const message_type_support_callbacks_t * callbacks =
    static_cast<const message_type_support_callbacks_t *>(ts->data);
  bool converted = callbacks->convert_ros_to_dds(
    untyped_ros_response, static_cast<void *>(&response.data()));
  if (!converted) {
    return false;
  }

  DDS::SampleIdentity_t request_identity;

  size_t SAMPLE_IDENTITY_SIZE = 16;
  memcpy(request_identity.guid.value, &request_header->writer_guid[0], SAMPLE_IDENTITY_SIZE);

  request_identity.seqnum.high = (int32_t)((request_header->sequence_number & 0xFFFFFFFF00000000) >> 32);
  request_identity.seqnum.low = (uint32_t)(request_header->sequence_number & 0xFFFFFFFF);

  ReplierType * replier = reinterpret_cast<ReplierType *>(untyped_replier);

  replier->send_reply(response, request_identity);
  return true;
}

void *
get_request_datawriter__@(spec.srv_name)(void * untyped_requester)
{
  return @(spec.pkg_name)::srv::typesupport_coredx_cpp::get_request_datawriter__@(spec.srv_name)(
    untyped_requester);
}

void *
get_reply_datareader__@(spec.srv_name)(void * untyped_requester)
{
  return @(spec.pkg_name)::srv::typesupport_coredx_cpp::get_reply_datareader__@(spec.srv_name)(
    untyped_requester);
}

void *
get_request_datareader__@(spec.srv_name)(void * untyped_replier)
{
  return @(spec.pkg_name)::srv::typesupport_coredx_cpp::get_request_datareader__@(spec.srv_name)(
    untyped_replier);
}

void *
get_reply_datawriter__@(spec.srv_name)(void * untyped_replier)
{
  return @(spec.pkg_name)::srv::typesupport_coredx_cpp::get_reply_datawriter__@(spec.srv_name)(
    untyped_replier);
}

static service_type_support_callbacks_t __callbacks = {
  "@(spec.pkg_name)",
  "@(spec.srv_name)",
  &create_requester__@(spec.srv_name),
  &destroy_requester__@(spec.srv_name),
  &create_replier__@(spec.srv_name),
  &destroy_replier__@(spec.srv_name),
  &send_request__@(spec.srv_name),
  &take_request__@(spec.srv_name),
  &send_response__@(spec.srv_name),
  &take_response__@(spec.srv_name),
  &get_request_datawriter__@(spec.srv_name),
  &get_reply_datareader__@(spec.srv_name),
  &get_request_datareader__@(spec.srv_name),
  &get_reply_datawriter__@(spec.srv_name),
};

static rosidl_service_type_support_t __type_support = {
  rosidl_typesupport_coredx_c__identifier,
  &__callbacks,
  get_service_typesupport_handle_function,
};


const rosidl_service_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_SYMBOL_NAME(rosidl_typesupport_coredx_c, @(spec.pkg_name), @(spec.srv_name))() {
  return &__type_support;
}

#if defined(__cplusplus)
}
#endif

