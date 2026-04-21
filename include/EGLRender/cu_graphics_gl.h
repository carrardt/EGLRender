/*
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
*/

#pragma once

#include <EGLRender/egl_platform.h>

#define EGL_GPU_COMPUTE_CHECK_ERROR( expr ) ::EGLRender::egl_gpu_compute_check_error( (expr) , #expr , __FILE__ , __LINE__ )

namespace EGLRender
{
  void egl_gpu_compute_check_error( int err_code, const char* expr_str="", const char* src_file="", int lineno=0 );
  void egl_gpu_compute_gl_get_devices(int* countPtr, int * devicesPtr , int maxDeviceCount);

  void* egl_gpu_compute_gl_register_buffer( GLuint buffer );
  void* egl_gpu_compute_map_resource_ptr( void* resource_handle, void* gpu_stream_handle=nullptr );
  void egl_gpu_compute_unmap_resource( void* resource_handle, void* gpu_stream_handle=nullptr );
  void egl_gpu_compute_unregister_resource( void* resource_handle );

}

