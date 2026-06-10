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
  struct UniformCameraBinding
  {
    std::shared_ptr<GLShaderProgram> m_shader = nullptr; // shader to write matrices to
    GLint m_block_id = -1;
    GLint m_modelview_id = -1;
    GLint m_projection_id = -1;
    GLint m_aspect_ratio_id = -1;
    GLint m_viewport_id = -1;
  };
  
  struct UniformCameraMatrix
  {    
    vec3 m_eye = { 1.0f , 1.0f , 1.0f  };
    vec3 m_center = { 0.0f , 0.0f , 0.0f  };

    vec3 m_side = { 1.0f , 0.0f , 0.0f };
    vec3 m_up = { 0.0f , 1.0f , 0.0f };
    vec3 m_forward = { 0.0f , 0.0f , 1.0f };

    GLfloat m_fov = 60.0f; // fov angle in degrees
    GLfloat m_aspect_ratio = 800.0f / 600.0f; // = w/h
    GLfloat m_near = 0.1f;
    GLfloat m_far = 100.0f;
    GLint m_viewport[2] = { 800 , 600 };

    GLfloat m_modelview_matrix[16] = { std::numeric_limits<GLfloat>::quiet_NaN(), };
    GLfloat m_projection_matrix[16] = { std::numeric_limits<GLfloat>::quiet_NaN(), };

    std::map< std::shared_ptr<GLShaderProgram> , UniformCameraBinding > m_shader_binding;

    void viewport(int width, int height);
    void perspective(float fov, float ratio, float near, float far);
    void look_at( const vec3& eye, const vec3& center, const vec3& up = {0.0f,1.0f,0.0f} );
    void tilt( GLfloat horiz_angle, GLfloat vert_angle );
    void move( GLfloat side, GLfloat up, GLfloat forward );

    // default names are those defined in include header "uniform/camera"
    void attach_shader( std::shared_ptr<GLShaderProgram> prog
                         , std::string_view block = "camera"
                         , std::string_view mvmat = "modelview"
                         , std::string_view projmat = "projection"
                         , std::string_view aspectname = "aspect_ratio"
                         , std::string_view viewportname = "viewport" );

    void detach_shader( std::shared_ptr<GLShaderProgram> prog );
                         
    void update_uniform();

    void update_modelview();
    void update_projection();
    void transform(const GLfloat v[4], GLfloat out[4]);
  };

}
