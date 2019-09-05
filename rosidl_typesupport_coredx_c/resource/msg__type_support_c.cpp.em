@# Included from rosidl_typesupport_coredx_c/resource/idl__dds_coredx__type_support_c.cpp.em
@{
from rosidl_cmake import convert_camel_case_to_lower_case_underscore
from rosidl_generator_c import idl_structure_type_to_c_include_prefix
from rosidl_generator_c import idl_structure_type_to_c_typename
from rosidl_generator_c import idl_type_to_c
from rosidl_parser.definition import AbstractNestedType
from rosidl_parser.definition import AbstractSequence
from rosidl_parser.definition import AbstractString
from rosidl_parser.definition import AbstractWString
from rosidl_parser.definition import Array
from rosidl_parser.definition import BasicType
from rosidl_parser.definition import BoundedSequence
from rosidl_parser.definition import NamespacedType
include_parts = [package_name] + list(interface_path.parents[0].parts)
include_base = '/'.join(include_parts)

cpp_include_prefix = interface_path.stem
c_include_prefix = convert_camel_case_to_lower_case_underscore(cpp_include_prefix)

header_files = [
    include_base + '/' + c_include_prefix + '__rosidl_typesupport_coredx_c.h',
    'rcutils/types/uint8_array.h',
    'rosidl_typesupport_coredx_c/identifier.h',
    'rosidl_typesupport_coredx_c/wstring_conversion.hpp',
    'rosidl_typesupport_coredx_cpp/message_type_support.h',
    package_name + '/msg/rosidl_typesupport_coredx_c__visibility_control.h',
    include_base + '/' + c_include_prefix + '__struct.h',
    include_base + '/' + c_include_prefix + '__functions.h',
]

dds_specific_header_files = [
    include_base + '/dds_coredx/' + cpp_include_prefix + '_TypeSupport.hh',
    'new'
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

#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-parameter"
# ifdef __clang__
#  pragma clang diagnostic ignored "-Wdeprecated-register"
#  pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
# endif
#endif

@[for header_file in dds_specific_header_files]@
@[    if header_file in include_directives]@
// already included above
// @
@[    else]@
@{include_directives.add(header_file)}@
@[    end if]@
#include "@(header_file)"
@[end for]@

#ifndef _WIN32
# pragma GCC diagnostic pop
#endif

// includes and forward declarations of message dependencies and their conversion functions
@# // Include the message header for each non-primitive field.
#ifdef __cplusplus
extern "C"
{
#endif

@{
from collections import OrderedDict
includes = OrderedDict()
for member in message.structure.members:
    keys = set([])
    if isinstance(member.type, AbstractNestedType) and isinstance(member.type.value_type, BasicType):
       includes.setdefault('rosidl_generator_c/primitives_sequence.h', []).append(member.name)
       includes.setdefault('rosidl_generator_c/primitives_sequence_functions.h', []).append(member.name)
    else:
        type_ = member.type
        if isinstance(type_, AbstractNestedType):
            type_ = type_.value_type
        if isinstance(type_, AbstractString):
           includes.setdefault('rosidl_generator_c/string.h', []).append(member.name)
           includes.setdefault('rosidl_generator_c/string_functions.h', []).append(member.name)
        elif isinstance(type_, AbstractWString):
           includes.setdefault('rosidl_generator_c/u16string.h', []).append(member.name)
           includes.setdefault('rosidl_generator_c/u16string_functions.h', []).append(member.name)
        elif isinstance(type_, NamespacedType):
           include_prefix = idl_structure_type_to_c_include_prefix(type_)
           if include_prefix.endswith('__request'):
              include_prefix = include_prefix[:-9]
           elif include_prefix.endswith('__response'):
              include_prefix = include_prefix[:-10]
           if include_prefix.endswith('__goal'):
              include_prefix = include_prefix[:-6]
           elif include_prefix.endswith('__result'):
              include_prefix = include_prefix[:-8]
           elif include_prefix.endswith('__feedback'):
              include_prefix = include_prefix[:-10]
           includes.setdefault(include_prefix + '__struct.h', []).append(member.name)
           includes.setdefault(include_prefix + '__functions.h', []).append(member.name)
}@
@[if includes]@
// Include directives for member types
@[    for header_file, member_names in includes.items()]@
@[        for member_name in member_names]@
// Member '@(member_name)'
@[        end for]@
@[        if header_file in include_directives]@
// already included above
// @
@[        else]@
@{include_directives.add(header_file)}@
@[        end if]@
#include "@(header_file)"
@[    end for]@
@[end if]@

// forward declare type support functions
@{
forward_declares = {}
for member in message.structure.members:
    _type = member.type
    if isinstance(_type, AbstractNestedType):
       _type = member.type.value_type

    if isinstance(_type, NamespacedType):
        key = (*_type.namespaces, _type.name)
        if key not in forward_declares:
            forward_declares[key] = set([])
        forward_declares[key].add(member.name)
}@
@[for key in sorted(forward_declares.keys())]@
@[  if key[0] != package_name]@
ROSIDL_TYPESUPPORT_COREDX_C_IMPORT_@(package_name)
@[  end if]@
const rosidl_message_type_support_t *
  ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
  rosidl_typesupport_coredx_c, @(', '.join(key)))();
@[end for]@

@# // Make callback functions specific to this message type.
@{
__ros_c_msg_type = '__'.join(message.structure.namespaced_type.namespaced_name())
__dds_cpp_msg_type_prefix = '::'.join(message.structure.namespaced_type.namespaces + ['dds_', message.structure.namespaced_type.name])
__dds_cpp_msg_type = __dds_cpp_msg_type_prefix + '_'
}@

static bool
_@(message.structure.namespaced_type.name)__register_type(void * untyped_participant, const char * type_name)
{
  if (!untyped_participant) {
    fprintf(stderr, "untyped participant handle is null\n");
    return false;
  }
  if (!type_name) {
    fprintf(stderr, "type name handle is null\n");
    return false;
  }
  DDS::DomainParticipant * participant = static_cast<DDS::DomainParticipant *>(untyped_participant);

  DDS::ReturnCode_t status =
    @(__dds_cpp_msg_type_prefix)_TypeSupport::register_type(participant, type_name);
  switch (status) {
    case DDS::RETCODE_ERROR:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_TypeSupport::register_type: "
        "an internal error has occurred\n");
      return false;
    case DDS::RETCODE_BAD_PARAMETER:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_TypeSupport::register_type: "
        "bad domain participant or type name parameter\n");
      return false;
    case DDS::RETCODE_OUT_OF_RESOURCES:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_TypeSupport::register_type: "
        "out of resources\n");
      return false;
    case DDS::RETCODE_PRECONDITION_NOT_MET:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_TypeSupport::register_type: "
        "already registered with a different TypeSupport class\n");
      return false;
    case DDS::RETCODE_OK:
      return true;
    default:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_TypeSupport::register_type: unknown return code\n");
  }
  return false;
}

