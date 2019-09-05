@# Included from rosidl_typesupport_coredx_cpp/resource/idl__rosidl_typesupport_coredx_cpp.hpp.em
@{
from rosidl_cmake import convert_camel_case_to_lower_case_underscore
include_parts = [package_name] + list(interface_path.parents[0].parts)
include_base = '/'.join(include_parts)
header_filename = convert_camel_case_to_lower_case_underscore(interface_path.stem)
header_files = [
    'rosidl_generator_c/message_type_support_struct.h',
    'rosidl_typesupport_interface/macros.h',
    package_name + '/msg/rosidl_typesupport_coredx_cpp__visibility_control.h',
    include_base + '/' + header_filename + '__struct.hpp'
]
dds_specific_header_files = [
    include_base + '/dds_coredx/' + interface_path.stem + '_TypeSupport.hh'
]
}@

@[for header_file in header_files]@
@[    if header_file in include_directives]@
// already included above
// @
@[    else]@
@{include_directives.add(header_file)}@
@[    end if]@
#include "@(header_file)"
@[end for]@

@[for header_file in dds_specific_header_files]@
@[    if header_file in include_directives]@
// already included above
// @
@[    else]@
@{include_directives.add(header_file)}@
@[    end if]@
#include "@(header_file)"
@[end for]@

@[for ns in message.structure.namespaced_type.namespaces]@

namespace @(ns)
{
@[end for]@
@{
__ros_msg_pkg_prefix = '::'.join(message.structure.namespaced_type.namespaces)
__ros_msg_type = __ros_msg_pkg_prefix + '::' + message.structure.namespaced_type.name
__dds_msg_type_prefix = __ros_msg_pkg_prefix + '::dds_::' + message.structure.namespaced_type.name
__dds_msg_type = __dds_msg_type_prefix + '_'
}@
namespace typesupport_coredx_cpp
{

bool
ROSIDL_TYPESUPPORT_COREDX_CPP_PUBLIC_@(package_name)
convert_ros_message_to_dds(
  const @(__ros_msg_type) & ros_message,
  @(__dds_msg_type) & dds_message);

bool
ROSIDL_TYPESUPPORT_COREDX_CPP_PUBLIC_@(package_name)
convert_dds_message_to_ros(
  const @(__dds_msg_type) & dds_message,
  @(__ros_msg_type) & ros_message);

// TODO: replace these with appropriate declarations if required....
#if 0
DDS_TypeCode *
get_type_code__@(message.structure.namespaced_type.name)();

bool
to_cdr_stream__@(message.structure.namespaced_type.name)(
  const void * untyped_ros_message,
  void * cdr_stream);

bool
to_message__@(message.structure.namespaced_type.name)(
  const void  * cdr_stream,
  void * untyped_ros_message);
#endif

}  // namespace typesupport_coredx_cpp

@[for ns in reversed(message.structure.namespaced_type.namespaces)]@
}  // namespace @(ns)

@[end for]@

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_COREDX_CPP_PUBLIC_@(package_name)
const rosidl_message_type_support_t *
  ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
  rosidl_typesupport_coredx_cpp,
  @(', '.join([package_name] + list(interface_path.parents[0].parts))),
  @(message.structure.namespaced_type.name))();

#ifdef __cplusplus
}
#endif

