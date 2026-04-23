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

#include "egl_platform.h"
#include <string>
#include <iostream>

namespace EGLRender
{

  struct GLPipelineConfig
  {
    GLenum m_alpha_func = GL_ALWAYS;
    GLclampf m_alpha_func_ref = 0;

    GLenum m_blend_src = GL_ONE;
    GLenum m_blend_dst = GL_ZERO;

    GLuint m_stencil_mask = 0xFF;
    GLenum m_stencil_func = GL_ALWAYS;
    GLint m_stencil_func_ref = 0;
    GLuint m_stencil_func_mask = 0xFF;
    GLenum m_stencil_op_sfail = GL_KEEP;
    GLenum m_stencil_op_dpfail =  GL_KEEP;
    GLenum m_stencil_op_dppass = GL_KEEP;

    // customizable boolean pipeline config flags,
    // i.e. GL_PROGRAM_POINT_SIZE, GL_DEPTH_TEST, GL_ALPHA_TEST, GL_BLEND, GL_STENCIL_TEST
    std::vector<GLenum> m_enable_flags;
    std::vector<GLenum> m_disable_flags;

    void use() const;
    template<class StreamT> inline StreamT& to_stream(StreamT& out)
    {
      if( ! m_enable_flags.empty() )
      {
        out << "enable:";
        for(auto e:m_enable_flags) out << " "<< gl_enum_to_string(e);
        out << std::endl;
      }
      if( ! m_disable_flags.empty() )
      {
        out << "disable:";
        for(auto e:m_disable_flags) out << " "<< gl_enum_to_string(e);
        out << std::endl;
      }
      out << "alpha_func: " << gl_enum_to_string(m_alpha_func) << std::endl;
      out << "alpha_func_ref: " << m_alpha_func_ref << std::endl;
      out << "blend_src: " << gl_enum_to_string(m_blend_src) << std::endl;
      out << "blend_dst: " << gl_enum_to_string(m_blend_dst) << std::endl;
      out << "stencil_mask: " << m_stencil_mask << std::endl;
      out << "stencil_func: " << gl_enum_to_string(m_stencil_func) << std::endl;
      out << "stencil_func_ref: " << m_stencil_func_ref << std::endl;
      out << "stencil_func_mask: " << m_stencil_func_mask << std::endl;
      out << "stencil_op_sfail: " << gl_enum_to_string(m_stencil_op_sfail) << std::endl;
      out << "stencil_op_dpfail: " << gl_enum_to_string(m_stencil_op_dpfail) << std::endl;
      out << "stencil_op_dppass: " << gl_enum_to_string(m_stencil_op_dppass) << std::endl;
      return out;
    }
  };


  struct GLUniformVariable
  {
    static inline constexpr size_t MAX_NAME_LEN = 64;
    char m_name[MAX_NAME_LEN] = { '\0' , };
    GLint m_type = GL_FLOAT;
    GLint m_offset = 0; // bytes
    GLint m_size = 1;   // number of elements
    GLint m_stride = 4; // bytes
  };

  struct GLUniformBlock
  {
    static inline constexpr size_t MAX_NAME_LEN = 64;
    GLint m_binding = 0;
    char m_name[MAX_NAME_LEN] = { '\0' , };
    std::vector<GLUniformVariable> m_variables;
//    const GLUniformVariable& variable(int idx);
//    int variable_id(std:string_view name);
//    const GLUniformVariable& variable(std:string_view name);
  };

  // GL Shader program encapsulation
  struct GLShaderProgram
  {
    std::string m_vertex_shader_source =
  R"EOF(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        void main()
        {
            gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
        }
  )EOF";

    std::string m_geometry_shader_source = {};

    std::string m_fragment_shader_source =
  R"EOF(
        #version 330 core
        out vec4 FragColor;
        void main()
        {
            FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
        }
  )EOF";

    GLPipelineConfig m_pipeline_config = {};

    GLuint m_vertex_shader   = compile_shader(m_vertex_shader_source, GL_VERTEX_SHADER);
    GLuint m_geometry_shader = compile_shader(m_geometry_shader_source, GL_GEOMETRY_SHADER);
    GLuint m_fragment_shader = compile_shader(m_fragment_shader_source, GL_FRAGMENT_SHADER);
    GLuint m_shader_program  = link_program( m_vertex_shader, m_geometry_shader, m_fragment_shader );

    std::vector<GLUniformBlock> m_uniforms = init_uniform_blocks(m_shader_program);

    static GLuint compile_shader(const std::string& shader_source, GLenum shader_type);
    static GLuint link_program(GLuint vertShaderId, GLuint geomShaderId, GLuint fragShaderId);
    static std::vector<GLUniformBlock> init_uniform_blocks(GLuint prog);

//    const GLUniformBlock& uniform(int id);
//    int uniform_id(std:string_view name);
//    const GLUniformBlock& uniform(std:string_view name);

    void use() const;

    ~GLShaderProgram();
  };

}
