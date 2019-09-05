@# Included from rosidl_typesupport_coredx_c/resource/idl__dds_coredx__type_support_c.cpp.em
@{
from rosidl_cmake import convert_camel_case_to_lower_case_underscore
include_parts = [package_name] + list(interface_path.parents[0].parts)
include_base = '/'.join(include_parts)
cpp_include_prefix = interface_path.stem

c_include_prefix = convert_camel_case_to_lower_case_underscore(cpp_include_prefix)

header_files = [
    include_base + '/' + c_include_prefix + '__rosidl_typesupport_coredx_c.h',
    'rosidl_typesupport_coredx_cpp/service_type_support.h',
    'rosidl_typesupport_coredx_cpp/message_type_support.h',
    'rmw/rmw.h',
    'rosidl_typesupport_cpp/service_type_support.hpp',
    'rosidl_typesupport_coredx_c/identifier.h',
    package_name + '/msg/rosidl_typesupport_coredx_c__visibility_control.h',
    include_base + '/dds_coredx/' + cpp_include_prefix + '_TypeSupport.hh',
    include_base + '/' + c_include_prefix + '.h',
# Re-use most of the functions from C++ typesupport
    include_base + '/' + c_include_prefix + '__rosidl_typesupport_coredx_cpp.hpp',
]

dds_specific_header_files = [
    'dds/dds.hh',
    'dds/request_reply.hh'
]
}@
#ifdef CoreDX_GLIBCXX_USE_CXX11_ABI_ZERO
#define _GLIBCXX_USE_CXX11_ABI 0
#endif

@[for header_file in dds_specific_header_files]@
@[    if header_file in include_directives]@
// already included above
// @
@[    else]@
@{include_directives.add(header_file)}@
@[    end if]@
#include <@(header_file)>
@[end for]@

@[for header_file in header_files]@
@[    if header_file in include_directives]@
// already included above
// @
@[    else]@
@{include_directives.add(header_file)}@
@[    end if]@
#include "@(header_file)"
@[end for]@

@{
TEMPLATE(
    'msg__type_support_c.cpp.em',
    package_name=package_name, interface_path=interface_path,
    message=service.request_message,
    include_directives=include_directives
)
}@

@{
TEMPLATE(
    'msg__type_support_c.cpp.em',
    package_name=package_name, interface_path=interface_path,
    message=service.response_message,
    include_directives=include_directives
)
}@

#ifdef __cplusplus
extern "C"
{
#endif
@{
__ros_srv_pkg_prefix = '::'.join(service.namespaced_type.namespaces)
__ros_request_msg_type = __ros_srv_pkg_prefix + '::' + service.request_message.structure.namespaced_type.name
__ros_response_msg_type = __ros_srv_pkg_prefix + '::' + service.response_message.structure.namespaced_type.name
__dds_request_msg_type = __ros_srv_pkg_prefix + '::dds_::' + service.request_message.structure.namespaced_type.name + '_'
__dds_response_msg_type = __ros_srv_pkg_prefix + '::dds_::' + service.response_message.structure.namespaced_type.name + '_'
}@

// forward declare type support functions
const rosidl_message_type_support_t *
  ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
    rosidl_typesupport_coredx_c,
    @(', '.join(service.request_message.structure.namespaced_type.namespaces)),
    @(service.request_message.structure.namespaced_type.name))();
const rosidl_message_type_support_t *
  ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
    rosidl_typesupport_coredx_c,
    @(', '.join(service.response_message.structure.namespaced_type.namespaces)),
    @(service.response_message.structure.namespaced_type.name))();