static bool
_@(message.structure.namespaced_type.name)__convert_ros_to_dds(const void * untyped_ros_message, void * untyped_dds_message)
{
  if (!untyped_ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }
  if (!untyped_dds_message) {
    fprintf(stderr, "dds message handle is null\n");
    return false;
  }
  const @(__ros_c_msg_type) * ros_message = static_cast<const @(__ros_c_msg_type) *>(untyped_ros_message);
  @(__dds_cpp_msg_type) * dds_message = static_cast<@(__dds_cpp_msg_type) *>(untyped_dds_message);
@[if not message.structure.members]@
  // No fields is a no-op.
  (void)dds_message;
  (void)ros_message;
@[end if]@
@[for member in message.structure.members]@
  // Member name: @(member.name)
  {
@{
type_ = member.type
if isinstance(type_, AbstractNestedType):
    type_ = type_.value_type
}@
@[  if isinstance(type_, NamespacedType)]@
    const message_type_support_callbacks_t * @('__'.join(type_.namespaced_name()))__callbacks =
      static_cast<const message_type_support_callbacks_t *>(
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_coredx_c, @(', '.join(type_.namespaced_name()))
      )()->data);
@[  end if]@
@[  if isinstance(member.type, AbstractNestedType)]@
@# //-------------------------------------------  ARRAY || SEQUENCE
@[    if isinstance(member.type, Array)]@
    size_t size = @(member.type.size);
@[    else]@
    size_t size = ros_message->@(member.name).size;
    if (size > (std::numeric_limits<int32_t>::max)()) {
      fprintf(stderr, "array size exceeds maximum DDS sequence size\n");
      return false;
    }
@[      if isinstance(member.type, BoundedSequence)]@
    if (size > @(member.type.maximum_size)) {
      fprintf(stderr, "array size exceeds upper bound\n");
      return false;
    }
@[      end if]@
    uint32_t length = static_cast<int32_t>(size);
    if (!dds_message->@(member.name)_.resize(length)) {
      fprintf(stderr, "failed to set length of sequence\n");
      return false;
    }
@[    end if]@
    for (int32_t i = 0; i < static_cast<int32_t>(size); ++i) {
@[    if isinstance(member.type, Array)]@
      auto & ros_i = ros_message->@(member.name)[i];
@[    else]@
      auto & ros_i = ros_message->@(member.name).data[i];
@[    end if]@
@[    if isinstance(type_, AbstractString)]@
@#    //------------------------------------------- STRING
      const rosidl_generator_c__String * str = &ros_i;
      if (str->capacity == 0 || str->capacity <= str->size) {
        fprintf(stderr, "string capacity not greater than size\n");
        return false;
      }
      if (str->data[str->size] != '\0') {
        fprintf(stderr, "string not null-terminated\n");
        return false;
      }
@[      if type_.has_maximum_size()]@
@[        if isinstance(member.type, Array) ]@
      // array of bounded strings
      strncpy(dds_message->@(member.name)_[static_cast<int32_t>(i)], str->data, @(type_.maximum_size)); 
      dds_message->@(member.name)_[static_cast<int32_t>(i)][static_cast<int32_t>(@(type_.maximum_size))] = '\0';
@[        else]@
      // sequence of bounded string
      strncpy(dds_message->@(member.name)_[static_cast<int32_t>(i)].value, str->data, @(type_.maximum_size)); 
      dds_message->@(member.name)_[static_cast<int32_t>(i)].value[static_cast<int32_t>(@(type_.maximum_size))] = '\0';
@[        end if]@
@[      else]@
      dds_message->@(member.name)_[static_cast<int32_t>(i)] = DDS::String_dup(str->data);
@[      end if]@
@[    elif isinstance(type_, AbstractWString)]@
@#    //------------------------------------------- WSTRING
      const rosidl_generator_c__U16String * str = &ros_i;
      if (str->capacity == 0 || str->capacity <= str->size) {
        fprintf(stderr, "string capacity not greater than size\n");
        return false;
      }
      if (str->data[str->size] != u'\0') {
        fprintf(stderr, "string not null-terminated\n");
        return false;
      }
@[      if type_.has_maximum_size()]@
@[        if isinstance(member.type, Array) ]@
      // array of bounded wstrings
      rosidl_typesupport_coredx_c::copy_wstring_from_u16string(dds_message->@(member.name)_[static_cast<int32_t>(i)], str, @(type_.maximum_size));
@[        else]@
      // sequence of bounded wstrings
      rosidl_typesupport_coredx_c::copy_wstring_from_u16string(dds_message->@(member.name)_[static_cast<int32_t>(i)].value, str, @(type_.maximum_size));
@[        end if]@
@[      else]@
      cdx_char32_t * wstr = rosidl_typesupport_coredx_c::create_wstring_from_u16string(*str);
      if (NULL == wstr) {
        fprintf(stderr, "failed to create wstring from u16string\n");
        return false;
      }
      dds_message->@(member.name)_[static_cast<int32_t>(i)] = wstr;
@[      end if]@
@[    elif isinstance(type_, BasicType)]@
@#    //------------------------------------------- BASIC
@[      if type_.typename == 'boolean']@
      dds_message->@(member.name)_[i] = ros_i ? 1 : 0;
@[      else]@
      dds_message->@(member.name)_[i] = ros_i;
@[      end if]@
@[    else]@
@#    //------------------------------------------- COMPLEX
      if (!@(idl_structure_type_to_c_typename(type_))__callbacks->convert_ros_to_dds(
          &ros_i, &dds_message->@(member.name)_[i]))
      {
        return false;
      }
@[    end if]@
    }
@[  elif isinstance(member.type, AbstractString)]@
@# //------------------------------------------- STRING
    const rosidl_generator_c__String * str = &ros_message->@(member.name);
    if (str->capacity == 0 || str->capacity <= str->size) {
      fprintf(stderr, "string capacity not greater than size\n");
      return false;
    }
    if (str->data[str->size] != '\0') {
      fprintf(stderr, "string not null-terminated\n");
      return false;
    }
@[    if member.type.has_maximum_size()]@
    // bounded string
    strncpy(dds_message->@(member.name)_, str->data, @(member.type.maximum_size));
    dds_message->@(member.name)_[static_cast<int32_t>(@(member.type.maximum_size))] = '\0';
@[    else]@
    // unbounded string
    DDS::String_free(dds_message->@(member.name)_);
    dds_message->@(member.name)_ = DDS::String_dup(str->data);
@[    end if]@
@[  elif isinstance(member.type, AbstractWString)]@
@# //------------------------------------------- WSTRING
    const rosidl_generator_c__U16String * str = &ros_message->@(member.name);
    if (str->capacity == 0 || str->capacity <= str->size) {
      fprintf(stderr, "string capacity not greater than size\n");
      return false;
    }
    if (str->data[str->size] != u'\0') {
      fprintf(stderr, "string not null-terminated\n");
      return false;
    }
@[    if member.type.has_maximum_size()]@
    // bounded wstring
    rosidl_typesupport_coredx_c::copy_wstring_from_u16string(dds_message->@(member.name)_, str, @(member.type.maximum_size) );
@[    else]@
    // unbounded wstring
    cdx_char32_t * wstr = rosidl_typesupport_coredx_c::create_wstring_from_u16string(*str);
    if (NULL == wstr) {
      fprintf(stderr, "failed to create wstring from u16string\n");
      return false;
    }
    delete[] dds_message->@(member.name)_;
    dds_message->@(member.name)_ = wstr;
@[    end if]@

@[  elif isinstance(member.type, BasicType)]@
@# //------------------------------------------- BASIC
    dds_message->@(member.name)_ = ros_message->@(member.name);
@# //------------------------------------------- COMPLEX
@[  else]@
    if (!@(idl_structure_type_to_c_typename(member.type))__callbacks->convert_ros_to_dds(
        &ros_message->@(member.name), &dds_message->@(member.name)_))
    {
      return false;
    }
@[  end if]@
  }
