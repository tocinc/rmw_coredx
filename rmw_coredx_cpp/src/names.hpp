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

#ifndef RMW_COREDX_CPP__NAMES_HPP_
#define RMW_COREDX_CPP__NAMES_HPP_

#include <string>
#include <vector>

extern const char * const ros_topic_prefix;
extern const char * const ros_service_requester_prefix;
extern const char * const ros_service_response_prefix;

extern std::vector<std::string> _ros_prefixes;

/// Return the ROS specific prefix if it exists, otherwise "".
std::string
_get_ros_prefix_if_exists(const std::string & topic_name);

extern bool
rmw_coredx_process_topic_name( const char * topic_name,
                               bool avoid_ros_namespace_conventions,
                               char ** topic_str,
                               char ** partition_str);

extern bool
rmw_coredx_process_service_name( const char * service_name,
				 bool avoid_ros_namespace_conventions,
				 char ** request_topic_str,
				 char ** response_topic_str);

#endif // RMW_COREDX_CPP__NAMES_HPP_
