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

#ifndef ROSIDL_TYPESUPPORT_COREDX_C__WSTRING_CONVERSION_HPP_
#define ROSIDL_TYPESUPPORT_COREDX_C__WSTRING_CONVERSION_HPP_

#include "rosidl_generator_c/u16string.h"
#include "rosidl_typesupport_coredx_c/visibility_control.h"
#include <dds/dds.hh>

namespace rosidl_typesupport_coredx_c
{

ROSIDL_TYPESUPPORT_COREDX_C_PUBLIC
cdx_char32_t * create_wstring_from_u16string(
  const rosidl_generator_c__U16String & u16str);

ROSIDL_TYPESUPPORT_COREDX_C_PUBLIC
bool wstring_to_u16string(
  const cdx_char32_t * wstr, rosidl_generator_c__U16String & u16str);

ROSIDL_TYPESUPPORT_COREDX_C_PUBLIC
bool copy_wstring_from_u16string(
  cdx_char32_t * dest, const rosidl_generator_c__U16String * src, uint32_t max );
  
}  // namespace rosidl_typesupport_coredx_c

#endif  // ROSIDL_TYPESUPPORT_COREDX_C__WSTRING_CONVERSION_HPP_