@[end for]@
  return true;
}

static bool
_@(message.structure.namespaced_type.name)__publish(void * dds_data_writer, const void * untyped_ros_message)
{
  if (!dds_data_writer) {
    fprintf(stderr, "data writer handle is null\n");
    return false;
  }
  const @(__ros_c_msg_type) * ros_message = static_cast<const @(__ros_c_msg_type) *>(untyped_ros_message);
  if (!ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }

  DDS::DataWriter * topic_writer = static_cast<DDS::DataWriter *>(dds_data_writer);

  @(__dds_cpp_msg_type) dds_message;
  if (!_@(message.structure.namespaced_type.name)__convert_ros_to_dds(ros_message, &dds_message)) {
    return false;
  }
  @(__dds_cpp_msg_type_prefix)_DataWriter * data_writer =
    @(__dds_cpp_msg_type_prefix)_DataWriter::narrow(topic_writer);
  DDS::ReturnCode_t status = data_writer->write(&dds_message, DDS::HANDLE_NIL);

  switch (status) {
    case DDS::RETCODE_ERROR:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_DataWriter.write: "
        "an internal error has occurred\n");
      return false;
    case DDS::RETCODE_BAD_PARAMETER:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_DataWriter.write: "
        "bad handle or instance_data parameter\n");
      return false;
    case DDS::RETCODE_ALREADY_DELETED:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_DataWriter.write: "
        "this @(__dds_cpp_msg_type_prefix)DataWriter has already been deleted\n");
      return false;
    case DDS::RETCODE_OUT_OF_RESOURCES:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_DataWriter.write: "
        "out of resources\n");
      return false;
    case DDS::RETCODE_NOT_ENABLED:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_DataWriter.write: "
        "this @(__dds_cpp_msg_type_prefix)DataWriter is not enabled\n");
      return false;
    case DDS::RETCODE_PRECONDITION_NOT_MET:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_DataWriter.write: "
        "the handle has not been registered with this @(__dds_cpp_msg_type_prefix)DataWriter\n");
      return false;
    case DDS::RETCODE_TIMEOUT:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_DataWriter.write: "
        "writing resulted in blocking and then exceeded the timeout set by the "
        "max_blocking_time of the ReliabilityQosPolicy\n");
      return false;
    case DDS::RETCODE_OK:
      return true;
    default:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_DataWriter.write: "
        "unknown return code\n");
  }
  return false;
}

