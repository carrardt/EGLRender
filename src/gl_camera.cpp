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

#include <EGLRender/gl_camera.h>
#include <cmath>

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

  void GLCamera::lookAt( GLfloat eyeX, GLfloat eyeY, GLfloat eyeZ, GLfloat toX, GLfloat toY, GLfloat toZ)
  {
    m_location = { eyeX, eyeY, eyeZ };
    std::array<GLfloat,3> to = { toX, toY, toZ };
    m_front = normalize( sub(to,m_location) );
    m_up = { 0.0f , 1.0f , 0.0f  };
    m_left = normalize( cross(m_up,m_front) );
    m_up = normalize( cross(m_front,m_left) );
  }

  void GLCamera::tilt( GLfloat h, GLfloat v )
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
  
  void GLCamera::use()
  {
    constexpr double DEG2RAD = acos(-1.0f) / 180;

    // no, don't use glMatrix* functions, use unform binding, see reference below
    // https://perso.univ-lyon1.fr/jean-claude.iehl/Public/educ/M1IMAGE/html/group__uniform__buffers.html

    glMatrixMode( GL_PROJECTION_MATRIX );
    glLoadIdentity();
    double tangent = tan(m_fov_x/2 * DEG2RAD);   // tangent of half fovY
    double width = m_near * tangent;          // half height of near plane
    double  height = width / m_aspect_ratio;      // half width of near plane
    glFrustum(-width, width, -height, height, m_near, m_far);

    glMatrixMode( GL_MODELVIEW_MATRIX );
    glLoadIdentity();
    GLfloat modelview[16] = { m_left[0], m_up[0], m_front[0], 0.0f
                            , m_left[1], m_up[1], m_front[1], 0.0f
                            , m_left[2], m_up[2], m_front[2], 0.0f
                            , 0.0f     , 0.0f   , 0.0f      , 1.0f };
    glMultMatrixf( modelview );
    glTranslatef( -m_location[0], -m_location[1], -m_location[2] );
  }

}
