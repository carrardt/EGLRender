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

#include <EGLRender/egl_render_manager.h>
#include <iostream>
#include <string_view>
#include <ranges>

namespace EGLRender
{
  void EGLRenderManager::init_platform(bool use_native_dpy, bool gles_mode)
  {
    if( m_egl != nullptr )
    {
      std::cerr << "EGL render platform already initialized";
      std::abort();
    }
    m_egl = new EGLRenderer { use_native_dpy, gles_mode };
  }

  int EGLRenderManager::create_surface( std::string_view name, EGLRenderSurfaceClass surf_type, EGLint w, EGLint h, EGLint colbits, EGLint alphabits, EGLint zbits, EGLint stencilbits )
  {
    if( m_surf_names.find(name.data()) != m_surf_names.end() )
    {
      std::cerr << "EGL Error: a surface named '"<<name<<"' already exists"<<std::endl;
      std::abort();
    }
    const size_t surf_id = m_surfaces.size();
    m_surf_names[name.data()] = surf_id;
    m_surfaces.emplace_back( new EGLRenderSurface { m_egl, surf_type, w, h, colbits, alphabits, zbits, stencilbits, name.data() } );
#   ifndef NDEBUG
    std::cout << "EGL : surface '"<<name<<"' @"<<surf_id<<std::endl;
#   endif
    if( m_surfaces.size() == 1 )
    {
      m_surfaces.back()->make_current();
#     ifndef NDEBUG
      const GLenum sq [] = { GL_VENDOR, GL_RENDERER, GL_VERSION, GL_SHADING_LANGUAGE_VERSION, GL_EXTENSIONS , GL_NONE };
      for( auto * sqp=sq ; *sqp != GL_NONE ; sqp++ ) std::cout<< gl_enum_to_string(*sqp) << " = " << gl_string_non_null(glGetString(*sqp)) << std::endl;
#     endif
    }
    return surf_id;
  }

  int EGLRenderManager::create_shader_program( std::string_view name, std::span<GLShaderTypeSource> sources, const GLPipelineConfig& pc )
  {
    if( m_prog_names.find(name.data()) != m_prog_names.end() )
    {
      std::cerr << "EGL Error: a shader program named '"<<name<<"' already exists"<<std::endl;
      std::abort();
    }
    const size_t prog_id = m_programs.size();
    m_prog_names[name.data()] = prog_id;
    m_programs.emplace_back( new GLShaderProgram { {sources.begin(), sources.end()} , pc } );
#   ifndef NDEBUG
    std::cout << "EGL : shader program '"<<name<<"' @"<<prog_id<<std::endl;
#   endif
    return prog_id;
  }

  int EGLRenderManager::create_vertex_buffers( std::string_view name, GLuint n_vertices, std::span<const GLint> attrib_formats )
  {
    auto it = m_buf_names.find(name.data());
    if( it != m_buf_names.end() )
    {
      if( n_vertices != m_buffers[it->second]->number_of_vertices() || ! vector_span_equal(m_buffers[it->second]->m_attrib_formats,attrib_formats) )
      {
        m_buffers[it->second]->set_attrib_formats( attrib_formats );
        m_buffers[it->second]->set_number_of_vertices( n_vertices );
      }
      return it->second;
    }
    else
    {
      const size_t buffer_id = m_buffers.size();
      m_buf_names[name.data()] = buffer_id;
      m_buffers.emplace_back( new GLVertexBuffers { n_vertices, { attrib_formats.begin(), attrib_formats.end() } } );
#     ifndef NDEBUG
      std::cout << "EGL : vertex buffer '"<<name<<"' @"<<buffer_id<<std::endl;
#     endif
      return buffer_id;
    }
  }

  int EGLRenderManager::create_vertex_buffers( std::string_view name, GLuint n_vertices, const std::vector<GLint>& attrib_formats )
  {
    return create_vertex_buffers( name, n_vertices, std::span<const GLint>(attrib_formats.data(),attrib_formats.size()) );
  }

  int EGLRenderManager::create_element_buffer( std::string_view name, GLuint n_elements, GLuint restart_index )
  {
    auto it = m_elbuf_names.find(name.data());
    if( it != m_elbuf_names.end() )
    {
      if( n_elements != m_elbuffers[it->second]->size() )
      {
        m_elbuffers[it->second]->resize( n_elements );
      }
      m_elbuffers[it->second]->m_restart_index = restart_index;
      return it->second;
    }
    else
    {
      const size_t elbuffer_id = m_elbuffers.size();
      m_elbuf_names[name.data()] = elbuffer_id;
      m_elbuffers.emplace_back( new GLElementBuffer { n_elements, restart_index } );
#     ifndef NDEBUG
      std::cout << "EGL : element buffer '"<<name<<"' @"<<elbuffer_id<<std::endl;
#     endif
      return elbuffer_id;
    }
  }