void * create_requester__@(service.namespaced_type.name)(
  void * untyped_participant,
  const char * service_name,
  const char * req_topic_name,
  const char * rep_topic_name,
  const void * untyped_datareader_qos,
  const void * untyped_datawriter_qos,
  void ** untyped_reader,
  void ** untyped_writer,
  void * (*allocator)(size_t))
{
  return @('::'.join(service.namespaced_type.namespaces))::typesupport_coredx_cpp::create_requester__@(service.namespaced_type.name)(
    untyped_participant,
    service_name,
    req_topic_name,
    rep_topic_name,
    untyped_datareader_qos,
    untyped_datawriter_qos,
    untyped_reader,
    untyped_writer,
    allocator);
}
const char * destroy_requester__@(service.namespaced_type.name)(
  void * untyped_requester,
  void (*deallocator)(void *))
{
  return @('::'.join(service.namespaced_type.namespaces))::typesupport_coredx_cpp::destroy_requester__@(service.namespaced_type.name)(
    untyped_requester, deallocator);
}

int64_t send_request__@(service.namespaced_type.name)(
  void * untyped_requester,
  const void * untyped_ros_request)
{
  using RequesterType = DDS::rpc::Requester<
    @(__dds_request_msg_type),
    @(__dds_response_msg_type)>;
  DDS::WriteSample<@(__dds_request_msg_type)> request;
  const rosidl_message_type_support_t * ts =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
      rosidl_typesupport_coredx_c,
      @(', '.join(service.request_message.structure.namespaced_type.namespaces)),
      @(service.request_message.structure.namespaced_type.name))();
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

void * create_replier__@(service.namespaced_type.name)(
  void * untyped_participant,
  const char * service_name,
  const char * req_topic_name,
  const char * rep_topic_name,
  const void * untyped_datareader_qos,
  const void * untyped_datawriter_qos,
  void ** untyped_reader,
  void ** untyped_writer,
  void * (*allocator)(size_t))
{
  return @('::'.join(service.namespaced_type.namespaces))::typesupport_coredx_cpp::create_replier__@(service.namespaced_type.name)(
    untyped_participant,
    service_name,
    req_topic_name,
    rep_topic_name,
    untyped_datareader_qos,
    untyped_datawriter_qos,
    untyped_reader,
    untyped_writer,
    allocator);
}

const char * destroy_replier__@(service.namespaced_type.name)(
  void * untyped_replier,
  void (*deallocator)(void *))
{
  return @('::'.join(service.namespaced_type.namespaces))::typesupport_coredx_cpp::destroy_replier__@(service.namespaced_type.name)(
    untyped_replier, deallocator);
}

bool take_request__@(service.namespaced_type.name)(
  void * untyped_replier,
  rmw_request_id_t * request_header,
  void * untyped_ros_request)
{
  using ReplierType = DDS::rpc::Replier<
    @(__dds_request_msg_type),
    @(__dds_response_msg_type)>;
  if (!untyped_replier || !request_header || !untyped_ros_request) {
    return false;
  }

  ReplierType * replier = reinterpret_cast<ReplierType *>(untyped_replier);

  DDS::Sample<@(__dds_request_msg_type)> request;
  bool taken = replier->take_request(request);
  if (!taken) {
    return false;
  }
  if (!request.info().valid_data) {
    return false;
  }

  const rosidl_message_type_support_t * ts =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
    rosidl_typesupport_coredx_c,
    @(', '.join(service.request_message.structure.namespaced_type.namespaces)),
    @(service.request_message.structure.namespaced_type.name))();
  const message_type_support_callbacks_t * callbacks =
    static_cast<const message_type_support_callbacks_t *>(ts->data);
  bool converted = callbacks->convert_dds_to_ros(
    static_cast<const void *>(&request.data()), untyped_ros_request);
  if (!converted) {
    throw std::runtime_error("failed to convert request to ros");
    return false;
  }

  size_t SAMPLE_IDENTITY_SIZE = 16;
  memcpy(&(request_header->writer_guid[0]), request.identity().guid.value, SAMPLE_IDENTITY_SIZE);

  request_header->sequence_number = ((int64_t)request.identity().seqnum.high) << 32 | request.identity().seqnum.low;
  return true;
}

