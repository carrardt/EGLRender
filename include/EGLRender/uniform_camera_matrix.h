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
#include <EGLRender/gl_shader.h>
#include <memory>
#include <span>
#include <limits>
#include <cmath>

namespace EGLRender
{
  
  struct UniformCameraMatrix
  {    
    vec3 m_eye = { 0.0f , 0.0f , -15.0f  };
    vec3 m_center = { 0.0f , 0.0f , 0.0f  };
    vec3 m_up = { 0.0f , 1.0f , 0.0f };

    GLfloat m_fov = 60.0f; // fov angle in degrees
    GLfloat m_aspect_ratio = 16.0f/9.0f; // = w/h
    GLfloat m_near = 0.1f;
    GLfloat m_far = 100.0f;

    GLfloat m_modelview_matrix[16] = { std::numeric_limits<GLfloat>::quiet_NaN(), };
    GLfloat m_projection_matrix[16] = { std::numeric_limits<GLfloat>::quiet_NaN(), };

    std::shared_ptr<GLShaderProgram> m_shader = nullptr; // shader to write matrices to
    GLint m_block_id = -1;
    GLint m_modelview_variable_id = -1;
    GLint m_projection_variable_id = -1;

    void perspective(float fov, float ratio, float near, float far);
    void look_at( const vec3& eye, const vec3& center );    
    
    void attach_to_shader( std::shared_ptr<GLShaderProgram> prog, std::string_view uniform_name, std::string_view mvmat_name, std::string_view projmat_name );
    void update_uniform();

    void update_modelview();
    void update_projection();
    void transform(const GLfloat v[4], GLfloat out[4]);
  };

}