  int EGLRenderManager::create_pixel_buffer( std::string_view name, GLuint width, GLuint height, GLenum format, GLenum direction )
  {
    auto it = m_pixbuf_names.find(name.data());
    if( it != m_pixbuf_names.end() )
    {
      if( width != m_pixbuffers[it->second]->width() || height != m_pixbuffers[it->second]->height())
      {
        m_pixbuffers[it->second]->resize( width, height );
      }
      return it->second;
    }
    else
    {
      const size_t pixbuffer_id = m_pixbuffers.size();
      m_pixbuf_names[name.data()] = pixbuffer_id;
      m_pixbuffers.emplace_back( new GLPixelBuffer { width, height, format, direction } );
#     ifndef NDEBUG
      std::cout << "EGL : pixel buffer '"<<name<<"' @"<<pixbuffer_id<<std::endl;
#     endif
      return pixbuffer_id;
    }    
  }

  int EGLRenderManager::create_frame_buffer( std::string_view name, GLenum fbtarget )
  {
    auto it = m_framebuf_names.find(name.data());
    if( it != m_framebuf_names.end() )
    {
      if( m_framebuffers[it->second]->m_bind_target != fbtarget )
      {
        m_framebuffers[it->second]->unbind();
        m_framebuffers[it->second]->m_bind_target = fbtarget;
      }
      return it->second;
    }
    else
    {
      const size_t framebuffer_id = m_framebuffers.size();
      m_framebuf_names[name.data()] = framebuffer_id;
      m_framebuffers.emplace_back( new GLFrameBuffer { fbtarget } );
#     ifndef NDEBUG
      std::cout << "EGL : frame buffer '"<<name<<"' @"<<framebuffer_id<<std::endl;
#     endif
      return framebuffer_id;
    }
  }

  EGLRenderer& EGLRenderManager::renderer()
  {
    return *m_egl;
  }

  EGLNativeDisplayType EGLRenderManager::native_display()
  {
    if( m_egl == nullptr )
    {
      std::cerr << "EGL Error: EGL renderer not initialized"<<std::endl;
      std::abort();
    }
    return m_egl->native_display();
  }

  int EGLRenderManager::surface_id(std::string_view name)
  {
    auto it = m_surf_names.find(name.data());
    if( it == m_surf_names.end() ) return -1;
    else return it->second;
  }

  EGLRenderSurface& EGLRenderManager::surface(int id)
  {
    if( id<0 || id>=m_surf_names.size() )
    {
      std::cerr << "EGL Error: surface id "<<id<<" is invalid"<<std::endl;
      std::abort();
    }
    return * m_surfaces[id];
  }

  EGLRenderSurface& EGLRenderManager::surface(std::string_view name)
  {
    auto it = m_surf_names.find(name.data());
    if( it == m_surf_names.end() )
    {
      std::cerr << "EGL Error: surface named "<<name<<" not found"<<std::endl;
      std::abort();
    }
    return surface(it->second);
  }

  GLShaderProgram& EGLRenderManager::shader_program(int id)
  {
    return * m_programs[id];
  }
  
  std::shared_ptr<GLShaderProgram> EGLRenderManager::shader_program_ptr(int id)
  {
    if( id<0 || id>=m_programs.size() ) return nullptr;
    return m_programs[id];
  }

  std::shared_ptr<GLShaderProgram> EGLRenderManager::shader_program_ptr(std::string_view name)
  {
    return shader_program_ptr( shader_program_id(name) );
  }

  int EGLRenderManager::shader_program_id(std::string_view name)
  {
    auto it = m_prog_names.find(name.data());
    if( it == m_prog_names.end() ) return -1;
    else return it->second;
  }

  GLShaderProgram& EGLRenderManager::shader_program(std::string_view name)
  {
    auto it = m_prog_names.find(name.data());
    if( it == m_prog_names.end() )
    {
      std::cerr << "EGL Error: camera named "<<name<<" not found"<<std::endl;
      std::abort();
    }
    return shader_program(it->second);
  }

  int EGLRenderManager::create_camera( std::string_view name )
  {
    auto it = m_camera_names.find(name.data());
    if( it != m_camera_names.end() ) return it->second;
    const auto id = m_cameras.size();
    m_camera_names[name.data()] = id;
    m_cameras.emplace_back( new UniformCameraMatrix{} );
    return id;
  }

