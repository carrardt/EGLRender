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

#include <EGLRender/gl_vbo.h>
#include <EGLRender/egl_error.h>

#include <cassert>
#include <iostream>

#ifdef EGLRENDER_USE_CUDA
#include <cuda_gl_interop.h>
#define EGL_GPU_COMPUTE_API_CHECK( expr ) \
  do { auto _cu_err_code = ( expr ) ; \
  if( _cu_err_code != cudaSuccess ) { \
    std::cerr << #expr << " failed with error : "<< cudaGetErrorString(_cu_err_code) << std::endl; \
    std::abort(); } }while(0)
#endif

#ifndef EGL_GPU_COMPUTE_API_CHECK
#define EGL_GPU_COMPUTE_API_CHECK( expr ) (void)0
#endif

namespace EGLRender
{
  std::vector<GLuint> GLVertexBuffers::gen_buffer_ids( GLuint nv, std::span<GLint> attrib_formats )
  {
    assert( attrib_formats.size()%2 == 0 );
    const size_t n_vbos = attrib_formats.size()/2;
    std::vector<GLuint> vbo(n_vbos,0);
    glGenBuffers(n_vbos, vbo.data());
    for(size_t i=0;i<n_vbos;i++)
    {
      const GLuint typesz = gl_type_bytes(attrib_formats[i*2]);
      const GLuint nc = attrib_formats[i*2+1];
#     ifndef NDEBUG
      std::cout<<"buffer "<<i<<" : "<<nv<<"x"<<gl_enum_to_string(attrib_formats[i*2])<<"x"<<nc<<std::endl;
#     endif
      glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
      glNamedBufferStorage(vbo[i], nv*nc*typesz, NULL,  /*GL_DYNAMIC_STORAGE_BIT |*/ GL_MAP_WRITE_BIT );
    }
    return vbo;
  }

  void GLVertexBuffers::delete_buffers()
  {
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glDeleteBuffers(m_vbo.size(), m_vbo.data());
    m_vbo.clear();
  }

  void GLVertexBuffers::set_attrib_formats(std::span<const GLint> attribs)
  {
    if( ! vector_span_equal(m_attrib_formats,attribs) )
    {
#     ifndef NDEBUG
      std::cout << "rebuild vertex buffers from new attrb formats"<<std::endl;
#     endif
      delete_buffers();
      m_attrib_formats.assign( attribs.begin() , attribs.end() );
      m_vbo = gen_buffer_ids( 0 , m_attrib_formats );
    }
  }

  void GLVertexBuffers::set_number_of_vertices(GLuint nv)
  {
    const size_t n_vbos = m_vbo.size();
    assert( m_attrib_formats.size() == n_vbos*2 );
    if( m_vertices != nv )
    {
#     ifndef NDEBUG
      std::cout << "resize vertex buffers to "<<nv<<std::endl;
#     endif
      m_vertices = nv;
      for(size_t i=0;i<n_vbos;i++)
      {
        const GLuint typesz = gl_type_bytes(m_attrib_formats[i*2]);
        const GLuint nc = m_attrib_formats[i*2+1];
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo[i]);
        glNamedBufferStorage(m_vbo[i], nv*nc*typesz, NULL,  /*GL_DYNAMIC_STORAGE_BIT |*/ GL_MAP_WRITE_BIT );
      }
    }
  }

  void* GLVertexBuffers::host_map_write_only(GLuint index)
  {
    return glMapNamedBuffer(m_vbo[index], GL_WRITE_ONLY);
  }

  void GLVertexBuffers::host_unmap(GLuint index)
  {
    glUnmapNamedBuffer(m_vbo[index]);
  }

  void* GLVertexBuffers::gpu_map_write_only(GLuint index, void* gpu_stream)
  {
#   ifndef EGLRENDER_GPU_COMPUTE_API
    return nullptr;
#   endif
    if( index >= m_buffer_resource.size() )
    {
      m_buffer_resource.resize( index + 1 , nullptr );
    }
    if( m_buffer_resource[index] == nullptr )
    {
#     ifdef EGLRENDER_USE_CUDA
      cudaGraphicsResource_t cu_res = nullptr;
      EGL_GPU_COMPUTE_API_CHECK( cudaGraphicsGLRegisterBuffer ( & cu_res , m_vbo[index], cudaGraphicsRegisterFlagsWriteDiscard ) );
      m_buffer_resource[index] = cu_res;
#     endif
    }
    if( m_buffer_resource[index] == nullptr )
    {
      std::cerr << "Internal error: unsupported GPU compute API" << std::endl;
      std::abort();
    }
#   ifdef EGLRENDER_USE_CUDA
    cudaGraphicsResource_t cu_res = (cudaGraphicsResource_t) m_buffer_resource[index];
    EGL_GPU_COMPUTE_API_CHECK( cudaGraphicsMapResources (1, & cu_res, (cudaStream_t) gpu_stream ) );
    void * gpu_dev_ptr = nullptr;
    size_t map_sz = 0;
    EGL_GPU_COMPUTE_API_CHECK( cudaGraphicsResourceGetMappedPointer( & gpu_dev_ptr, & map_sz, cu_res ) );
    return gpu_dev_ptr;
#   endif
    return nullptr;
  }
  
  void GLVertexBuffers::gpu_unmap(GLuint index, void* gpu_stream)
  {
#   ifdef EGLRENDER_GPU_COMPUTE_API
#     ifdef EGLRENDER_USE_CUDA
      EGL_GPU_COMPUTE_API_CHECK( cudaGraphicsUnmapResources(1, (cudaGraphicsResource_t*) & m_buffer_resource[index], (cudaStream_t) gpu_stream ) );
#     else
      std::cerr << "Internal error: unsupported operation" << std::endl;
      std::abort();
#     endif
#   endif
  }

  void GLVertexBuffers::use()
  {
    const size_t n_vbos = m_vbo.size();
    for(size_t i=0;i<n_vbos;i++)
    {
      glBindBuffer(GL_ARRAY_BUFFER, m_vbo[i]);
      glEnableVertexAttribArray(i);
      glVertexAttribPointer( i, m_attrib_formats[i*2+1] , m_attrib_formats[i*2], GL_FALSE, 0, 0);
    }
  }

  GLVertexBuffers::~GLVertexBuffers()
  {
    delete_buffers();
  }
}
