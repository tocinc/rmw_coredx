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

#ifdef CoreDX_GLIBCXX_USE_CXX11_ABI_ZERO
#define _GLIBCXX_USE_CXX11_ABI 0
#endif

#include <rmw/rmw.h>
#include <rmw/types.h>
#include <rmw/error_handling.h>
#include <rmw/get_node_info_and_types.h>


#if defined(__cplusplus)
extern "C" {
#endif

  /* ************************************************
   *  Return a list of subscribed topic names and their types.
   *
   * This function returns a list of subscribed topic names and their types.
   *
   * The node parameter must not be `NULL`, and must point to a valid node.
   *
   * The topic_names_and_types parameter must be allocated and zero initialized.
   * The topic_names_and_types is the output for this function, and contains
   * allocated memory.
   * Therefore, it should be passed to rmw_names_and_types_fini() when
   * it is no longer needed.
   * Failing to do so will result in leaked memory.
   *
   * \param[in] node the handle to the node being used to query the ROS graph
   * \param[in] allocator allocator to be used when allocating space for strings
   * \param[in] node_name the name of the node to get information for
   * \param[in] node_namespace the namespace of the node to get information for
   * \param[in] no_demangle if true, list all topics without any demangling
   * \param[out] topic_names_and_types list of topic names and their types the node_name is subscribed to
   * \return `RMW_RET_OK` if the query was successful, or
   * \return `RMW_RET_INVALID_ARGUMENT` if the node is invalid, or
   * \return `RMW_RET_INVALID_ARGUMENT` if any arguments are invalid, or
   * \return `RMW_RET_BAD_ALLOC` if memory allocation fails, or
   * \return `RMW_RET_ERROR` if an unspecified error occurs.
   */
  rmw_ret_t
  rmw_get_subscriber_names_and_types_by_node( const rmw_node_t * node,
					      rcutils_allocator_t * allocator,
					      const char * node_name,
					      const char * node_namespace,
					      bool demangle,
					      rmw_names_and_types_t * topic_names_and_types)
  {
    if (!node) {
      RMW_SET_ERROR_MSG("node handle is null");
      return RMW_RET_INVALID_ARGUMENT;
    }
    if (!allocator) {
      RMW_SET_ERROR_MSG("allocator is null");
      return RMW_RET_INVALID_ARGUMENT;
    }
    if (!node_name) {
      RMW_SET_ERROR_MSG("node_name is null");
      return RMW_RET_INVALID_ARGUMENT;
    }
    if (!node_namespace) {
      RMW_SET_ERROR_MSG("node_namespace is null");
      return RMW_RET_INVALID_ARGUMENT;
    }
    if (!topic_names_and_types) {
      RMW_SET_ERROR_MSG("topic_names_and_types is null");
      return RMW_RET_INVALID_ARGUMENT;
    }

    /* Is this intended to answer for only 'local' (locally created) nodes or
       for any/all discovered node[s] too?
    */
    (void)demangle; 
   
    /* NOT YET IMPLEMENTED  */
    return RMW_RET_ERROR;
  }

  /* ************************************************
   * Return a list of published topic names and their types.
   * This function returns a list of published topic names and their types.
   *
   * The node parameter must not be `NULL`, and must point to a valid node.
   *
   * The topic_names_and_types parameter must be allocated and zero initialized.
   * The topic_names_and_types is the output for this function, and contains
   * allocated memory.
   * Therefore, it should be passed to rmw_names_and_types_fini() when
   * it is no longer needed.
   * Failing to do so will result in leaked memory.
   *
   * \param[in] node the handle to the node being used to query the ROS graph
   * \param[in] allocator allocator to be used when allocating space for strings
   * \param[in] node_name the name of the node to get information for
   * \param[in] node_namespace the namespace of the node to get information for
   * \param[in] no_demangle if true, list all topics without any demangling
   * \param[out] topic_names_and_types list of topic names and their types the node_name is publishing
   * \return `RMW_RET_OK` if the query was successful, or
   * \return `RMW_RET_INVALID_ARGUMENT` if the node is invalid, or
   * \return `RMW_RET_INVALID_ARGUMENT` if any arguments are invalid, or
   * \return `RMW_RET_BAD_ALLOC` if memory allocation fails, or
   * \return `RMW_RET_ERROR` if an unspecified error occurs.
   */
  rmw_ret_t
  rmw_get_publisher_names_and_types_by_node( const rmw_node_t * node,
					     rcutils_allocator_t * allocator,
					     const char * node_name,
					     const char * node_namespace,
					     bool demangle,
					     rmw_names_and_types_t * topic_names_and_types )
  {
    if (!node) {
      RMW_SET_ERROR_MSG("node handle is null");
      return RMW_RET_INVALID_ARGUMENT;
    }
    if (!allocator) {
      RMW_SET_ERROR_MSG("allocator is null");
      return RMW_RET_INVALID_ARGUMENT;
    }
    if (!node_name) {
      RMW_SET_ERROR_MSG("node_name is null");
      return RMW_RET_INVALID_ARGUMENT;
    }
    if (!node_namespace) {
      RMW_SET_ERROR_MSG("node_namespace is null");
      return RMW_RET_INVALID_ARGUMENT;
    }
    if (!topic_names_and_types) {
      RMW_SET_ERROR_MSG("topic_names_and_types is null");
      return RMW_RET_INVALID_ARGUMENT;
    }
    
    /* Is this intended to answer for only 'local' (locally created) nodes or
       for any/all discovered node[s] too?
    */
    (void)demangle;
    
    /* NOT YET IMPLEMENTED  */
    return RMW_RET_ERROR;
  }
  
  /* ************************************************
   *  Return a list of service topic names and their types.
   * This function returns a list of service topic names and their types.
   *
   * The node parameter must not be `NULL`, and must point to a valid node.
   *
   * The topic_names_and_types parameter must be allocated and zero initialized.
   * The topic_names_and_types is the output for this function, and contains
   * allocated memory.
   * Therefore, it should be passed to rmw_names_and_types_fini() when
   * it is no longer needed.
   * Failing to do so will result in leaked memory.
   *
   * \param[in] node the handle to the node being used to query the ROS graph
   * \param[in] allocator allocator to be used when allocating space for strings
   * \param[in] node_name the name of the node to get information for
   * \param[in] node_namespace the namespace of the node to get information for
   * \param[out] topic_names_and_types list of topic names and their types the node_name has created a service for
   * \return `RMW_RET_OK` if the query was successful, or
   * \return `RMW_RET_INVALID_ARGUMENT` if the node is invalid, or
   * \return `RMW_RET_INVALID_ARGUMENT` if any arguments are invalid, or
   * \return `RMW_RET_BAD_ALLOC` if memory allocation fails, or
   * \return `RMW_RET_ERROR` if an unspecified error occurs.
   */
  rmw_ret_t
  rmw_get_service_names_and_types_by_node( const rmw_node_t * node,
					   rcutils_allocator_t * allocator,
					   const char * node_name,
					   const char * node_namespace,
					   rmw_names_and_types_t * service_names_and_types)
  {
    if (!node) {
      RMW_SET_ERROR_MSG("node handle is null");
      return RMW_RET_INVALID_ARGUMENT;
    }
    if (!allocator) {
      RMW_SET_ERROR_MSG("allocator is null");
      return RMW_RET_INVALID_ARGUMENT;
    }
    if (!node_name) {
      RMW_SET_ERROR_MSG("node_name is null");
      return RMW_RET_INVALID_ARGUMENT;
    }
    if (!node_namespace) {
      RMW_SET_ERROR_MSG("node_namespace is null");
      return RMW_RET_INVALID_ARGUMENT;
    }
    if (!service_names_and_types) {
      RMW_SET_ERROR_MSG("service_names_and_types is null");
      return RMW_RET_INVALID_ARGUMENT;
    }
    
    /* Is this intended to answer for only 'local' (locally created) nodes or
       for any/all discovered node[s] too?
    */
    
    /* NOT YET IMPLEMENTED  */
    return RMW_RET_ERROR;
  }

  
#if defined(__cplusplus)
}
#endif