  int EGLRenderManager::camera_id(std::string_view name)
  {
    auto it = m_camera_names.find(name.data());
    if( it != m_camera_names.end() ) return it->second;
    else return -1;
  }

  UniformCameraMatrix& EGLRenderManager::camera(int id)
  {
    return * m_cameras[id];
  }

  UniformCameraMatrix& EGLRenderManager::camera(std::string_view name)
  {
    auto it = m_camera_names.find(name.data());
    if( it == m_camera_names.end() )
    {
      std::cerr << "EGL Error: camera named "<<name<<" not found"<<std::endl;
      std::abort();
    }
    return camera(it->second);
  }

  int EGLRenderManager::vertex_buffers_id(std::string_view name)
  {
    auto it = m_buf_names.find(name.data());
    if( it != m_buf_names.end() ) return it->second;
    else return -1;
  }

  GLVertexBuffers& EGLRenderManager::vertex_buffers(int id)
  {
    return * m_buffers[id];
  }

  GLVertexBuffers& EGLRenderManager::vertex_buffers(std::string_view name)
  {
    auto it = m_buf_names.find(name.data());
    if( it == m_buf_names.end() )
    {
      std::cerr << "EGL Error: vertex buffer named "<<name<<" not found"<<std::endl;
      std::abort();
    }
    return vertex_buffers(it->second);
  }

  int EGLRenderManager::element_buffer_id(std::string_view name)
  {
    auto it = m_elbuf_names.find(name.data());
    if( it != m_elbuf_names.end() ) return it->second;
    else return -1;
  }

  GLElementBuffer& EGLRenderManager::element_buffer(int id)
  {
    return * m_elbuffers[id];
  }

  GLElementBuffer& EGLRenderManager::element_buffer(std::string_view name)
  {
    auto it = m_elbuf_names.find(name.data());
    if( it == m_elbuf_names.end() )
    {
      std::cerr << "EGL Error: element buffer named "<<name<<" not found"<<std::endl;
      std::abort();
    }
    return element_buffer(it->second);
  }

  std::shared_ptr<GLElementBuffer> EGLRenderManager::element_buffer_ptr(int id)
  {
    if( id<0 || id>=m_elbuffers.size() ) return nullptr;
    return m_elbuffers[id];
  }
  
  std::shared_ptr<GLElementBuffer> EGLRenderManager::element_buffer_ptr(std::string_view name)
  {
    return element_buffer_ptr( element_buffer_id(name) );
  }



  int EGLRenderManager::pixel_buffer_id(std::string_view name)
  {
    auto it = m_pixbuf_names.find(name.data());
    if( it != m_pixbuf_names.end() ) return it->second;
    else return -1;
  }

  GLPixelBuffer& EGLRenderManager::pixel_buffer(int id)
  {
    return * m_pixbuffers[id];
  }

  GLPixelBuffer& EGLRenderManager::pixel_buffer(std::string_view name)
  {
    auto it = m_pixbuf_names.find(name.data());
    if( it == m_pixbuf_names.end() )
    {
      std::cerr << "EGL Error: pixel buffer named "<<name<<" not found"<<std::endl;
      std::abort();
    }
    return pixel_buffer(it->second);
  }

  std::shared_ptr<GLPixelBuffer> EGLRenderManager::pixel_buffer_ptr(int id)
  {
    if( id<0 || id>=m_pixbuffers.size() ) return nullptr;
    return m_pixbuffers[id];
  }
  
  std::shared_ptr<GLPixelBuffer> EGLRenderManager::pixel_buffer_ptr(std::string_view name)
  {
    return pixel_buffer_ptr( pixel_buffer_id(name) );
  }

  int EGLRenderManager::frame_buffer_id( std::string_view name )
  {
    auto it = m_framebuf_names.find(name.data());
    if( it == m_framebuf_names.end() ) return -1;
    else return it->second;
  }
  
  GLFrameBuffer& EGLRenderManager::frame_buffer( int id )
  {
    return * m_framebuffers[id];
  }
  
  GLFrameBuffer& EGLRenderManager::frame_buffer( std::string_view name )
  {
    auto it = m_framebuf_names.find(name.data());
    if( it == m_framebuf_names.end() )
    {
      std::cerr << "EGL Error: frame buffer named "<<name<<" not found"<<std::endl;
      std::abort();
    }
    return frame_buffer(it->second);
  }

}