static bool
_@(message.structure.namespaced_type.name)__convert_dds_to_ros(const void * untyped_dds_message, void * untyped_ros_message)
{
  if (!untyped_ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }
  if (!untyped_dds_message) {
    fprintf(stderr, "dds message handle is null\n");
    return false;
  }
  const @(__dds_cpp_msg_type) * dds_message = static_cast<const @(__dds_cpp_msg_type) *>(untyped_dds_message);
  @(__ros_c_msg_type) * ros_message = static_cast<@(__ros_c_msg_type) *>(untyped_ros_message);
@[if not message.structure.members]@
  // No fields is a no-op.
  (void)dds_message;
  (void)ros_message;
@[end if]@
@[for member in message.structure.members]@
  // Member name: @(member.name)
  {
@{
type_ = member.type
if isinstance(type_, AbstractNestedType):
    type_ = type_.value_type
}@
@[  if isinstance(member.type, AbstractNestedType)]@
@# // ----------------------- ARRAY || SEQUENCE
@[    if isinstance(member.type, Array)]@
    int32_t size = @(member.type.size);
@[    else]@
    int32_t size = dds_message->@(member.name)_.length();
    if (ros_message->@(member.name).data) {
      @(idl_type_to_c(member.type) + '__fini')(&ros_message->@(member.name));
    }
    if (!@(idl_type_to_c(member.type) + '__init')(&ros_message->@(member.name), size)) {
      return "failed to create array for field '@(member.name)'";
    }
@[    end if]@
    for (int32_t i = 0; i < size; i++) {
@[    if isinstance(member.type, Array)]@
      auto & ros_i = ros_message->@(member.name)[i];
@[    else]@
      auto & ros_i = ros_message->@(member.name).data[i];
@[    end if]@
@[    if isinstance(type_, BasicType)]@
@[      if type_.typename == 'boolean']@
      ros_i = (dds_message->@(member.name)_[i] != 0);
@[      else]@
      ros_i = dds_message->@(member.name)_[i];
@[      end if]@
@[    elif isinstance(type_, AbstractString)]@
@# // ----------------------- ARRAY || SEQUENCE [ STRING ]
      if (!ros_i.data) {
        rosidl_generator_c__String__init(&ros_i);
      }
@[      if type_.has_maximum_size()]@
@[        if isinstance(member.type, Array) ]@
      // array of bounded strings
      bool succeeded = rosidl_generator_c__String__assign(&ros_i,dds_message->@(member.name)_[i]);
@[        else]@
      // sequence of bounded strings
      bool succeeded = rosidl_generator_c__String__assign(&ros_i,dds_message->@(member.name)_[i].value);
@[        end if]@
@[      else ]@
      // sequence or array of unbounded strings
      bool succeeded = rosidl_generator_c__String__assign(&ros_i,dds_message->@(member.name)_[i]);
@[      end if]@
      if (!succeeded) {
        fprintf(stderr, "failed to assign string into field '@(member.name)'\n");
        return false;
      }
@[    elif isinstance(type_, AbstractWString)]@
@# // ----------------------- ARRAY || SEQUENCE [ WSTRING ]
      if (!ros_i.data) {
        rosidl_generator_c__U16String__init(&ros_i);
      }
@[      if type_.has_maximum_size()]@
@[        if isinstance(member.type, Array) ]@
      // array of bounded wstrings
      bool succeeded = rosidl_typesupport_coredx_c::wstring_to_u16string(dds_message->@(member.name)_[i], ros_i);
@[        else]@
      // sequence of bounded wstrings
      bool succeeded = rosidl_typesupport_coredx_c::wstring_to_u16string(dds_message->@(member.name)_[i].value, ros_i);
@[        end if]@
@[      else ]@
      // sequence/array of unbounded wstrings
      bool succeeded = rosidl_typesupport_coredx_c::wstring_to_u16string(dds_message->@(member.name)_[i], ros_i);
@[      end if]@
      if (!succeeded) {
        fprintf(stderr, "failed to create wstring from u16string\n");
        rosidl_generator_c__U16String__fini(&ros_i);
        return false;
      }
@[    elif isinstance(type_, NamespacedType)]@
      const rosidl_message_type_support_t * ts =
        ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
	  rosidl_typesupport_coredx_c,
        @(', '.join(type_.namespaces)),
        @(type_.name))();
      const message_type_support_callbacks_t * callbacks =
        static_cast<const message_type_support_callbacks_t *>(ts->data);
      callbacks->convert_dds_to_ros(&dds_message->@(member.name)_[i], &ros_i);
@[    else]@
@{      assert False, 'Unknown member base type'}@
@[    end if]@
    }
@[  elif isinstance(member.type, AbstractString)]@
@# // ----------------------- STRING
    if (!ros_message->@(member.name).data) {
      rosidl_generator_c__String__init(&ros_message->@(member.name));
    }
@[    if member.type.has_maximum_size()]@
    // bounded string
    bool succeeded = rosidl_generator_c__String__assign(
      &ros_message->@(member.name),
      &dds_message->@(member.name)_[0]);
@[    else ]@
    // unbounded string
    bool succeeded = rosidl_generator_c__String__assign(
      &ros_message->@(member.name),
      dds_message->@(member.name)_);
@[    end if]@
    if (!succeeded) {
      fprintf(stderr, "failed to assign string into field '@(member.name)'\n");
      return false;
    }
@[  elif isinstance(member.type, AbstractWString)]@
@# // ----------------------- WSTRING
    if (!ros_message->@(member.name).data) {
      rosidl_generator_c__U16String__init(&ros_message->@(member.name));
    }
@[    if member.type.has_maximum_size()]@
    // bounded wstring
    bool succeeded = rosidl_typesupport_coredx_c::wstring_to_u16string(
         &dds_message->@(member.name)_[0],
    	 ros_message->@(member.name));
@[    else ]@
    // unbounded wstring
    bool succeeded = rosidl_typesupport_coredx_c::wstring_to_u16string(
         dds_message->@(member.name)_,
    	 ros_message->@(member.name));
@[    end if]@
    if (!succeeded) {
      fprintf(stderr, "failed to create wstring from u16string\n");
      rosidl_generator_c__U16String__fini(&ros_message->@(member.name));
      return false;
    }
@[  elif isinstance(member.type, BasicType)]@
@# // ----------------------- PRIMITIVE
    ros_message->@(member.name) = dds_message->@(member.name)_@(' != 0' if member.type.typename == 'boolean' else '');
@[  elif isinstance(member.type, NamespacedType)]@
@# // ----------------------- COMPLEX
    const rosidl_message_type_support_t * ts =
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
        rosidl_typesupport_coredx_c,
 	@(', '.join(member.type.namespaces)),
        @(member.type.name))();
    const message_type_support_callbacks_t * callbacks =
      static_cast<const message_type_support_callbacks_t *>(ts->data);
    callbacks->convert_dds_to_ros(&dds_message->@(member.name)_, &ros_message->@(member.name));
