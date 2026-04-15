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

namespace EGLRender
{

  struct EGLVersion
  {
    EGLint m_vermajor = 0;
    EGLint m_verminor = 0;
  };

  struct EGLRenderer
  {
    const bool m_use_native_dpy = true; // set to false if you intend to only use PBuffers and no on screen windows
    const bool m_gles = false;

    EGLNativeDisplayType m_native_dpy = init_native_display(m_use_native_dpy);
    EGLDisplay m_eglDpy = init_display(m_use_native_dpy, m_native_dpy);
    EGLVersion m_version = init_opengl_api(m_eglDpy, m_gles);

    static EGLNativeDisplayType init_native_display(bool use_native_dpy);
    static EGLDisplay init_display(bool use_native_display, EGLNativeDisplayType native_display);
    static EGLVersion init_opengl_api(EGLDisplay dpy, bool gles_mode);

    inline EGLNativeDisplayType native_display() const { return m_native_dpy; }

    void finalize();
    ~EGLRenderer();
  };

}

