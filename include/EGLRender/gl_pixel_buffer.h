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
  struct GLPixelBuffer
  {
    GLuint m_width = 0;
    GLuint m_height = 0;
    GLenum m_format = GL_RGBA;
    GLenum m_direction = GL_PIXEL_PACK_BUFFER;
    GLuint m_pixel_buffer = gen_buffer_id( m_width, m_height, m_format, m_direction );
    GLuint m_texture = 0;

    static GLuint gen_buffer_id(GLuint width, GLuint height, GLenum format, GLenum direction);

    inline GLuint width() const { return m_width; }
    inline GLuint height() const { return m_height; }
    inline GLenum format() const { return m_format; }
    GLenum data_type() const;
    inline GLuint data_size() const { return m_width*m_height*4; }
    void resize(GLuint w, GLuint h);

    const void* map_buffer_read_only();
    void* map_buffer_write_only();
    void unmap_buffer();

    void use();
    void unuse();
    void read_pixels();
    GLuint copy_to_texture();

    ~GLPixelBuffer();
  };

}