@[  else]@
@{    assert False, 'Unknown member type'}@
@[  end if]@
  }

@[end for]@
  return true;
}

static bool
_@(message.structure.namespaced_type.name)__take(
  void * dds_data_reader,
  bool ignore_local_publications,
  void * untyped_ros_message,
  bool * taken,
  void * sending_publication_handle)
{
  if (untyped_ros_message == 0) {
    fprintf(stderr, "invalid ros message pointer\n");
    return false;
  }

  DDS::DataReader * topic_reader = static_cast<DDS::DataReader *>(dds_data_reader);

  @(__dds_cpp_msg_type_prefix)_DataReader * data_reader =
    @(__dds_cpp_msg_type_prefix)_DataReader::narrow(topic_reader);

  @(__dds_cpp_msg_type_prefix)_PtrSeq dds_messages;
  DDS::SampleInfoSeq sample_infos;
  DDS::ReturnCode_t status = data_reader->take(
    &dds_messages,
    &sample_infos,
    1,
    DDS::ANY_SAMPLE_STATE,
    DDS::ANY_VIEW_STATE,
    DDS::ANY_INSTANCE_STATE);

  bool ignore_sample = false;
  
  switch (status) {
    case DDS::RETCODE_ERROR:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_DataReader.take: "
        "an internal error has occurred\n");
      goto finally;
    case DDS::RETCODE_ALREADY_DELETED:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_DataReader.take: "
        "this @(__dds_cpp_msg_type_prefix)DataReader has already been deleted\n");
      goto finally;
    case DDS::RETCODE_OUT_OF_RESOURCES:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_DataReader.take: "
        "out of resources\n");
      goto finally;
    case DDS::RETCODE_NOT_ENABLED:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_DataReader.take: "
        "this @(__dds_cpp_msg_type_prefix)DataReader is not enabled\n");
      goto finally;
    case DDS::RETCODE_PRECONDITION_NOT_MET:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_DataReader.take: "
        "a precondition is not met, one of: "
        "max_samples > maximum and max_samples != LENGTH_UNLIMITED, or "
        "the two sequences do not have matching parameters (length, maximum, release), or "
        "maximum > 0 and release is false.\n");
      goto finally;
    case DDS::RETCODE_NO_DATA:
      *taken = false;
      goto finally;
    case DDS::RETCODE_OK:
      break;
    default:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_DataReader.take: "
        "unknown return code\n");
      goto finally;
  }

  {
    DDS::SampleInfo * sample_info = sample_infos[0];
    if (!sample_info->valid_data) {
      // skip sample without data
      ignore_sample = true;
    } else if (ignore_local_publications) {
      // compare the lower 12 octets of the guids from the sender and this receiver
      // if they are equal the sample has been sent from this process and should be ignored
      void * pub_key = DDS_InstanceHandle_get_key(sample_info->publication_handle);
      void * sub_key = DDS_InstanceHandle_get_key(topic_reader->get_instance_handle());
      if (pub_key && sub_key) {
        if (memcmp(pub_key, sub_key, 12) == 0) { /* belong to same DomainParticipant */
          ignore_sample = true;
        } else {
          ignore_sample = false;
        }
      }
      // This is nullptr when being used with plain rmw_take, so check first.
      if (sending_publication_handle) {
        *static_cast<DDS::InstanceHandle_t *>(sending_publication_handle) =
          sample_info->publication_handle;
      }
    }
  }

  if (!ignore_sample) {
    if (!_@(message.structure.namespaced_type.name)__convert_dds_to_ros(dds_messages[0], untyped_ros_message)) {
      goto finally;
    }
    *taken = true;
  } else {
    *taken = false;
  }

