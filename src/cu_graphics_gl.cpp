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

#include <EGLRender/cu_graphics_gl.h>
#include <cassert>
#include <iostream>

#ifdef EGLRENDER_USE_CUDA
#include <cuda_gl_interop.h>
#endif

#ifdef EGLRENDER_USE_HIP
#error Not Implemented yet
#endif

namespace EGLRender
{
  void egl_gpu_compute_check_error( int err_code, const char* expr_str, const char* src_file, int lineno )
  {
#   ifdef EGLRENDER_USE_CUDA
    cudaError_t cuerr = *reinterpret_cast<cudaError_t*>(&err_code);
    if( cuerr != cudaSuccess )
    {
      std::cerr << expr_str << " failed with error : "<< cudaGetErrorString(cuerr) << std::endl;
      std::cerr << "in file "<<src_file<<" at line "<<lineno<<std::endl;
      std::abort();
    }
#   endif
  }

  void egl_gpu_compute_gl_get_devices(int* countPtr, int * devicesPtr , int maxDeviceCount)
  {
    *countPtr = 0;
#   ifdef EGLRENDER_USE_CUDA
    unsigned int ndev = 0;
    EGL_GPU_COMPUTE_CHECK_ERROR( cudaGLGetDevices(&ndev,devicesPtr,maxDeviceCount,cudaGLDeviceListAll) );
    *countPtr = ndev;
#   endif
  }

  void* egl_gpu_compute_gl_register_buffer(GLuint buffer)
  {
    void* resource_handle = nullptr;
#   ifdef EGLRENDER_USE_CUDA
    cudaGraphicsResource_t cu_res = nullptr;
    EGL_GPU_COMPUTE_CHECK_ERROR( cudaGraphicsGLRegisterBuffer ( & cu_res , buffer, cudaGraphicsRegisterFlagsWriteDiscard ) );
    resource_handle = cu_res;
#   endif
    return resource_handle;
  }

  void* egl_gpu_compute_map_resource_ptr( void* resource_handle, void* gpu_stream )
  {
    void * gpu_dev_ptr = nullptr;
#   ifdef EGLRENDER_USE_CUDA
    cudaGraphicsResource_t cu_res = (cudaGraphicsResource_t) resource_handle;
    EGL_GPU_COMPUTE_CHECK_ERROR( cudaGraphicsMapResources(1, & cu_res, (cudaStream_t) gpu_stream ) );
    size_t map_sz = 0;
    EGL_GPU_COMPUTE_CHECK_ERROR( cudaGraphicsResourceGetMappedPointer( & gpu_dev_ptr, & map_sz, cu_res ) );
#   endif
    return gpu_dev_ptr;
  }

  void egl_gpu_compute_unmap_resource( void* resource_handle, void* gpu_stream )
  {
#   ifdef EGLRENDER_USE_CUDA
    cudaGraphicsResource_t cu_res = (cudaGraphicsResource_t) resource_handle;
    EGL_GPU_COMPUTE_CHECK_ERROR( cudaGraphicsUnmapResources(1, &cu_res, (cudaStream_t) gpu_stream ) );
#   endif
  }

  void egl_gpu_compute_unregister_resource( void* resource_handle )
  {
#   ifdef EGLRENDER_USE_CUDA
    EGL_GPU_COMPUTE_CHECK_ERROR( cudaGraphicsUnregisterResource( (cudaGraphicsResource_t) resource_handle ) );
#   endif
  }

}
