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

#include <EGL/egl.h>
#define EGL_EGLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#include <EGL/eglext.h>
#include <GL/gl.h>

#include <string_view>
#include <EGLRender/native_window_event_handler.h>

namespace EGLRender
{

  EGLNativeDisplayType platform_get_native_display();
  EGLNativeWindowType platform_get_native_window(EGLNativeDisplayType native_display, int w, int h, bool fullscreen, std::string_view name = "EGL");
  void platform_native_window_process_events(EGLNativeDisplayType dpy, EGLNativeWindowType win, const NativeWindowEventHandler& cb);

  const char * gl_string_non_null(const GLubyte* s);
  GLuint gl_type_bytes(GLenum t);
  const std::string& gl_enum_to_string(GLenum t);
  GLenum gl_enum_from_string(std::string_view name);
  GLenum gl_enum_from_string(const std::string& name);
  bool string_is_gl_enum(std::string_view name);
}
