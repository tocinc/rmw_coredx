# Copyright 2014 Open Source Robotics Foundation, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from rosidl_generator_dds_idl import get_post_struct_lines as get_default_post_struct_lines

def get_post_struct_lines(spec):
    lines = get_default_post_struct_lines(spec)
    # str="// " + spec.base_type.pkg_name + "::" + spec.base_type.type
    # lines.append(str)
    if '_Request' in spec.base_type.type:
        lines.append('//@request_type')
    elif '_Response' in spec.base_type.type:
        lines.append('//@reply_type')
    return lines
