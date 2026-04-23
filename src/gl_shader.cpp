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

#include <EGLRender/gl_shader.h>
#include <EGLRender/egl_error.h>
#include <iostream>
#include <memory>

namespace EGLRender
{
  void GLPipelineConfig::use() const
  {
    for(auto e:m_enable_flags) glEnable( e );
    for(auto e:m_disable_flags) glDisable( e );    
    glAlphaFunc( m_alpha_func , m_alpha_func_ref );
    glBlendFunc( m_blend_src , m_blend_dst );
    glStencilMask( m_stencil_mask );
    glStencilFunc( m_stencil_func, m_stencil_func_ref, m_stencil_func_mask  );
    glStencilOp( m_stencil_op_sfail, m_stencil_op_dpfail, m_stencil_op_dppass );
  }

  GLuint GLShaderProgram::compile_shader(const std::string& shader_source, GLenum shader_type)
  {
    const char * src [] = { shader_source.data() };
    if( shader_source.empty() ) return 0;
    GLuint shaderId = glCreateShader(shader_type);
    glShaderSource(shaderId, 1, src, NULL);
    glCompileShader(shaderId);
    GLint compile_status = 0;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compile_status);
    if( ! compile_status )
    {
      GLint info_log_len = 0;
      glGetShaderiv(shaderId,GL_INFO_LOG_LENGTH,&info_log_len);
      auto log_data = std::make_unique_for_overwrite<char[]>(info_log_len+2);
      glGetShaderInfoLog(shaderId,info_log_len,&info_log_len,log_data.get());      
      log_data[info_log_len] = '\0';
      std::cerr << "Shader #"<<shaderId<<" : " << ( (info_log_len>0) ? log_data.get() : "ok" ) << std::endl;
    }
#   ifndef NDEBUG
    else
    {
      std::cout << "Shader #"<<shaderId<<" Ok"<<std::endl;
    }
#   endif
    return shaderId;
  }

  GLuint GLShaderProgram::link_program(GLuint vertShaderId, GLuint geomShaderId, GLuint fragShaderId)
  {
    if( vertShaderId==0 && geomShaderId==0 && fragShaderId==0 ) return 0;

    GLuint prog = glCreateProgram();
    if( vertShaderId != 0 ) glAttachShader(prog, vertShaderId);
    if( geomShaderId != 0 ) glAttachShader(prog, geomShaderId);
    if( fragShaderId != 0 ) glAttachShader(prog, fragShaderId);
    glLinkProgram(prog);

    GLint link_status = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &link_status);
    if( ! link_status )
    {
      GLint info_log_len = 0;
      glGetProgramiv(prog,GL_INFO_LOG_LENGTH,&info_log_len);
      auto log_data = std::make_unique_for_overwrite<char[]>(info_log_len+2);
      glGetProgramInfoLog(prog,info_log_len,&info_log_len,log_data.get());
      log_data[info_log_len] = '\0';
      std::cerr << "Program #"<<prog<<" : " << ( (info_log_len>0) ? log_data.get() : "ok" ) << std::endl;
    }
#   ifndef NDEBUG
    else
    {
      std::cout << "Program #"<<prog<<" Ok"<<std::endl;
    }
#   endif

    return prog;
  }

  std::vector<GLUniformBlock> GLShaderProgram::init_uniform_blocks(GLuint prog)
  {
    std::vector<GLUniformBlock> blocks;
    
    // gather information about shader program
    GLint uniform_block_count = 0; 
    glGetProgramiv(prog, GL_ACTIVE_UNIFORM_BLOCKS, &uniform_block_count);
    for(int i=0; i<uniform_block_count; i++)
    {
      GLint b=0;
      glGetActiveUniformBlockiv(prog, i, GL_UNIFORM_BLOCK_BINDING, &b);
      if( b >= blocks.size() ) blocks.resize( b+1 );
      
      blocks[b].m_binding = b;
      glGetActiveUniformBlockName(prog, i, blocks[b].MAX_NAME_LEN , nullptr, blocks[b].m_name );
      
      GLint variable_count= 0;
      glGetActiveUniformBlockiv(prog, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &variable_count);
      blocks[b].m_variables.assign( variable_count, GLUniformVariable{} );
      
      std::vector<GLint> variables( variable_count , 0 );
      glGetActiveUniformBlockiv(prog, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, variables.data() );
      for(int k= 0; k < variable_count; k++)
      {
        glGetActiveUniformName(prog, variables[k], blocks[b].m_variables[k].MAX_NAME_LEN , nullptr, blocks[b].m_variables[k].m_name );
        glGetActiveUniformsiv(prog, 1, (GLuint *) &variables[k], GL_UNIFORM_TYPE, & blocks[b].m_variables[k].m_type );
        glGetActiveUniformsiv(prog, 1, (GLuint *) &variables[k], GL_UNIFORM_OFFSET, & blocks[b].m_variables[k].m_offset);
        glGetActiveUniformsiv(prog, 1, (GLuint *) &variables[k], GL_UNIFORM_SIZE, & blocks[b].m_variables[k].m_size);
        glGetActiveUniformsiv(prog, 1, (GLuint *) &variables[k], GL_UNIFORM_ARRAY_STRIDE, & blocks[b].m_variables[k].m_stride);
      }
    }

#   ifndef NDEBUG
    std::cout << blocks.size() << " uniform blocks :" << std::endl;
    for(const auto & b : blocks)
    {
      std::cout << "  block '" << b.m_name <<"' bound to index #"<<b.m_binding<<std::endl;
      for(const auto & v : b.m_variables)
      {
        std::cout << "    variable '"<<v.m_name<<"' type="<<gl_enum_to_string(v.m_type)<<", offset="<<v.m_offset<<", size="<<v.m_size<<", stride="<<v.m_stride<<std::endl;
      }
    }
#endif
    
    return blocks;
  }

  void GLShaderProgram::use() const
  {
    m_pipeline_config.use();
    glUseProgram( m_shader_program );
  }

  GLShaderProgram::~GLShaderProgram()
  {
    glUseProgram(0);
    glDeleteProgram(m_shader_program);
    glDeleteShader(m_vertex_shader);
    glDeleteShader(m_geometry_shader);
    glDeleteShader(m_fragment_shader);
  }

}
