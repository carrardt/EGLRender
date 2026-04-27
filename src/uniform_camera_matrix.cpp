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
#include <EGLRender/uniform_camera_matrix.h>
#include <cmath>
#include <cstring>

namespace EGLRender
{
  
  static inline void matrix_identity(GLfloat m[16])
  {
      m[0+4*0] = 1; m[0+4*1] = 0; m[0+4*2] = 0; m[0+4*3] = 0;
      m[1+4*0] = 0; m[1+4*1] = 1; m[1+4*2] = 0; m[1+4*3] = 0;
      m[2+4*0] = 0; m[2+4*1] = 0; m[2+4*2] = 1; m[2+4*3] = 0;
      m[3+4*0] = 0; m[3+4*1] = 0; m[3+4*2] = 0; m[3+4*3] = 1;
  }

  static inline void matrix_translate(GLfloat m[16], GLfloat x, GLfloat y, GLfloat z)
  {
      m[0+4*0] = 1; m[0+4*1] = 0; m[0+4*2] = 0; m[0+4*3] = x;
      m[1+4*0] = 0; m[1+4*1] = 1; m[1+4*2] = 0; m[1+4*3] = y;
      m[2+4*0] = 0; m[2+4*1] = 0; m[2+4*2] = 1; m[2+4*3] = z;
      m[3+4*0] = 0; m[3+4*1] = 0; m[3+4*2] = 0; m[3+4*3] = 1;
  }

  static inline void matrix_perspective(GLfloat out[16], GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar)
  {
      GLfloat m[4][4];
      double sine, cotangent, deltaZ;
      double radians = fovy / 2 * M_PI / 180;

      deltaZ = zFar - zNear;
      sine = sin(radians);
      if ((deltaZ == 0) || (sine == 0) || (aspect == 0)) {
        matrix_identity(&m[0][0]);
        return;
      }
      cotangent = cos(radians) / sine;

      matrix_identity(&m[0][0]);
      m[0][0] = cotangent / aspect;
      m[1][1] = cotangent;
      m[2][2] = -(zFar + zNear) / deltaZ;
      m[2][3] = -1;
      m[3][2] = -2 * zNear * zFar / deltaZ;
      m[3][3] = 0;
      std::memcpy( out, &m[0][0], 16*sizeof(GLfloat) );
  }

  static inline void mult_matrix(const GLfloat a[16], const GLfloat b[16], GLfloat r[16])
  {
    int i, j;
    for (i = 0; i < 4; i++) {
      for (j = 0; j < 4; j++) {
        r[i*4+j] = 
        a[i*4+0]*b[0*4+j] +
        a[i*4+1]*b[1*4+j] +
        a[i*4+2]*b[2*4+j] +
        a[i*4+3]*b[3*4+j];
      }
    }
  }

  static inline void normalize(GLfloat v[3])
  {
      float r;

      r = sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
      if (r == 0.0) return;

      v[0] /= r;
      v[1] /= r;
      v[2] /= r;
  }

  static inline void cross(const GLfloat v1[3], const GLfloat v2[3], GLfloat result[3])
  {
      result[0] = v1[1]*v2[2] - v1[2]*v2[1];
      result[1] = v1[2]*v2[0] - v1[0]*v2[2];
      result[2] = v1[0]*v2[1] - v1[1]*v2[0];
  }

  static inline void 
  matrix_look_at( GLfloat out[16]
                , GLfloat eyex, GLfloat eyey, GLfloat eyez
                , GLfloat centerx, GLfloat centery, GLfloat centerz
                , GLfloat upx, GLfloat upy, GLfloat upz)
  {
      float forward[3], side[3], up[3];
      GLfloat m[4][4];

      forward[0] = centerx - eyex;
      forward[1] = centery - eyey;
      forward[2] = centerz - eyez;

      up[0] = upx;
      up[1] = upy;
      up[2] = upz;

      normalize(forward);

      /* Side = forward x up */
      cross(forward, up, side);
      normalize(side);

      /* Recompute up as: up = side x forward */
      cross(side, forward, up);

      matrix_identity(&m[0][0]);
      m[0][0] = side[0];
      m[1][0] = side[1];
      m[2][0] = side[2];

      m[0][1] = up[0];
      m[1][1] = up[1];
      m[2][1] = up[2];

      m[0][2] = -forward[0];
      m[1][2] = -forward[1];
      m[2][2] = -forward[2];
      
      GLfloat tmat[4][4];
      matrix_translate(&tmat[0][0] , -eyex, -eyey, -eyez );
      mult_matrix( &m[0][0] , &tmat[0][0] , out );
  }

  void UniformCameraMatrix::perspective(float fov, float ratio, float near, float far)
  {
    m_fov = fov;
    m_aspect_ratio = ratio;
    m_near = near;
    m_far = far;
  }

  void UniformCameraMatrix::look_at(const vec3& eye, const vec3& center)
  {
    m_eye = eye;
    m_center = center;
  }
  
  void UniformCameraMatrix::attach_to_shader(std::shared_ptr<GLShaderProgram> prog, std::string_view uniform_name, std::string_view mvmat_name, std::string_view projmat_name)
  {
    m_shader = prog;
    if( m_shader == nullptr ) return;
    m_block_id = prog->uniform_id(uniform_name);
    if(m_block_id==-1)
    {
      std::cerr << "EGL Error: uniform block '"<<uniform_name<<"' not found in shader #"<<prog->m_shader_program <<std::endl;
      std::abort();
    }
    m_modelview_variable_id = prog->uniform(m_block_id).variable_id(mvmat_name);
    if(m_modelview_variable_id==-1)
    {
      std::cerr << "EGL Error: variable '"<<mvmat_name<<"' not found in uniform block '"<<uniform_name<<"'" <<std::endl;
      std::abort();
    }
    m_projection_variable_id = prog->uniform(m_block_id).variable_id(projmat_name);
    if(m_projection_variable_id==-1)
    {
      std::cerr << "EGL Error: variable '"<<projmat_name<<"' not found in uniform block '"<<uniform_name<<"'" <<std::endl;
      std::abort();
    }
    std::cout << "atached to shader's block #"<<m_block_id<<", modelview is variable #"<<m_modelview_variable_id<<", projection variable is #"<<m_projection_variable_id<<std::endl;
  }

  void UniformCameraMatrix::update_uniform()
  {
    constexpr double DEG2RAD = acos(-1.0f) / 180;
    GLfloat mat[16];

    if( m_fov > 0.0 ) matrix_perspective( mat, m_fov, m_aspect_ratio, m_near, m_far );
    else matrix_identity( mat );
    m_shader->uniform(m_block_id).variable(m_projection_variable_id).set( mat, 16 );

    matrix_look_at(mat, m_eye[0],m_eye[1],m_eye[2], m_center[0],m_center[1],m_center[2], m_up[0],m_up[1],m_up[2]);
    m_shader->uniform(m_block_id).variable(m_modelview_variable_id).set( mat, 16 );
  }

}