finally:
  // Ensure the loan is returned.
  status = data_reader->return_loan(&dds_messages, &sample_infos);
  switch (status) {
    case DDS::RETCODE_ERROR:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_DataReader.return_loan: "
        "an internal error has occurred\n");
      return false;
    case DDS::RETCODE_ALREADY_DELETED:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_DataReader.return_loan: "
        "this @(__dds_cpp_msg_type_prefix)DataReader has already been deleted\n");
      return false;
    case DDS::RETCODE_OUT_OF_RESOURCES:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_DataReader.return_loan: "
        "out of resources\n");
      return false;
    case DDS::RETCODE_NOT_ENABLED:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_DataReader.return_loan: "
        "this @(__dds_cpp_msg_type_prefix)DataReader is not enabled\n");
      return false;
    case DDS::RETCODE_PRECONDITION_NOT_MET:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_DataReader.return_loan: "
        "a precondition is not met, one of: "
        "the data_values and info_seq do not belong to a single related pair, or "
        "the data_values and info_seq were not obtained from this "
        "@(__dds_cpp_msg_type_prefix)DataReader\n");
      return false;
    case DDS::RETCODE_OK:
      return true;
    default:
      fprintf(stderr, "@(__dds_cpp_msg_type_prefix)_DataReader.return_loan: "
        "unknown return code\n");
  }

  return false;
}

