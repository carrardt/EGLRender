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
#include <EGLRender/egl_error.h>
#include <EGLRender/egl_renderer.h>
#include <EGLRender/egl_surface.h>
#include <EGLRender/gl_shader.h>
#include <EGLRender/gl_vbo.h>
#include <EGLRender/egl_render_manager.h>

#include <iostream>
#include <fstream>
#include <memory>
#include <cmath>
#include <format>

int main(int argc, char *argv[])
{
  using namespace EGLRender;

  EGLRenderManager eglm;
  eglm.init_platform();

  eglm.create_surface( "main_window", EGLRenderSurfaceClass::WINDOW, 800, 600 );
  eglm.surface("main_window").make_current();

  std::cout<<"OpenGL "<<gl_string_non_null(glGetString(GL_VERSION))<<std::endl;

  const auto shader_prog_id = eglm.create_shader_program(
    "rotating_triangle" ,
    /* vertex_shader */ R"EOF(
    #version 330 core
    layout (location = 0) in vec4 aPos;
    layout (location = 1) in float aAngle;
    out vec4 aColor;
    void main()
    {
        gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
        gl_PointSize = 8;
        aColor = vec4( sin(aAngle)*0.5f+0.5f , cos(aAngle)*0.5f+0.5f , cos(aAngle)*sin(aAngle)*0.5f+0.5f , 1.0f );
    }
    )EOF" ,
    /* geometry_shader */ "" ,
    /* fragment_shader */ R"EOF(
    #version 330 core
    in vec4 aColor;
    out vec4 FragColor;
    void main()
    {
        FragColor = aColor;
    }
    )EOF" ,
    { .m_enable_flags = { GL_PROGRAM_POINT_SIZE , GL_DEPTH_TEST } }
  );
  std::cout << "Pipeline config :" << std::boolalpha << std::endl;
  eglm.shader_program(shader_prog_id).m_pipeline_config.to_stream( std::cout );
  eglm.shader_program(shader_prog_id).use();
  // equivalent to
  // eglm.shader_program("rotating_triangle").use();

  const int n_points = 32;
  const auto buf_id = eglm.create_vertex_buffers("vertex_attribs",n_points , { GL_FLOAT,3, GL_FLOAT,1 } );

  glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDepthMask (GL_TRUE);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  bool should_exit = false;
  auto & ren_surf = eglm.surface("main_window");
  ren_surf.m_event_handler.on_button_press = [&should_exit,f=ren_surf.m_event_handler.on_button_press](int state, int b, int x,int y) { f(state,b,x,y); if(b==3) should_exit=true; };

  auto & glvbos = eglm.vertex_buffers(buf_id);
  // auto & glvbos = eglm.vertex_buffer("vertex_attribs");

  int i=0;
  while( ! should_exit )
  {
    ren_surf.process_events();
    ren_surf.make_current();

    GLfloat phi_base = i*0.003f;

    GLfloat* v = (GLfloat*) glvbos.host_map_write_only(0);
    GLfloat* a = (GLfloat*) glvbos.host_map_write_only(1);
    for(int j=0;j<n_points;j++)
    {
      GLfloat phi = phi_base + (2*M_PI*j/n_points);
      v[j*3+0]=std::cos(phi)*0.5f;
      v[j*3+1]=std::sin(phi)*0.5f;
      v[j*3+2]=(std::cos(phi)+std::cos(phi))*0.5f;
      a[j] = -phi_base*10.0f;
    }
    glvbos.host_unmap(0); v=nullptr;
    glvbos.host_unmap(1); a=nullptr;

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

    glvbos.use();
    glDrawArrays(GL_POINTS, 0, n_points);

    ren_surf.swap_buffers();
    ++i;
  }


  return 0;
}
