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

#include <EGLRender/egl_renderer.h>
#include <EGLRender/egl_error.h>

#include <iostream>
#include <cstdlib>

namespace EGLRender
{

  EGLNativeDisplayType EGLRenderer::init_native_display(bool use_native_dpy)
  {
    if( use_native_dpy ) return platform_get_native_display();
    else return NULL;
  }

  EGLDisplay EGLRenderer::init_display(bool use_native_display, EGLNativeDisplayType native_display)
  {
    // Initialize EGL
    EGLDisplay dpy = eglGetDisplay( use_native_display ? native_display : EGL_DEFAULT_DISPLAY );
    EGL_CHECK_ERROR();
    return dpy;
  }

  EGLVersion EGLRenderer::init_opengl_api(EGLDisplay dpy, bool gles_mode)
  {
    EGLVersion version;
    if( ! eglInitialize( dpy, & version.m_vermajor, & version.m_verminor) )
    {
      std::cerr << "Failed to initialize EGL"<<std::endl;
      std::abort();
    }

    // Bind the API
    if( ! eglBindAPI( gles_mode ?  EGL_OPENGL_ES_API : EGL_OPENGL_API) )
    {
      std::cerr<<"Could not bind OpenGL API"<<std::endl;
      std::abort();
    }
#   ifndef NDEBUG
    std::cout<<"*** EGL ***"<<std::endl;
    std::cout<<"Vendor     : "<<eglQueryString(dpy,EGL_VENDOR)<<std::endl;
    std::cout<<"Version    : "<<version.m_vermajor<<"."<<version.m_verminor<<" ("<<eglQueryString(dpy,EGL_VERSION)<<")"<<std::endl;
    std::cout<<"Extensions : "<<eglQueryString(dpy,EGL_EXTENSIONS)<<std::endl;
#   endif

    return version;
  }

  void EGLRenderer::finalize()
  {
    eglTerminate(m_eglDpy);
    EGL_CHECK_ERROR();
  }

  EGLRenderer::~EGLRenderer()
  {
    finalize();
  }

}

