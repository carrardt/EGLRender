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

#include <iostream>
#include <cstdlib>

#ifdef EGLRENDER_USE_CUDA
#include <cuda_gl_interop.h>
#define EGL_GPU_COMPUTE_API_CHECK( expr ) \
  do { auto _cu_err_code = ( expr ) ; \
  if( _cu_err_code != cudaSuccess ) { \
    std::cerr << #expr << " failed with error : "<< cudaGetErrorString(_cu_err_code) << std::endl; \
    std::abort(); } }while(0)
#define EGL_GPU_COMPUTE_GL_GET_DEVICES(countPtr,devs,nmax) cudaGLGetDevices(countPtr,devs,nmax,cudaGLDeviceListAll)
#endif

#ifdef EGLRENDER_USE_HIP
#error Not Implemented yet
#endif

#if !defined(EGLRENDER_USE_CUDA) && !defined(EGLRENDER_USE_HIP)
#define EGL_GPU_COMPUTE_API_CHECK( expr ) do{ expr; }while(0)
#define EGL_GPU_COMPUTE_GL_GET_DEVICES(countPtr,devs,nmax) do{*countPtr=0;}while(0)
#endif
