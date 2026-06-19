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

#include <EGLRender/gl_pixel_buffer.h>
#include <EGLRender/egl_error.h>

#include <cassert>
#include <iostream>

namespace EGLRender
{
  // we handle 32-bits only data types
  GLenum GLPixelBuffer::data_type() const
  {
    if( m_format == GL_RGBA ) return GL_UNSIGNED_BYTE;
    else if( m_format == GL_DEPTH_COMPONENT ) return GL_FLOAT;
    else if( m_format == GL_DEPTH_STENCIL ) return GL_UNSIGNED_INT_24_8;
    else return GL_NONE;
  }

  GLuint GLPixelBuffer::gen_buffer_id( GLuint width, GLuint height, GLenum format, GLenum direction )
  {
    GLuint sz = width * height;
    GLenum dtype = GL_NONE;
    if( format == GL_RGBA ) dtype = GL_UNSIGNED_BYTE;
    else if( format == GL_DEPTH_COMPONENT ) dtype = GL_FLOAT;
    else if( format == GL_DEPTH_STENCIL ) dtype = GL_UNSIGNED_INT_24_8;
    else
    {
      std::cerr << "pixel buffer format invalid : "<<gl_enum_to_string(format)<<std::endl;
      std::abort();
    }

    GLuint storage_hint = 0;
    if( direction == GL_PIXEL_PACK_BUFFER )
    {
      storage_hint = GL_MAP_READ_BIT;
    }
    else if( direction == GL_PIXEL_UNPACK_BUFFER )
    {
      storage_hint = GL_MAP_WRITE_BIT;
    }
    else
    {
      std::cerr<<"invalid pixel buffer direction : "<<gl_enum_to_string(direction)<<std::endl;
      std::abort();
    }
    
    GLuint buf_id = 0;
    glGenBuffers( 1 , & buf_id );
    glBindBuffer( direction, buf_id );    
    glNamedBufferStorage(buf_id, sz * sizeof(GLuint), NULL,  /*GL_DYNAMIC_STORAGE_BIT |*/ storage_hint );
    return buf_id;
  }

  void GLPixelBuffer::resize(GLuint w, GLuint h)
  {
    static_assert( sizeof(GLuint)==4 && sizeof(GLfloat)==4 );
    static constexpr size_t data_type_sz = 4;
    if( w != m_width || h != m_height )
    {
      m_width=w; m_height=h;
      GLuint sz = m_width * m_height;
      glBindBuffer( m_direction, m_pixel_buffer);
      glNamedBufferStorage(m_pixel_buffer, sz * data_type_sz, NULL, ( m_direction == GL_PIXEL_PACK_BUFFER ) ? GL_MAP_READ_BIT : GL_MAP_WRITE_BIT );
    }
  }

  void * GLPixelBuffer::map_buffer_write_only()
  {
    return glMapNamedBuffer(m_pixel_buffer, GL_WRITE_ONLY);
  }

  const void * GLPixelBuffer::map_buffer_read_only()
  {
    return glMapNamedBuffer(m_pixel_buffer, GL_READ_ONLY) ;
  }

  void GLPixelBuffer::unmap_buffer()
  {
    glUnmapNamedBuffer(m_pixel_buffer);
  }

  void GLPixelBuffer::use()
  {
    glBindBuffer(m_direction, m_pixel_buffer);
  }
  void GLPixelBuffer::unuse()
  {
    glBindBuffer(m_direction, 0);
  }

  GLuint GLPixelBuffer::copy_to_texture()
  {
    if( m_texture == 0 )
    {
      glGenTextures(1,&m_texture);
      glBindTexture(GL_TEXTURE_2D,m_texture);
      use();
      glTexImage2D(GL_TEXTURE_2D,0,m_format,m_width,m_height,0,m_format,data_type(),NULL);
    }
    else
    {
      glBindTexture(GL_TEXTURE_2D,m_texture);
      use();
      glTexSubImage2D(GL_TEXTURE_2D,0,0,0,m_width,m_height,m_format,data_type(),NULL);
    }
    return m_texture;
  }

  void GLPixelBuffer::read_pixels()
  {
    use();
#ifndef NDEBUG
    std::cout << "read pixels : size="<<m_width<<"x"<<m_height<<", format="<<gl_enum_to_string(m_format)<<", type="<<gl_enum_to_string(data_type())<<std::endl;
#endif
    glReadPixels(0, 0, m_width, m_height, m_format, data_type(), NULL );
  }

  GLPixelBuffer::~GLPixelBuffer()
  {
    unuse();
    glDeleteBuffers( 1 , & m_pixel_buffer );
  }
}