bool take_response__@(service.namespaced_type.name)(
  void * untyped_requester,
  rmw_request_id_t * request_header,
  void * untyped_ros_response)
{
  using RequesterType = DDS::rpc::Requester<
    @(__dds_request_msg_type),
    @(__dds_response_msg_type)>;
  if (!untyped_requester || !request_header || !untyped_ros_response) {
    return false;
  }

  RequesterType * requester = reinterpret_cast<RequesterType *>(untyped_requester);

  DDS::Sample<@(__dds_response_msg_type)> response;
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
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
      rosidl_typesupport_coredx_c,
      @(', '.join(service.response_message.structure.namespaced_type.namespaces)),
      @(service.response_message.structure.namespaced_type.name))();
  const message_type_support_callbacks_t * callbacks =
    static_cast<const message_type_support_callbacks_t *>(ts->data);
  bool converted = callbacks->convert_dds_to_ros(
    static_cast<const void *>(&response.data()), untyped_ros_response);
  if ( !converted )
    throw std::runtime_error("failed to convert response to ros");

  return converted;
}

bool send_response__@(service.namespaced_type.name)(
  void * untyped_replier,
  const rmw_request_id_t * request_header,
  const void * untyped_ros_response)
{
  using ReplierType = DDS::rpc::Replier<
    @(__dds_request_msg_type),
    @(__dds_response_msg_type)>;  
  if (!untyped_replier || !request_header || !untyped_ros_response) {
    return false;
  }

  DDS::WriteSample<@(__dds_response_msg_type)> response;
  const rosidl_message_type_support_t * ts =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
      rosidl_typesupport_coredx_c,
      @(', '.join(service.response_message.structure.namespaced_type.namespaces)),
      @(service.response_message.structure.namespaced_type.name))();
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
get_request_datawriter__@(service.namespaced_type.name)(void * untyped_requester)
{
  return @('::'.join(service.namespaced_type.namespaces))::typesupport_coredx_cpp::get_request_datawriter__@(service.namespaced_type.name)(
    untyped_requester);
}

void *
get_reply_datareader__@(service.namespaced_type.name)(void * untyped_requester)
{
  return @('::'.join(service.namespaced_type.namespaces))::typesupport_coredx_cpp::get_reply_datareader__@(service.namespaced_type.name)(
    untyped_requester);
}

void *
get_request_datareader__@(service.namespaced_type.name)(void * untyped_replier)
{
  return @('::'.join(service.namespaced_type.namespaces))::typesupport_coredx_cpp::get_request_datareader__@(service.namespaced_type.name)(
    untyped_replier);
}

void *
get_reply_datawriter__@(service.namespaced_type.name)(void * untyped_replier)
{
  return @('::'.join(service.namespaced_type.namespaces))::typesupport_coredx_cpp::get_reply_datawriter__@(service.namespaced_type.name)(
    untyped_replier);
}

static service_type_support_callbacks_t _@(service.namespaced_type.name)__callbacks = {
  "@('::'.join([package_name] + list(interface_path.parents[0].parts)))",  // service_namespace
  "@(service.namespaced_type.name)",
  &create_requester__@(service.namespaced_type.name),
  &destroy_requester__@(service.namespaced_type.name),
  &create_replier__@(service.namespaced_type.name),
  &destroy_replier__@(service.namespaced_type.name),
  &send_request__@(service.namespaced_type.name),
  &take_request__@(service.namespaced_type.name),
  &send_response__@(service.namespaced_type.name),
  &take_response__@(service.namespaced_type.name),
  &get_request_datawriter__@(service.namespaced_type.name),
  &get_reply_datareader__@(service.namespaced_type.name),
  &get_request_datareader__@(service.namespaced_type.name),
  &get_reply_datawriter__@(service.namespaced_type.name),
};

static rosidl_service_type_support_t _@(service.namespaced_type.name)__type_support = {
  rosidl_typesupport_coredx_c__identifier,
  &_@(service.namespaced_type.name)__callbacks,
  get_service_typesupport_handle_function,
};


const rosidl_service_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_SYMBOL_NAME(
  rosidl_typesupport_coredx_c,
  @(', '.join([package_name] + list(interface_path.parents[0].parts))),
  @(service.namespaced_type.name))() {
  return &_@(service.namespaced_type.name)__type_support;
}

#ifdef __cplusplus
}
#endif
