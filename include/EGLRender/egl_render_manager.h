/*11
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
#include <EGLRender/egl_error.h>
#include <EGLRender/egl_renderer.h>
#include <EGLRender/egl_surface.h>
#include <EGLRender/gl_shader.h>
#include <EGLRender/gl_vbo.h>
#include <EGLRender/uniform_camera_matrix.h>

#include <map>
#include <string>
#include <string_view>
#include <span>
#include <memory>

namespace EGLRender
{

  struct EGLRenderManager
  {
    EGLRenderer* m_egl = nullptr;

    std::vector< std::shared_ptr<EGLRenderSurface> > m_surfaces;
    std::vector< std::shared_ptr<GLShaderProgram> > m_programs;
    std::vector< std::shared_ptr<GLVertexBuffers> > m_buffers;
    std::vector< std::shared_ptr<UniformCameraMatrix> > m_cameras;

    std::map<std::string,size_t> m_surf_names;
    std::map<std::string,size_t> m_prog_names;
    std::map<std::string,size_t> m_buf_names;
    std::map<std::string,size_t> m_camera_names;

    void init_platform(bool use_native_dpy=true, bool gles_mode=false);
    int create_surface( std::string_view name, EGLRenderSurfaceClass surf_type=EGLRenderSurfaceClass::PBUFFER, EGLint w=1920, EGLint h=1080, EGLint colbits=8, EGLint alphabits=8, EGLint zbits=24 , EGLint stencilbits=8);
    int create_shader_program( std::string_view name, std::string_view vs, std::string_view gs, std::string_view fs, const GLPipelineConfig& pc = GLPipelineConfig{} );
    int create_vertex_buffers( std::string_view name, GLuint n_vertices, std::span<const GLint> attrib_formats );
    int create_vertex_buffers( std::string_view name, GLuint n_vertices, const std::vector<GLint>& attrib_formats );
    int create_camera( std::string_view name );

    EGLRenderer& renderer();
    EGLNativeDisplayType native_display();

    int surface_id(int id);
    EGLRenderSurface& surface(int id);
    EGLRenderSurface& surface(std::string_view name);

    int shader_program_id(std::string_view name);
    GLShaderProgram& shader_program(int id);
    GLShaderProgram& shader_program(std::string_view name);
    std::shared_ptr<GLShaderProgram> shader_program_ptr(int id);

    int vertex_buffers_id(std::string_view name);
    GLVertexBuffers& vertex_buffers(int id);
    GLVertexBuffers& vertex_buffers(std::string_view name);

    int camera_id(std::string_view name);
    UniformCameraMatrix& camera(int id);
    UniformCameraMatrix& camera(std::string_view name);
  };

}
