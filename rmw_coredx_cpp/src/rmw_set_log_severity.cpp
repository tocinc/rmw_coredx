// Copyright 2019 Twin Oaks Computing, Inc.
// Modifications copyright (C) 2017 Twin Oaks Computing, Inc.
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

#include "rmw/rmw.h"

#if defined(__cplusplus)
extern "C" {
#endif

  rmw_ret_t
  rmw_set_log_severity(rmw_log_severity_t severity)
  {
    /* what is expected? */
    (void)severity;
    return RMW_RET_OK;
  }


#if defined(__cplusplus)
}
#endif
