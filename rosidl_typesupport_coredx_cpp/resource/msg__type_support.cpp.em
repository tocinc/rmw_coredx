// generated from rosidl_typesupport_coredx_cpp/resource/msg__type_support.cpp.em
@# Included from rosidl_typesupport_coredx_cpp/resource/idl__dds_coredx__type_support.cpp.em
@{
from rosidl_cmake import convert_camel_case_to_lower_case_underscore
from rosidl_parser.definition import AbstractGenericString
from rosidl_parser.definition import AbstractNestedType
from rosidl_parser.definition import AbstractString
from rosidl_parser.definition import AbstractWString
from rosidl_parser.definition import Array
from rosidl_parser.definition import BasicType
from rosidl_parser.definition import BoundedSequence
from rosidl_parser.definition import NamespacedType
include_parts = [package_name] + list(interface_path.parents[0].parts)
include_base = '/'.join(include_parts)

include_prefix = convert_camel_case_to_lower_case_underscore(interface_path.stem)

header_files = [
    include_base + '/' + include_prefix + '__rosidl_typesupport_coredx_cpp.hpp',
    'rcutils/types/uint8_array.h',
    'rosidl_typesupport_cpp/message_type_support.hpp',
    'rosidl_typesupport_coredx_cpp/identifier.hpp',
    'rosidl_typesupport_coredx_cpp/message_type_support.h',
    'rosidl_typesupport_coredx_cpp/message_type_support_decl.hpp',
    'rosidl_typesupport_coredx_cpp/wstring_conversion.hpp',
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

@[for member in message.structure.members]@
@{
type_ = member.type
if isinstance(type_, AbstractNestedType):
   type_ = type_.value_type
}@
@[    if isinstance(type_, NamespacedType)]@
@[        for ns in type_.namespaces]@
namespace @(ns)
{
@[        end for]@
namespace dds_
{
class @(type_.name)_;
}  // namespace dds_

namespace typesupport_coredx_cpp
{
@{
member_ros_msg_pkg_prefix = '::'.join(type_.namespaces)
member_ros_msg_type = member_ros_msg_pkg_prefix + '::' + type_.name
member_dds_msg_type = member_ros_msg_pkg_prefix + '::dds_::' + type_.name + '_'
}@

bool convert_ros_message_to_dds( const @(member_ros_msg_type) &,
                                       @(member_dds_msg_type) & );
bool convert_dds_message_to_ros( const @(member_dds_msg_type) &,
                                       @(member_ros_msg_type) & );
}  // namespace typesupport_coredx_cpp
@[        for ns in reversed(type_.namespaces)]@
}  // namespace @(ns)
@[        end for]@
@[    end if]@
@[end for]@

@[for ns in message.structure.namespaced_type.namespaces]@

namespace @(ns)
{
@[end for]@

namespace typesupport_coredx_cpp
{

@{
__ros_msg_pkg_prefix = '::'.join(message.structure.namespaced_type.namespaces)
__ros_msg_type = __ros_msg_pkg_prefix + '::' + message.structure.namespaced_type.name
__dds_msg_type_prefix = __ros_msg_pkg_prefix + '::dds_::' + message.structure.namespaced_type.name
__dds_msg_type = __dds_msg_type_prefix + '_'
__type_name = message.structure.namespaced_type.name
}@

bool
register_type__@(__type_name)(
  void * untyped_participant,
  const char * type_name)
{
  DDS::DomainParticipant * participant = static_cast<DDS::DomainParticipant *>(untyped_participant);
  DDS::ReturnCode_t status =
    @(__dds_msg_type)TypeSupport::register_type(participant, type_name);
  return status == DDS::RETCODE_OK;
}
 
bool
convert_ros_message_to_dds__@(__type_name)(
  const void * vros_message,
        void * vdds_message )
{
@[if not message.structure.members]@
  (void)vros_message;
  (void)vdds_message;
@[end if]@
  const @(__ros_msg_type) * ros_message = (const @(__ros_msg_type) *)vros_message;
        @(__dds_msg_type) * dds_message = (@(__dds_msg_type) *)vdds_message;
@[for member in message.structure.members]@
  // member.name @(member.name)
@{
type_ = member.type
if isinstance(type_, AbstractNestedType):
   elem_type_ = type_.value_type
}@
@[  if isinstance(member.type, AbstractNestedType)]@
  {
@# ---------------------------------------------- SEQUENCE / ARRAY
@[    if isinstance(member.type, Array)]@
    size_t size = @(member.type.size);
@[    else]@
    size_t size = ros_message->@(member.name).size();
    if (size > (std::numeric_limits<int32_t>::max)()) {
      throw std::runtime_error("array size exceeds maximum DDS sequence size");
    }
@[      if isinstance(member.type, BoundedSequence)]@
    if (size > @(member.type.maximum_size)) {
      throw std::runtime_error("array size exceeds upper bound");
    }
@[      end if]@
@[    end if]@
  
@# clear seq/array unbounded [w]string contents
@# @[    if isinstance(elem_type_, (AbstractString, AbstractWString)) and not elem_type_.has_maximum_size()]@
@#     for (size_t i = 0;i <  dds_message->@(member.name)_.length(); i++) {
@# @#    // clear unbounded string/wstring
@#       delete[] dds_message->@(member.name)_[static_cast<int32_t>(i)];
@#     }
@# @[    end if]@
  
@[    if not isinstance(member.type, Array)]@
    uint32_t length = static_cast<int32_t>(size);
    if (!dds_message->@(member.name)_.resize(length)) {
      throw std::runtime_error("failed to set length of sequence");
    }
@[    end if]@
    for (size_t i = 0; i < size; i++) {
      @#---------------------------------------------------------
@[    if isinstance(elem_type_, AbstractString)]@
@[      if elem_type_.has_maximum_size()]@
@[        if isinstance(member.type, Array) ]@
          @# --------------------- ARRAY of bounded strings
      strncpy(dds_message->@(member.name)_[static_cast<int32_t>(i)],
              ros_message->@(member.name)[i].c_str(),
              @(elem_type_.maximum_size));
@[        else]@
          @# --------------------- SEQUENCE of bounded strings
      strncpy(dds_message->@(member.name)_[static_cast<int32_t>(i)].value,
              ros_message->@(member.name)[i].c_str(),
              @(elem_type_.maximum_size));
@[        end if]@
@[      else]@
          @# --------------------- ARRAY/SEQUENCE of unbounded strings
      dds_message->@(member.name)_[static_cast<int32_t>(i)] =
        DDS::String_dup(ros_message->@(member.name)[i].c_str());
@[      end if]@
      @#---------------------------------------------------------
@[    elif isinstance(elem_type_, AbstractWString)]@
@[      if elem_type_.has_maximum_size()]@
@[        if isinstance(member.type, Array) ]@
          @# --------------------- ARRAY of bounded wstrings
      rosidl_typesupport_coredx_cpp::copy_wstring_from_u16string(dds_message->@(member.name)_[static_cast<int32_t>(i)],
        ros_message->@(member.name)[i],
        @(elem_type_.maximum_size));
@[        else]@
          @# --------------------- SEQUENCE of bounded wstrings
      rosidl_typesupport_coredx_cpp::copy_wstring_from_u16string(dds_message->@(member.name)_[static_cast<int32_t>(i)].value,
        ros_message->@(member.name)[i],
        @(elem_type_.maximum_size));
@[        end if]@
@[      else]@
          @# --------------------- ARRAY/SEQUENCE of unbounded wstrings
      cdx_char32_t * wstr =
      rosidl_typesupport_coredx_cpp::create_wstring_from_u16string( ros_message->@(member.name)[i] );
      if (NULL == wstr) {
        throw std::runtime_error("failed to create wstring from u16string");
      }
      dds_message->@(member.name)_[static_cast<int32_t>(i)] = wstr;
@[      end if]@
      @#---------------------------------------------------------
@[    elif isinstance(elem_type_, BasicType)]@
      dds_message->@(member.name)_[static_cast<int32_t>(i)] =
        ros_message->@(member.name)[i];
      @#---------------------------------------------------------
@[    else]@
      if (
        !@('::'.join(member.type.value_type.namespaces))::typesupport_coredx_cpp::convert_ros_message_to_dds(
          ros_message->@(member.name)[i],
          dds_message->@(member.name)_[static_cast<int32_t>(i)]))
      {
        return false;
      }
@[    end if]@
     }
  }
@[  elif isinstance(member.type, AbstractString)]@
@# ---------------------------------------------- STRING
@[    if member.type.has_maximum_size()]@
  strncpy(dds_message->@(member.name)_,
          ros_message->@(member.name).c_str(),
          @(member.type.maximum_size));
@[    else]@
  DDS::String_free(dds_message->@(member.name)_);
  dds_message->@(member.name)_ =
    DDS::String_dup(ros_message->@(member.name).c_str());
@[    end if]@
@[  elif isinstance(member.type, AbstractWString)]@
@# ---------------------------------------------- WSTRING
@[    if member.type.has_maximum_size()]@
  rosidl_typesupport_coredx_cpp::copy_wstring_from_u16string(dds_message->@(member.name)_,
	&ros_message->@(member.name),
	@(member.type.maximum_size) );
@[    else]@
  delete[] dds_message->@(member.name)_;
  dds_message->@(member.name)_ =
    rosidl_typesupport_coredx_cpp::create_wstring_from_u16string( ros_message->@(member.name) );
@[    end if]@
@[  elif isinstance(member.type, BasicType)]@
@# ---------------------------------------------- BASIC
  dds_message->@(member.name)_ = ros_message->@(member.name);
@[  else]@
@# ---------------------------------------------- COMPLEX
  if (
    !@('::'.join(member.type.namespaces))::typesupport_coredx_cpp::convert_ros_message_to_dds(
      ros_message->@(member.name),
      dds_message->@(member.name)_) )
  {
    return false;
  }
@[  end if]@
@[end for]@
  return true;
}

bool
convert_ros_message_to_dds(
  const @(__ros_msg_type) & ros_message,
        @(__dds_msg_type) & dds_message )
{
  return convert_ros_message_to_dds__@(__type_name)( (const void*)&ros_message, (void *)&dds_message );
}
   
bool
publish__@(__type_name)(
  void * untyped_topic_writer,
  const void * untyped_ros_message)
{
  DDS::DataWriter * topic_writer = static_cast<DDS::DataWriter *>(untyped_topic_writer);
  const @(__ros_msg_type) & ros_message = *(const @(__ros_msg_type) *)untyped_ros_message;
  @(__dds_msg_type) * dds_message = new @(__dds_msg_type);
  if (!dds_message) {
    return false;
  }
  bool success = convert_ros_message_to_dds(ros_message, *dds_message);
  if (success) {
    @(__dds_msg_type)DataWriter * data_writer =
      @(__dds_msg_type)DataWriter::narrow(topic_writer);
    DDS::ReturnCode_t status = data_writer->write(dds_message, DDS::HANDLE_NIL);
    success = status == DDS::RETCODE_OK;
  }
  delete dds_message;
  return success;
}

bool
convert_dds_message_to_ros__@(__type_name)(
			   const void * untyped_dds_message,
			   void * untyped_ros_message )
{
@[if not message.structure.members]@
  (void)untyped_ros_message;
  (void)untyped_dds_message;
@[else]@
  const @(__dds_msg_type) * dds_message = (const @(__dds_msg_type) *)untyped_dds_message;
        @(__ros_msg_type) * ros_message = (      @(__ros_msg_type) *)untyped_ros_message;
@[for member in message.structure.members]@
  // member.name @(member.name)
@{
type_ = member.type
if isinstance(type_, AbstractNestedType):
   elem_type_ = type_.value_type
}@
@[  if isinstance(member.type, AbstractNestedType)]@
@# ----------------------------------------------------------
  {
@[    if isinstance(member.type, Array)]@
    size_t size = @(member.type.size);
@[    else]@
    size_t size = dds_message->@(member.name)_.length();
    ros_message->@(member.name).resize(size);
@[    end if]@

    for (size_t i = 0; i < size; i++) {
@[    if isinstance(member.type.value_type, BasicType)]@
      ros_message->@(member.name)[i] =
        dds_message->@(member.name)_[static_cast<int32_t>(i)]@(' != 0' if elem_type_.typename == 'boolean' else '');
@[    elif isinstance(member.type.value_type, AbstractString)]@
@[      if (elem_type_.has_maximum_size() )]@
@[        if isinstance(member.type, Array)]@
      // array of bounded string
      ros_message->@(member.name)[i] = dds_message->@(member.name)_[static_cast<int32_t>(i)];
@[        else]@
      // seq of bounded string
      ros_message->@(member.name)[i] = dds_message->@(member.name)_[static_cast<int32_t>(i)].value;
@[        end if]@
@[      else]@
      // array/seq of unbounded string
      ros_message->@(member.name)[i] = dds_message->@(member.name)_[static_cast<int32_t>(i)];
@[      end if]@
@[    elif isinstance(member.type.value_type, AbstractWString)]@
      bool succeeded;
@[      if (elem_type_.has_maximum_size() )]@
@[        if isinstance(member.type, Array)]@
      // array of bounded string
      succeeded = rosidl_typesupport_coredx_cpp::wstring_to_u16string(dds_message->@(member.name)_[static_cast<int32_t>(i)], ros_message->@(member.name)[i]);
@[        else]@
      // seq of bounded string
      succeeded = rosidl_typesupport_coredx_cpp::wstring_to_u16string(dds_message->@(member.name)_[static_cast<int32_t>(i)].value, ros_message->@(member.name)[i]);
@[        end if]@
@[      else]@
      // array/seq of unbounded string
      succeeded = rosidl_typesupport_coredx_cpp::wstring_to_u16string(dds_message->@(member.name)_[static_cast<int32_t>(i)], ros_message->@(member.name)[i]);
@[      end if]@
      if (!succeeded) {
        fprintf(stderr, "failed to create wstring from u16string\n");
        return false;
      }
@[    else]@
      if (
        !@('::'.join(member.type.value_type.namespaces))::typesupport_coredx_cpp::convert_dds_message_to_ros(
          dds_message->@(member.name)_[static_cast<int32_t>(i)],
          ros_message->@(member.name)[i]))
      {
        return false;
      }
@[    end if]@
    }
  }

@[  elif isinstance(member.type, BasicType)]@
  ros_message->@(member.name) =
    dds_message->@(member.name)_@(' != 0' if member.type.typename == 'boolean' else '');
@[  elif isinstance(member.type, AbstractString)]@
  ros_message->@(member.name) = dds_message->@(member.name)_;
@[  elif isinstance(member.type, AbstractWString)]@
  {
    bool succeeded = rosidl_typesupport_coredx_cpp::wstring_to_u16string(dds_message->@(member.name)_, ros_message->@(member.name));
    if (!succeeded) {
      fprintf(stderr, "failed to create wstring from u16string\n");
      return false;
    }
  }
@[  else]@
  if (
    !@('::'.join(member.type.namespaces))::typesupport_coredx_cpp::convert_dds_message_to_ros(
      dds_message->@(member.name)_,
      ros_message->@(member.name)))
  {
    return false;
  }
@[  end if]@

@[end for]@
@[end if]@
  return true;
}
 
bool
convert_dds_message_to_ros(const @(__dds_msg_type) & dds_message,
			         @(__ros_msg_type) & ros_message )
{
  return convert_dds_message_to_ros__@(__type_name)( (const void*)&dds_message, (void*)&ros_message );
}

bool
take__@(__type_name)(
  void * untyped_topic_reader,
  bool ignore_local_publications,
  void * untyped_ros_message,
  bool * taken,
  void * sending_publication_handle)
{
  if (!untyped_topic_reader) {
    throw std::runtime_error("topic reader handle is null");
  }
  if (!untyped_ros_message) {
    throw std::runtime_error("ros message handle is null");
  }
  if (!taken) {
    throw std::runtime_error("taken handle is null");
  }

  DDS::DataReader * topic_reader = static_cast<DDS::DataReader *>(untyped_topic_reader);
  @(__dds_msg_type)DataReader * data_reader = @(__dds_msg_type)DataReader::narrow(topic_reader);
  @(__dds_msg_type)PtrSeq dds_messages;
  DDS::SampleInfoSeq sample_infos;
  DDS::ReturnCode_t status = data_reader->take(
    &dds_messages,
    &sample_infos,
    1,
    DDS::ANY_SAMPLE_STATE,
    DDS::ANY_VIEW_STATE,
    DDS::ANY_INSTANCE_STATE);
  if (status == DDS::RETCODE_NO_DATA) {
    *taken = false;
    return true;
  }
  if (status != DDS::RETCODE_OK) {
    fprintf(stderr, "take failed with status = %s\n", DDS_error(status));
    return false;
  }

  bool ignore_sample = false;
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
      if (memcmp(pub_key, sub_key, 12) == 0) { /* belongs to same DomainParticipant */
        ignore_sample = true;
      } else {
        ignore_sample = false;
      }
    }
  }

  if (sample_info->valid_data && sending_publication_handle) {
    *static_cast<DDS::InstanceHandle_t *>(sending_publication_handle) =
      sample_info->publication_handle;
  }

  bool success = true;
  if (!ignore_sample) {
    @(__ros_msg_type) & ros_message = *(@(__ros_msg_type) *)untyped_ros_message;
    success = convert_dds_message_to_ros(*(dds_messages[0]), ros_message);
    if (success) {
      *taken = true;
    }
  } else {
    *taken = false;
  }
  data_reader->return_loan(&dds_messages, &sample_infos);
  return success;
}

static bool serialize__@(__type_name)( const void    * untyped_ros_msg,
		       rmw_serialized_message_t *buf )
{
  if ( !untyped_ros_msg ) {
    fprintf(stderr, "serialize: invalid ros message pointer\n");
    return false;
  }
  if ( !buf ) {
    fprintf(stderr, "serialize: invalid buf pointer\n");
    return false;
  }

  const @(__ros_msg_type) * ros_message = static_cast<const @(__ros_msg_type) *>(untyped_ros_msg);
        @(__dds_msg_type)   dds_message;
  
  // ros data -> dds data
  if (!convert_ros_message_to_dds(*ros_message, dds_message)) {
    return false;
  }
  
  // get required size
  size_t buf_len = dds_message.get_marshal_size( 0, 0 );
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
  buf->buffer_length = dds_message.marshal_cdr( cbuf+4, 0, (unsigned int)(buf->buffer_capacity-4), 0, 0 );
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
    }

  return true;
}
  
 static bool deserialize__@(__type_name)( void * untyped_ros_msg,
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
  
  @(__dds_msg_type)   dds_message;
  @(__ros_msg_type) * ros_msg = (@(__ros_msg_type)*)untyped_ros_msg;
  
  unsigned char * cbuf    = buf->buffer;
  uint32_t        buf_len = buf->buffer_length;

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
      dds_message.unmarshal_cdr( cbuf, 0, buf_len, (data_endian == my_endian)?0:1, 0 ); 
 
      if ( !convert_dds_message_to_ros(dds_message, *ros_msg) ) {
        return false;
      }
    }
  else
    return false;
  
  return true;
}

