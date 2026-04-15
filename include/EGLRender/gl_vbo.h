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

//#include <onika/cuda/cuda_context.h>

namespace EGLRender
{

  template<class T, class U>
  inline bool vector_span_equal(const T& s1 , const U& s2)
  {
    return std::equal( s1.data(), s1.data()+s1.size() , s2.data(), s2.data()+s2.size() );
  }

  struct GLVertexBuffers
  {
    GLuint m_vertices = 0;
    std::vector<GLint> m_attrib_formats = {};
    std::vector<GLuint> m_vbo = gen_buffer_ids( m_vertices, m_attrib_formats );
    std::vector<void*> m_buffer_resource = { m_vbo.size() , nullptr };

    static std::vector<GLuint> gen_buffer_ids( GLuint nv, std::span<GLint> attrib_formats );

    void set_attrib_formats(std::span<const GLint> attribs);
    inline size_t number_of_attribs() const { return m_attrib_formats.size()/2; }
    inline GLint attrib_type(GLuint i) const { return m_attrib_formats[i*2]; }
    inline GLint attrib_components(GLuint i) const { return m_attrib_formats[i*2+1]; }

    inline GLuint number_of_vertices() const { return m_vertices; }
    void set_number_of_vertices(GLuint nv);

    void* host_map_write_only(GLuint index);
    void host_unmap(GLuint index);

    void* cu_map_write_only(GLuint index);
    void cu_unmap_buffer(GLuint index);

    void use();

    void delete_buffers();
    ~GLVertexBuffers();
  };

}