//--------------------------------------------
// convert dds msg to CDR stream
static bool
_@(message.structure.namespaced_type.name)__dds_msg_to_cdr( const void * untyped_dds_msg,
                rmw_serialized_message_t *buf )
{
  bool retval = false;
  const @(__dds_cpp_msg_type) * dds_message = static_cast<const @(__dds_cpp_msg_type) *>(untyped_dds_msg);

  // get required size
  size_t buf_len = dds_message->get_marshal_size( 0, 0 );
  if ( buf_len > 0 )
    buf_len += 4; /* CDR hdr */
  
  // allocate required buffer
  if ( buf_len > buf->buffer_capacity )
    {
      if ( buf->buffer_capacity > 0 )
	buf->allocator.deallocate( buf->buffer, buf->allocator.state );
      buf->buffer = static_cast<unsigned char *>(buf->allocator.allocate(buf_len , buf->allocator.state));
      if ( buf->buffer )
	buf->buffer_capacity = buf_len;
    }
  
  // call marshal to serialize data
  unsigned char * cbuf = buf->buffer;
  buf->buffer_length = dds_message->marshal_cdr( cbuf+4, 0, (unsigned int)(buf->buffer_capacity-4), 0, 0 );
  buf->buffer_length += 4;
  if ( buf->buffer_length == buf_len )
    {
      /* construct CDR hdr  */
      unsigned char my_endian;
      DDS_MARSH_MY_ENDIAN( my_endian );
      cbuf[ 0 ]  = 0x00;
      cbuf[ 1 ]  = my_endian;
      cbuf[ 2 ]  = 0x00;
      cbuf[ 3 ]  = 0x00;
      retval = true;
    }

  return retval;
}

//--------------------------------------------
// convert CDR stream to dds msg
static bool
_@(message.structure.namespaced_type.name)__cdr_to_dds_msg( void * untyped_dds_msg,
                const rmw_serialized_message_t *buf )
{
  bool retval = false;
  unsigned char * cbuf    = buf->buffer;
  uint32_t        buf_len = buf->buffer_length;
  @(__dds_cpp_msg_type) * dds_message = static_cast<@(__dds_cpp_msg_type) *>(untyped_dds_msg);
  
  // validate that the data is CDR encoding, and check endian flag
  unsigned char my_endian;
  unsigned char data_endian;
  DDS_MARSH_MY_ENDIAN( my_endian );
  if ( cbuf[0] == 0x00 )
    {
      data_endian = cbuf[1] & 0x01;
      // skip CDR hdr
      cbuf    += 4;
      buf_len -= 4;
  
      // call unmarshal
      dds_message->unmarshal_cdr( cbuf, 0, buf_len, (data_endian == my_endian)?0:1, 0 );
      retval = true;
    }
  return retval;
}

