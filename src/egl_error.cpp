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

#include <EGLRender/egl_error.h>

#include <iostream>
#include <cassert>

namespace EGLRender
{

  // EGL Error checking
  void check_egl_error( EGLint rc , const char * srcfile , int lineno )
  {
    if( rc == EGL_SUCCESS ) return;
    switch( rc )
    {
  #   define EGL_ERROR_CASE(EGLERR) case EGLERR : std::cerr << "EGL Error: " << #EGLERR << std::endl; break
      EGL_ERROR_CASE(EGL_NOT_INITIALIZED);
      EGL_ERROR_CASE(EGL_BAD_ACCESS);
      EGL_ERROR_CASE(EGL_BAD_ALLOC);
      EGL_ERROR_CASE(EGL_BAD_ATTRIBUTE);
      EGL_ERROR_CASE(EGL_BAD_CONTEXT);
      EGL_ERROR_CASE(EGL_BAD_CONFIG);
      EGL_ERROR_CASE(EGL_BAD_CURRENT_SURFACE);
      EGL_ERROR_CASE(EGL_BAD_DISPLAY);
      EGL_ERROR_CASE(EGL_BAD_SURFACE);
      EGL_ERROR_CASE(EGL_BAD_MATCH);
      EGL_ERROR_CASE(EGL_BAD_PARAMETER);
      EGL_ERROR_CASE(EGL_BAD_NATIVE_PIXMAP);
      EGL_ERROR_CASE(EGL_BAD_NATIVE_WINDOW);
      EGL_ERROR_CASE(EGL_CONTEXT_LOST);
  #   undef EGL_ERROR_CASE
      default: std::cerr << "EGL Error: unknown error #"<<rc<<std::endl; break;
    }
    std::abort();
  }

}

