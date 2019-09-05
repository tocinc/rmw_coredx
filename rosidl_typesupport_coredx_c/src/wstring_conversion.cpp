// Copyright 2019 Open Source Robotics Foundation, Inc.
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

#include <rosidl_typesupport_coredx_c/wstring_conversion.hpp>
#include "rosidl_generator_c/u16string_functions.h"

namespace rosidl_typesupport_coredx_c
{

  cdx_char32_t * create_wstring_from_u16string(const rosidl_generator_c__U16String & u16str)
  {
    cdx_char32_t * wstr = new cdx_char32_t[u16str.size];
    if (NULL == wstr) {
      return wstr;
    }
    for (size_t i = 0; i < u16str.size; ++i) {
      wstr[i] = static_cast<cdx_char32_t>(u16str.data[i]);
    }
    wstr[u16str.size] = static_cast<cdx_char32_t>(u'\0');
    return wstr;
  }

  bool wstring_to_u16string(const cdx_char32_t * wstr, rosidl_generator_c__U16String & u16str)
  {
    size_t size = static_cast<size_t>( toc_wstrlen( wstr ) );
    bool succeeded = rosidl_generator_c__U16String__resize(&u16str, size);
    if (!succeeded) {
      return false;
    }
    for (size_t i = 0; i < size; ++i) {
      u16str.data[i] = static_cast<char16_t>(wstr[i]);
    }
    u16str.data[size] = u'\0';
    return true;
  }

  bool copy_wstring_from_u16string( cdx_char32_t * dest,
				    const rosidl_generator_c__U16String * src,
				    uint32_t max )
  {
    if ( max > src->size )
      max = src->size;
    for (size_t i = 0; i < max; ++i) {
      dest[i] = static_cast<cdx_char32_t>(src->data[i]);
    }
    dest[max] = 0;
    return true;
  }
}  // namespace rosidl_typesupport_coredx_c