//--------------------------------------------
// convert ros msg to CDR stream
static bool
_@(message.structure.namespaced_type.name)__serialize( const void    * untyped_ros_msg,
		       rmw_serialized_message_t *buf )
{
  if (untyped_ros_msg == 0) {
    fprintf(stderr, "serialize: invalid ros message pointer\n");
    return false;
  }
  if ( !buf ) {
    fprintf(stderr, "serialize: invalid buf pointer\n");
    return false;
  }
  const @(__ros_c_msg_type) * ros_message = static_cast<const @(__ros_c_msg_type) *>(untyped_ros_msg);
  @(__dds_cpp_msg_type) dds_message;
  bool retval;
  retval = _@(message.structure.namespaced_type.name)__convert_ros_to_dds(ros_message, &dds_message);
  if ( retval )
    retval = _@(message.structure.namespaced_type.name)__dds_msg_to_cdr( &dds_message, buf );
  return retval;
}

//--------------------------------------------
// convert CDR stream to ros msg
static bool
_@(message.structure.namespaced_type.name)__deserialize( void * untyped_ros_msg,
			 const rmw_serialized_message_t *buf )
{
  if (untyped_ros_msg == 0) {
    fprintf(stderr, "deserialize: invalid ros message pointer\n");
    return false;
  } 
  if ( !buf  ) {
    fprintf(stderr, "deserialize: invalid buf pointer\n");
    return false;
  }
  
  @(__dds_cpp_msg_type) dds_message;
  bool retval = _@(message.structure.namespaced_type.name)__cdr_to_dds_msg( &dds_message, buf );
  if ( retval ) {
    retval = _@(message.structure.namespaced_type.name)__convert_dds_to_ros(&dds_message, untyped_ros_msg);
  }
  return retval;   
}

//--------------------------------------------
static void *
_@(message.structure.namespaced_type.name)__alloc_ros_msg( rcutils_allocator_t * allocator )
{
  void * buf = allocator->allocate( sizeof(@(__ros_c_msg_type)), allocator->state );
  if ( !buf )
    return NULL;
  @(__ros_c_msg_type) * ros_message = new(buf) @(__ros_c_msg_type);
  return ros_message;
}
  
//--------------------------------------------
static void
_@(message.structure.namespaced_type.name)__free_ros_msg( void * ros_msg, rcutils_allocator_t * allocator )
{
  if ( ros_msg )
    {
      @(__ros_c_msg_type) * ros_message = static_cast<@(__ros_c_msg_type) *>(ros_msg);
      ros_message->~@(__ros_c_msg_type)();
      allocator->deallocate( ros_msg, allocator->state );
    }
}

@# // Collect the callback functions and provide a function to get the type support struct.
static message_type_support_callbacks_t _@(message.structure.namespaced_type.name)__callbacks = {
  "@('::'.join([package_name] + list(interface_path.parents[0].parts)))",  // namespace
  "@(message.structure.namespaced_type.name)",  // message_name
  _@(message.structure.namespaced_type.name)__register_type,  // register_type
  _@(message.structure.namespaced_type.name)__publish,  // publish
  _@(message.structure.namespaced_type.name)__take,  // take
  _@(message.structure.namespaced_type.name)__convert_ros_to_dds,  // convert_ros_to_dds
  _@(message.structure.namespaced_type.name)__convert_dds_to_ros,  // convert_dds_to_ros
  _@(message.structure.namespaced_type.name)__serialize,
  _@(message.structure.namespaced_type.name)__deserialize,
  _@(message.structure.namespaced_type.name)__alloc_ros_msg,
  _@(message.structure.namespaced_type.name)__free_ros_msg
};

static rosidl_message_type_support_t _@(message.structure.namespaced_type.name)__type_support = {
  rosidl_typesupport_coredx_c__identifier,
  &_@(message.structure.namespaced_type.name)__callbacks,
  get_message_typesupport_handle_function,
};

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
  rosidl_typesupport_coredx_c,
  @(', '.join([package_name] + list(interface_path.parents[0].parts))),
  @(message.structure.namespaced_type.name))()
{
  return &_@(message.structure.namespaced_type.name)__type_support;
}

#ifdef __cplusplus
}
#endif
