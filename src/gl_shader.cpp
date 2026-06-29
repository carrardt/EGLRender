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
#include <mutex>
#include <cassert>
#include <regex>
#include <format>

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

  std::string GLShaderProgram::parse_shader_code(const std::string& shader_source, std::map<std::string,int>& auto_binding_map)
  {
    static const char * include_extension_mstr =
      "[ \t]*#[ \t]*extension[ \t]+GL_ARB_shading_language_include[^\n]*\n" ;
    static const char * commented_include_extension_rstr =
      "// removed #extension GL_ARB_shading_language_include\n" ;
    static const char * include_mstr =
      "#include[[:space:]]+[<\"](.+)[>\"][[:space:]]*\n";
    static const char * auto_uniform_binding_mstr =
      "layout[[:space:]]*\\([^\\)]*binding[[:space:]]*=[[:space:]]*(auto)[^\\)]*\\)[[:space:]]*uniform[[:space:]]+([a-zA-Z_][0-9a-zA-Z_]*)[[:space:]]*\\{" ;

    // find and suppress langue include extension directive
    std::string input = shader_source;
    auto re = std::regex(include_extension_mstr);
    input = std::regex_replace(input,re,commented_include_extension_rstr);
    
    // find include directives and replace them with include content
    re = std::regex(include_mstr);
    for( std::smatch sm ; std::regex_search(input, sm, re) ; )
    {
      // sm[0] is the whole match
      // sm[1] is the submatch corresponding to include file name in between <> or ""
      const auto incname = sm[1].str();
      // std::cout << "replacing include '"<<incname<<"' with registered content"<<std::endl;
      input = input.replace( sm.position() , sm.length() , platform_get_named_string(incname) );
    }
    
    // find uniform single declaration or uniform blocks with auto binding
    re = std::regex(auto_uniform_binding_mstr);
    for( std::smatch sm ; std::regex_search(input, sm, re); )
    {
      // sm[0] is the whole match
      // sm[1] is the auto keyword
      // sm[2] is the first identifier : either uniform variable type or uniform block name
      assert( sm.size() == 3 );
      std::string name = sm[2].str();
      int bp = -1;
      auto it = auto_binding_map.find(name);
      if( it == auto_binding_map.end() )
      {
        bp = auto_binding_map.size();
        auto_binding_map[name] = bp;
      }
      else bp = it->second;
      // std::cout << "auto binding for uniform block '"<<name<<"' = "<<bp<<std::endl;
      size_t pos = sm[1].first - input.begin();
      size_t len = sm[1].length();
      input = input.replace( pos , len , std::to_string(bp) );
    }

    return input;
  }

  std::vector<GLuint> GLShaderProgram::compile_shaders(std::span<GLShaderTypeSource> shader_sources, std::map<std::string,int>& auto_binding_map)
  {
    std::vector<GLuint> shader_ids;
    for(const auto& shader : shader_sources)
    {
      auto parsed_shader_source = parse_shader_code(shader.m_source,auto_binding_map);
      if( ! parsed_shader_source.empty() )
      {
        const char * src [] = { parsed_shader_source.data() };
        GLuint shaderId = glCreateShader(shader.m_type);
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
          if( info_log_len > 0 )
          {
            const std::string errstr = log_data.get();
            std::cerr << gl_enum_to_string(shader.m_type) <<" #"<<shaderId<<" : " << errstr;
            long lpos=0, next_pos=0;
            int l = 1;
            while( next_pos != std::string::npos )
            {
              next_pos = parsed_shader_source.find('\n',lpos);
              std::string line_str = parsed_shader_source.substr( lpos, next_pos - lpos );
              std::cerr<< std::format("{:5} : ",l) << line_str << std::endl;
              lpos = next_pos+1;
              ++l;
            }
          }
          glDeleteShader( shaderId );
        }
        else
        {
#         ifndef NDEBUG
          std::cout << "Shader #"<<shaderId<<" Ok"<<std::endl;
#         endif
          shader_ids.push_back( shaderId );
        }
      }
    }
    return shader_ids;
  }

  GLuint GLShaderProgram::link_program(std::span<GLuint> shaders)
  {
    if( shaders.empty() ) return 0;
    GLuint prog = glCreateProgram();
    for(auto shid : shaders) glAttachShader(prog, shid);
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
      std::cerr << "Program #"<<prog<<" link error : " << ( (info_log_len>0) ? log_data.get() : "ok" ) << std::endl;
      glDeleteProgram(prog);
      return 0;
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
    blocks.clear();
    blocks.resize(uniform_block_count);
    for(int b=0; b<uniform_block_count; b++)
    {
      glGetActiveUniformBlockiv(prog, b, GL_UNIFORM_BLOCK_BINDING, & blocks[b].m_binding);
      glGetActiveUniformBlockName(prog, b, blocks[b].MAX_NAME_LEN , nullptr, blocks[b].m_name );

      GLint variable_count= 0;
      glGetActiveUniformBlockiv(prog, b, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &variable_count);
      blocks[b].m_variables.assign( variable_count, GLUniformVariable{} );

      std::vector<GLint> variables( variable_count , 0 );
      glGetActiveUniformBlockiv(prog, b, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, variables.data() );
      for(int k= 0; k < variable_count; k++)
      {
        glGetActiveUniformName(prog, variables[k], blocks[b].m_variables[k].MAX_NAME_LEN , nullptr, blocks[b].m_variables[k].m_name );
        auto aspos = std::string_view(blocks[b].m_variables[k].m_name).find("[0]");
        if( aspos != std::string::npos ) { blocks[b].m_variables[k].m_name[aspos]='\0'; }
        glGetActiveUniformsiv(prog, 1, (GLuint *) &variables[k], GL_UNIFORM_TYPE, & blocks[b].m_variables[k].m_type );
        glGetActiveUniformsiv(prog, 1, (GLuint *) &variables[k], GL_UNIFORM_OFFSET, & blocks[b].m_variables[k].m_offset);
        glGetActiveUniformsiv(prog, 1, (GLuint *) &variables[k], GL_UNIFORM_SIZE, & blocks[b].m_variables[k].m_size);
        glGetActiveUniformsiv(prog, 1, (GLuint *) &variables[k], GL_UNIFORM_ARRAY_STRIDE, & blocks[b].m_variables[k].m_stride);
      }
    }

#   ifndef NDEBUG
    GLint atomic_buffers = 0;
    glGetProgramiv(prog, GL_ACTIVE_ATOMIC_COUNTER_BUFFERS, &atomic_buffers);
    std::cout << "shader program #"<<prog<<" details :"<<std::endl;
    std::cout << atomic_buffers << " atomic buffers" << std::endl;

    GLint active_uniforms = 0;
    glGetProgramiv(prog, GL_ACTIVE_UNIFORMS, &active_uniforms);
    std::cout << active_uniforms<<" active uniforms :" << std::endl;
    for(int i=0;i<active_uniforms;i++)
    {
      GLsizei len = 0;
      GLint data_sz = 0;
      GLenum data_type = GL_NONE;
      char name[64] = {'\0',};
      glGetActiveUniform(prog,i,63,&len,&data_sz,&data_type,name);
      name[len]='\0';
      std::cout<<"  uniform "<<i<<" : name='"<<name<<"', size="<<data_sz<<", type="<<gl_enum_to_string(data_type)<<std::endl;
    }

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

  void GLShaderProgram::use()
  {
    for(auto & u : m_uniforms) u.unmap_buffer();
    m_pipeline_config.use();
    glUseProgram( m_shader_program );
  }

  GLShaderProgram::~GLShaderProgram()
  {
    glUseProgram(0);
    glDeleteProgram(m_shader_program);
    for(auto id : m_shaders) glDeleteShader(id);
  }

  GLUniformBlock& GLShaderProgram::uniform(int i)
  {
    return m_uniforms[i];
  }

  int GLShaderProgram::uniform_id(std::string_view name)
  {
    for(size_t i=0;i<m_uniforms.size();i++) if(name==m_uniforms[i].m_name) return i;
    return -1;
  }

  GLUniformBlock& GLShaderProgram::uniform(std::string_view name)
  {
    return uniform(uniform_id(name));
  }

  const GLUniformVariableAccessor GLUniformBlock::variable(int i)
  {
    map_buffer();
    return { m_variables[i] , m_buffer_mapping };
  }

  int GLUniformBlock::variable_id(std::string_view name)
  {
    for(size_t i=0;i<m_variables.size();i++) if(name==m_variables[i].m_name) return i;
    return -1;
  }

  const GLUniformVariableAccessor GLUniformBlock::variable(std::string_view name)
  {
    return variable( variable_id(name) );
  }


  int GLUniformVariable::size() const
  {
    static const std::map<GLenum,int> gl_size_map =
      { { GL_BOOL , 4 }
      , { GL_INT , 4 }
      , { GL_UNSIGNED_INT , 4 }
      , { GL_FLOAT , 4 }

      , { GL_BOOL_VEC2 , 2*4 }
      , { GL_INT_VEC2 , 2*4 }
      , { GL_UNSIGNED_INT_VEC2 , 2*4 }
      , { GL_FLOAT_VEC2 , 2*4 }

      , { GL_BOOL_VEC3 , 3*4 }
      , { GL_INT_VEC3 , 3*4 }
      , { GL_UNSIGNED_INT_VEC3 , 3*4 }
      , { GL_FLOAT_VEC3 , 3*4 }

      , { GL_BOOL_VEC4 , 4*4 }
      , { GL_INT_VEC4 , 4*4 }
      , { GL_UNSIGNED_INT_VEC4 , 4*4 }
      , { GL_FLOAT_VEC4 , 4*4 }

      , { GL_FLOAT_MAT3 , 9*4 }
      , { GL_FLOAT_MAT4 , 16*4 }
      };
    auto it = gl_size_map.find( m_type );
    if( it == gl_size_map.end() )
    {
      std::cerr<<"Unrecognized variable type "<<gl_enum_to_string(m_type)<<std::endl;
      std::abort();
    }
    return (m_stride>0) ? (m_stride * m_size) : (it->second * m_size);
  }

  void GLUniformBlock::map_buffer()
  {
    if( m_buffer == 0 )
    {
      glGenBuffers(1,&m_buffer);
      glBindBufferBase(GL_UNIFORM_BUFFER,m_binding,m_buffer);
      int uniform_size = m_variables.back().m_offset + m_variables.back().size();
      //std::cout << "bind uniform buffer #"<<m_buffer<<" to block #"<<m_binding <<" and resize to "<<uniform_size<<std::endl;
      glNamedBufferData(m_buffer, uniform_size, nullptr, GL_DYNAMIC_DRAW);
    }
    if( m_buffer_mapping == nullptr )
    {
      //std::cout << "map uniform buffer #"<<m_buffer << std::endl;
      m_buffer_mapping = glMapNamedBuffer(m_buffer, GL_WRITE_ONLY);
      assert( m_buffer_mapping != nullptr );
    }
  }

  void GLUniformBlock::unmap_buffer()
  {
    if( m_buffer_mapping != nullptr )
    {
      glUnmapNamedBuffer(m_buffer);
      m_buffer_mapping = nullptr;
    }
  }

  GLUniformBlock::~GLUniformBlock()
  {
    unmap_buffer();
    if(m_buffer!=0)
    {
      glDeleteBuffers(1,&m_buffer);
      m_buffer = 0;
    }
  }

  template<class T>
  void GLUniformVariableAccessor::set(T value) const requires std::is_arithmetic_v<T>
  {
    assert( m_mapped_ptr != nullptr );
    GLubyte* bptr = ((GLubyte*)m_mapped_ptr) + m_variable.m_offset;
    if( m_variable.m_type==GL_BOOL || m_variable.m_type==GL_INT || m_variable.m_type==GL_UNSIGNED_INT )
    {
      * (GLint*) bptr  = value;
    }
    else if( m_variable.m_type==GL_FLOAT )
    {
      * (GLfloat*) bptr = value;
    }
    else if( m_variable.m_type==GL_DOUBLE )
    {
      * (GLdouble*) bptr = value;
    }
    else
    {
      std::cerr<<"Cannot set a uniform of type "<<gl_enum_to_string(m_variable.m_type)<<std::endl;
      std::abort();
    }
  }

  template void GLUniformVariableAccessor::set<GLint>(GLint value) const;
  template void GLUniformVariableAccessor::set<GLfloat>(GLfloat value) const;

  template<class T>
  void GLUniformVariableAccessor::set(const T* value, GLuint n) const requires std::is_arithmetic_v<T>
  {
    if( n==1 )
    {
      this->set( *value );
    }
    else
    {
      bool is_integer = true;
      // bool is_double = false; // ont supported yet
      switch(m_variable.m_type)
      {
        case GL_FLOAT:
        case GL_FLOAT_VEC2:
        case GL_FLOAT_VEC3:
        case GL_FLOAT_VEC4:
        case GL_FLOAT_MAT2:
        case GL_FLOAT_MAT3:
        case GL_FLOAT_MAT4:
          is_integer=false;
          //is_double=false;
          break;
      }

      int vecsize = 1;
      switch(m_variable.m_type)
      {
        case GL_FLOAT:
        case GL_INT:
        case GL_UNSIGNED_INT:
        case GL_BOOL:
          vecsize = 1;
          break;
        case GL_FLOAT_VEC2:
        case GL_INT_VEC2:
        case GL_UNSIGNED_INT_VEC2:
        case GL_BOOL_VEC2:
          vecsize = 2;
          break;
        case GL_FLOAT_VEC3:
        case GL_INT_VEC3:
        case GL_UNSIGNED_INT_VEC3:
        case GL_BOOL_VEC3:
          vecsize = 3;
          break;
        case GL_FLOAT_VEC4:
        case GL_INT_VEC4:
        case GL_UNSIGNED_INT_VEC4:
        case GL_BOOL_VEC4:
          vecsize = 4;
          break;
        case GL_FLOAT_MAT2:
          vecsize = 4;
          break;
        case GL_FLOAT_MAT3:
          vecsize = 9;
          break;
        case GL_FLOAT_MAT4:
          vecsize = 16;
          break;
      }
      if( n == vecsize * m_variable.m_size )
      {
        for(int ai=0;ai<m_variable.m_size;ai++)
        {
          GLubyte* bptr = ((GLubyte*)m_mapped_ptr) + m_variable.m_offset + m_variable.m_stride * ai;
          GLfloat * fptr = (GLfloat*) bptr;
          GLint * iptr = (GLint*) bptr;
          for(int vi=0;vi<vecsize;vi++)
          {
            if(is_integer) iptr[vi] = value[ai*vecsize+vi];
            else fptr[vi] = value[ai*vecsize+vi];
          }
        }
      }
      else
      {
        std::cerr<<"Cannot set variable "<<gl_enum_to_string(m_variable.m_type)<<" "<<m_variable.m_name<<"["<<m_variable.m_size<<"] with "<<n<<" values"<<std::endl;
        std::abort();
      }
    }
  }

  template void GLUniformVariableAccessor::set<GLint>(const GLint* value, GLuint n) const;
  template void GLUniformVariableAccessor::set<GLfloat>(const GLfloat* value, GLuint n) const;

}