@# #include <new>
  
static void * alloc_ros_msg__@(__type_name)( rcutils_allocator_t * allocator )
{
  void * buf = allocator->allocate( sizeof( @(__ros_msg_type) ), allocator->state );
  if ( !buf )
    return NULL;
   @(__ros_msg_type) * ros_message =
    new(buf) @(__ros_msg_type);
  return ros_message;
}
  
static void   free_ros_msg__@(__type_name)( void * ros_msg, rcutils_allocator_t * allocator )
{
  if ( ros_msg )
    {
      @(__ros_msg_type) * ros_message =
        static_cast< @(__ros_msg_type) *>(ros_msg);
      ros_message->@(__ros_msg_type)::~@(__type_name)();
      allocator->deallocate( ros_msg, allocator->state );
    }
}

static message_type_support_callbacks_t _@(message.structure.namespaced_type.name)__callbacks = {
  "@('::'.join([package_name] + list(interface_path.parents[0].parts)))",
  "@(message.structure.namespaced_type.name)",
   &register_type__@(__type_name),
   &publish__@(__type_name),
   &take__@(__type_name),
   convert_ros_message_to_dds__@(__type_name),
   convert_dds_message_to_ros__@(__type_name),
   serialize__@(__type_name),
   deserialize__@(__type_name),
   alloc_ros_msg__@(__type_name),
   free_ros_msg__@(__type_name)
};

