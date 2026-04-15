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

#include <EGLRender/egl_platform.h>

#include <map>
#include <string>
#include <string_view>
#include <cassert>
#include <iostream>

//#define EGL_VERBOSE_ENUM_CONFILCTS 1

namespace EGLRender
{

  static GLenum g_gl_enum_db_values [] =
  {
#define _SEP ,
#define _ENUM(N,E) static_cast<GLenum>(N)
#include "gl_enums.hxx"
  };

  static const char* g_gl_enum_db_strings [] =
  {
#define _SEP ,
#define _ENUM(N,E) #E
#include "gl_enums.hxx"
  };

  static inline std::map<GLenum,std::string> mk_gl_enum_string_map()
  {
    const size_t n_values = sizeof(g_gl_enum_db_values) / sizeof(GLenum);
    const size_t n_strings = sizeof(g_gl_enum_db_strings) / sizeof(const char*);
    if( n_values != n_strings )
    {
      std::cerr << "ERROR: GL enum database corruputed" << std::endl;
      std::abort();
    }
    std::map<GLenum,std::string> m;
    for(size_t i=0;i<n_values;i++)
    {
      auto it = m.find(g_gl_enum_db_values[i]);
      if( it != m.end() )
      {
        if( std::string(it->second).starts_with(g_gl_enum_db_strings[i]) )
        {
#         ifdef EGL_VERBOSE_ENUM_CONFILCTS
          std::cout << "GLenum -> string conflict : "<<g_gl_enum_db_strings[i]<<" now replaces "<<it->second << std::endl;
#         endif
          it->second = g_gl_enum_db_strings[i];
        }
#       ifdef EGL_VERBOSE_ENUM_CONFILCTS
        else
        {
          std::cout << "GLenum -> string conflict : "<<g_gl_enum_db_strings[i]<<" discarded by previous entry "<<it->second << std::endl;
        }
#       endif
      }
      else
      {
        m.insert( { g_gl_enum_db_values[i] , g_gl_enum_db_strings[i] } );
      }
    }
    return m;
  }

  static inline std::map<std::string,GLenum> mk_gl_string_enum_map()
  {
    const size_t n_values = sizeof(g_gl_enum_db_values) / sizeof(GLenum);
    const size_t n_strings = sizeof(g_gl_enum_db_strings) / sizeof(const char*);
    if( n_values != n_strings )
    {
      std::cerr << "ERROR: GL enum database corruputed" << std::endl;
      std::abort();
    }
    std::map<std::string,GLenum> m;
    for(size_t i=0;i<n_values;i++) m.insert( { g_gl_enum_db_strings[i] , g_gl_enum_db_values[i] } );
    return m;
  }

  static std::map<GLenum,std::string> g_gl_enum_string_map = mk_gl_enum_string_map();
  static std::map<std::string,GLenum> g_gl_string_enum_map = mk_gl_string_enum_map();

  const char * gl_string_non_null(const GLubyte* s)
  {
    return (s==nullptr) ? "<null>" : (const char*)s;
  }

  GLuint gl_type_bytes(GLenum t)
  {
    switch(t)
    {
      case GL_FLOAT: return sizeof(GLfloat);
      case GL_DOUBLE: return sizeof(GLdouble);
      case GL_INT: return sizeof(GLint);
      case GL_UNSIGNED_INT: return sizeof(GLuint);
      case GL_SHORT: return sizeof(GLshort);
      case GL_UNSIGNED_SHORT: return sizeof(GLushort);
      case GL_BYTE: return sizeof(GLbyte);
      case GL_UNSIGNED_BYTE: return sizeof(GLubyte);
    }
    return 0;
  }

  const std::string& gl_enum_to_string(GLenum t)
  {
    static const std::string not_found = "<unknown>";
    auto it = g_gl_enum_string_map.find(t);
    if( it != g_gl_enum_string_map.end() ) return it->second;
    else return not_found;
  }

  bool string_is_gl_enum(std::string_view t)
  {
    return g_gl_string_enum_map.find(t.data()) != g_gl_string_enum_map.end();
  }

  GLenum gl_enum_from_string(std::string_view t)
  {
    static const std::string not_found = "<unknown>";
    auto it = g_gl_string_enum_map.find(t.data());
    if( it != g_gl_string_enum_map.end() ) return it->second;
    else return GL_NONE;
  }

  GLenum gl_enum_from_string(const std::string& name)
  {
    return gl_enum_from_string( std::string_view(name) );
  }

}

