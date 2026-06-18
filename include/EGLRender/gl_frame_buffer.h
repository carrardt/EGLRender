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

  // 32-bist only pixel/depth buffer, either GL_RGBA, GL_DEPTH_COMPONENT or GL_DEPTH_STENCIL
  struct GLFrameBuffer
  {
    GLEnum m_bound_target = GL_NONE;
    GLuint m_framebuffer = gen_buffer_id();
    static GLuint gen_buffer_id();

    void bind_to(GLenum fbtarget);
    void unbind();
    void attach_texture(GLenum attachment, GLuint texture);

    ~GLFrameBuffer();
  };

}