static rosidl_message_type_support_t _@(message.structure.namespaced_type.name)__handle = {
  rosidl_typesupport_coredx_cpp::typesupport_coredx_identifier,
  &_@(message.structure.namespaced_type.name)__callbacks,
  get_message_typesupport_handle_function,
};

}  // namespace typesupport_coredx_cpp
@[for ns in reversed(message.structure.namespaced_type.namespaces)]@
}  // namespace @(ns)
@[end for]@

// ------------------------------


namespace rosidl_typesupport_coredx_cpp
{

template<>
ROSIDL_TYPESUPPORT_COREDX_CPP_EXPORT_@(package_name)
const rosidl_message_type_support_t *
get_message_type_support_handle<@(__ros_msg_type)>()
{
  return &@(__ros_msg_pkg_prefix)::typesupport_coredx_cpp::_@(message.structure.namespaced_type.name)__handle;
}

}  // namespace rosidl_typesupport_coredx_cpp

#ifdef __cplusplus
extern "C"
{
#endif

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
                  rosidl_typesupport_coredx_cpp,
                  @(', '.join([package_name] + list(interface_path.parents[0].parts))),
		  @(message.structure.namespaced_type.name))()
{
  return &@(__ros_msg_pkg_prefix)::typesupport_coredx_cpp::_@(message.structure.namespaced_type.name)__handle;
}

#ifdef __cplusplus
}
#endif
