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
#include <format>

namespace EGLRender
{

  static inline std::array<GLfloat,3> cross(const std::array<GLfloat,3>& u, const std::array<GLfloat,3>& v)
  {
    return { u[1] * v[2] - u[2] * v[1]
           , u[2] * v[0] - u[0] * v[2]
           , u[0] * v[1] - u[1] * v[0] };
  }

  static inline auto dot(const std::array<GLfloat,3> &u, const std::array<GLfloat,3> &v)
  {
    return u[0]*v[0]+u[1]*v[1]+u[2]*v[2];
  }

  static inline std::array<GLfloat,3> add(const std::array<GLfloat,3>& u, const std::array<GLfloat,3>& v)
  {
    return { u[0]+v[0] , u[1]+v[1] , u[2]+v[2] };
  }

  static inline std::array<GLfloat,3> sub(const std::array<GLfloat,3>& u, const std::array<GLfloat,3>& v)
  {
    return { u[0]-v[0] , u[1]-v[1] , u[2]-v[2] };
  }

  static inline std::array<GLfloat,3> neg(const std::array<GLfloat,3>& u)
  {
    return { -u[0] , -u[1] , -u[2] };
  }
  
  static inline auto norm2( const std::array<GLfloat,3> &u ) { return dot(u,u); }
  static inline auto norm( const std::array<GLfloat,3> &u ) { return sqrt(norm2(u)); }
  static inline std::array<GLfloat,3> mul( const std::array<GLfloat,3> &u , const GLfloat s ) { return {u[0]*s,u[1]*s,u[2]*s}; }
  static inline auto normalize( const std::array<GLfloat,3> &u ) { return mul( u , 1.0 / norm(u) ); }

  void UniformCameraMatrix::lookAt( GLfloat eyeX, GLfloat eyeY, GLfloat eyeZ, GLfloat toX, GLfloat toY, GLfloat toZ)
  {
    m_location = { eyeX, eyeY, eyeZ };
    std::array<GLfloat,3> to = { toX, toY, toZ };
    m_front = normalize( sub(to,m_location) );
    m_up = { 0.0f , 1.0f , 0.0f  };
    m_left = normalize( cross(m_up,m_front) );
    m_up = normalize( cross(m_front,m_left) );
  }

  void UniformCameraMatrix::tilt( GLfloat h, GLfloat v )
  {
    const GLfloat DEG2RAD = acos(-1.0f) / 180;
    const GLfloat hcos = cos(h*DEG2RAD);
    const GLfloat hsin = sin(h*DEG2RAD);
    const GLfloat vcos = cos(v*DEG2RAD);
    const GLfloat vsin = sin(v*DEG2RAD);

    auto nfront = add( mul(m_front,vcos) , mul(m_up,vsin) );
    m_up = add( mul(m_up,vcos) , mul(m_front,-vsin) );
    m_front = nfront;
    
    nfront = add( mul(m_front,hcos) , mul(m_left,hsin) );
    m_left = add( mul(m_left,hcos) , mul(m_front,-hsin) );
    m_front = nfront;
    
    m_left = normalize(m_left);
    m_up = normalize(m_up);
    m_front = normalize(m_front);
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

  static inline void transpose(GLfloat mat[16])
  {
    for(int j=0;j<4;j++) for(int i=0;i<4;i++) if(i!=j)
    {
      std::swap( mat[j*4+i] , mat[i*4+j] );
    }
  }

  void UniformCameraMatrix::update_uniform()
  {
    constexpr double DEG2RAD = acos(-1.0f) / 180;

    // no, don't use glMatrix* functions, use unform binding, see reference below
    // https://perso.univ-lyon1.fr/jean-claude.iehl/Public/educ/M1IMAGE/html/group__uniform__buffers.html

    double tangent = tan(m_fov_x/2 * DEG2RAD);   // tangent of half fovY
    double width = m_near * tangent;          // half height of near plane
    double height = width / m_aspect_ratio;      // half width of near plane
    GLfloat W = (2*m_near) / (2*width);
    GLfloat H = (2*m_near) / (2*height);
    GLfloat A = 0.0;
    GLfloat B = 0.0;
    GLfloat C = - (m_far+m_near) / (m_far-m_near);
    GLfloat D = - (2*m_far*m_near) / (m_far-m_near);

    GLfloat projection[16] = { 1, 0, 0, 0
                             , 0, 1, 0, 0
                             , 0, 0, 1, 0
                             , 0, 0, 0, 1 };
    transpose( projection );
    for(int i=0;i<16;i++) std::cout << std::format("{}{:.3e}",(i%4==0)?'\n':' ',projection[i]);
    std::cout<<std::endl;
    m_shader->uniform(m_block_id).variable(m_projection_variable_id).set( projection, 16 );

    GLfloat modelview[16] = { m_left[0], m_up[0], m_front[0], -m_location[0]
                            , m_left[1], m_up[1], m_front[1], -m_location[1]
                            , m_left[2], m_up[2], m_front[2], -m_location[2]
                            , 0.0f     , 0.0f   , 0.0f      , 1.0f };
    transpose( modelview );
    for(int i=0;i<16;i++) std::cout << std::format("{}{:.3e}",(i%4==0)?'\n':' ',modelview[i]);
    std::cout<<std::endl;

    m_shader->uniform(m_block_id).variable(m_modelview_variable_id).set( modelview, 16 );
  }

}
