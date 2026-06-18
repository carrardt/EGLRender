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

#include <EGLRender/gl_frame_buffer.h>
#include <EGLRender/egl_error.h>

#include <cassert>
#include <iostream>

namespace EGLRender
{
  GLuint GLFrameBuffer::gen_buffer_id()
  {
    GLuint buf_id = 0;
    glGenFramebuffers(1, &buf_id); 
    return buf_id;
  }

  void GLFrameBuffer::bind()
  {
    glBindFramebuffer(m_bind_target,m_framebuffer);
  }

  void GLFrameBuffer::unbind()
  {
    glBindFramebuffer(m_bind_target,0);
  }
  
  void GLFrameBuffer::attach_texture(GLuint texture, GLenum attachment)
  {
    glNamedFramebufferTexture(m_framebuffer, attachment, texture, 0 );
  }

  GLFrameBuffer::~GLFrameBuffer()
  {
    glDeleteFramebuffers(1, &m_framebuffer); 
  }
}
