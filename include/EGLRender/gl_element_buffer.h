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
#include <vector>
#include <span>

namespace EGLRender
{

  struct GLElementBuffer
  {
    GLuint m_size = 0;
    GLenum m_primitive_type = GL_TRIANGLES;
    GLuint m_restart_index = 0;
    GLuint m_element_buffer = gen_buffer_id( m_size );

    static GLuint gen_buffer_id(GLuint sz);

    inline GLuint size() const { return m_size; }
    void resize(GLuint sz);

    GLuint* map_buffer_write_only();
    void unmap_buffer();

    void use();
    void draw( GLuint start=0, GLint count=-1, GLenum draw_primitive = GL_NONE);

    ~GLElementBuffer();
  };

}
