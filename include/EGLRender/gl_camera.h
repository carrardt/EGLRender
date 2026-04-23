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
#include <array>

namespace EGLRender
{
  
  struct GLCamera
  {
    std::array<GLfloat,3> m_left  = { 1.0f , 0.0f , 0.0f  };
    std::array<GLfloat,3> m_up    = { 0.0f , 1.0f , 0.0f  };
    std::array<GLfloat,3> m_front = { 0.0f , 0.0f , 1.0f };
    std::array<GLfloat,3> m_location = { 0.0f , 0.0f , -5.0f };

    GLfloat m_fov_x = 60.0f; // fov angle in degrees
    GLfloat m_aspect_ratio = 16.0f/9.0f; // = w/h
    GLfloat m_near = 1.0f;
    GLfloat m_far = 10.0f;
    
    void lookAt( GLfloat eyeX, GLfloat eyeY, GLfloat eyeZ, GLfloat toX, GLfloat toY, GLfloat toZ);
    void tilt( GLfloat hAngle, GLfloat vAngle );
    void use();
  };

}
