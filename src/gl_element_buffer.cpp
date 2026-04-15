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

#include <EGLRender/gl_element_buffer.h>
#include <EGLRender/egl_error.h>

#include <cassert>
#include <iostream>

namespace EGLRender
{
  GLuint GLElementBuffer::gen_buffer_id( GLuint sz )
  {
    GLuint buf_id = 0;
    glGenBuffers( 1 , & buf_id );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buf_id );
    glNamedBufferStorage(buf_id, sz*sizeof(GLuint), NULL,  /*GL_DYNAMIC_STORAGE_BIT |*/ GL_MAP_WRITE_BIT );
    return buf_id;
  }

  void GLElementBuffer::resize(GLuint sz)
  {
    if( sz != m_size )
    {
      m_size = sz;
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_element_buffer);
      glNamedBufferStorage(m_element_buffer, sz*sizeof(GLuint), NULL,  /*GL_DYNAMIC_STORAGE_BIT |*/ GL_MAP_WRITE_BIT );
    }
  }

  GLuint * GLElementBuffer::map_buffer_write_only()
  {
    return reinterpret_cast<GLuint*>( glMapNamedBuffer(m_element_buffer, GL_WRITE_ONLY) );
  }

  void GLElementBuffer::unmap_buffer()
  {
    glUnmapNamedBuffer(m_element_buffer);
  }

  void GLElementBuffer::use()
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_element_buffer);
    if( m_restart_index > 0 )
    {
      glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
      glPrimitiveRestartIndex(m_restart_index);
    }
    else
    {
      glDisable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    }
  }

  void GLElementBuffer::draw(GLuint start, GLint count, GLenum draw_primitive)
  {
    if( draw_primitive == GL_NONE ) draw_primitive = m_primitive_type;
    if(count == -1) count = m_size;
    use();
    glDrawElements( draw_primitive, count, GL_UNSIGNED_INT, reinterpret_cast<const void*>(start*sizeof(GLuint)) );
  }

  GLElementBuffer::~GLElementBuffer()
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    glDeleteBuffers( 1 , & m_element_buffer );
  }
}
