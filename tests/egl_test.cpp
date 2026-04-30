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

  EGLRenderSurfaceClass surf_type = EGLRenderSurfaceClass::PBUFFER;
  if( argc>1 )
  {
    if( std::string(argv[1]) == "pb" ) surf_type = EGLRenderSurfaceClass::PBUFFER;
    else if( std::string(argv[1]) == "fs" ) surf_type = EGLRenderSurfaceClass::FULLSCREEN;
    else if( std::string(argv[1]) == "wn" ) surf_type = EGLRenderSurfaceClass::WINDOW;
  }
  std::cout << "Surface = "<<render_surface_type_as_string(surf_type)<<std::endl;

  EGLRenderManager eglm;
  eglm.init_platform( surf_type != EGLRenderSurfaceClass::PBUFFER );

  eglm.create_surface( "main_window", surf_type, 800, 600 );
  eglm.surface("main_window").make_current();

  std::cout<<"OpenGL "<<gl_string_non_null(glGetString(GL_VERSION))<<std::endl;

  const auto shader_prog_id = eglm.create_shader_program(
    "rotating_triangle" ,
    /* vertex shader */ R"EOF(
    #version 430 core
    #extension GL_ARB_shading_language_include : require
    #include <uniform/camera>
    layout (location = 0) in vec4 aPos;
    layout (location = 1) in float aAngle;
    out float geomAngle;
    void main()
    {
      mat4 mvp = projection * modelview;
      gl_Position = mvp * aPos;
      geomAngle = aAngle;
    }
    )EOF" ,
    /* geometry shader */ R"EOF(
    #version 430 core
    layout (points) in;
    layout (triangle_strip, max_vertices=3) out;
    in float geomAngle[];
    out vec4 aColor;
    void main()
    {
      vec4 aPos = gl_in[0].gl_Position;
      aColor = vec4( clamp(aPos.x,0.1f,1.0f), clamp(aPos.y,0.1f,1.0f), clamp(aPos.x+aPos.y,0.1f,1.0f), 1.0f );
      for(int i=0;i<3;i++)
      {
        gl_Position = gl_in[0].gl_Position + vec4( cos(geomAngle[0]+i*2*3.14159/3)*0.02 , sin(geomAngle[0]+i*2*3.14159/3)*0.02, 0.0 , 0.0 );
        EmitVertex();
      }
      EndPrimitive();
    }
    )EOF" ,
    /* fragment shader */ R"EOF(
    #version 430 core
    in vec4 aColor;
    out vec4 FragColor;
    void main()
    {
      FragColor = aColor;
    }
    )EOF" ,
    { .m_enable_flags = { GL_PROGRAM_POINT_SIZE } }
  );

  auto & shader = eglm.shader_program(shader_prog_id);

  std::cout << "Pipeline config :" << std::boolalpha << std::endl;
  shader.m_pipeline_config.to_stream( std::cout );    

  auto camera_id = eglm.create_camera("pov");
  auto & camera = eglm.camera(camera_id);
  camera.look_at( {0,5,10} , {0,0,0} );
  camera.perspective(60,800.0f/600.0f,0.1f,100.0f); // disable projection
  camera.attach_to_shader( eglm.shader_program_ptr(shader_prog_id), "camera", "modelview", "projection" );
  camera.update_uniform();

  shader.use();

  const int n_points = 64;
  const auto buf_id = eglm.create_vertex_buffers("vertex_attribs",n_points , { GL_FLOAT,4, GL_FLOAT,1 } );

  glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDepthMask (GL_TRUE);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  if( surf_type == EGLRenderSurfaceClass::PBUFFER )
  {
    auto & ren_surf = eglm.surface("main_window");
    const int width = ren_surf.width();
    const int height = ren_surf.height();

    glClear(GL_COLOR_BUFFER_BIT);

    GLfloat phi_base = 0.0f;
    auto & glvbos = eglm.vertex_buffers(buf_id);
    // equivalent to
    // auto & glvbos = eglm.vertex_buffer("vertex_attribs");
    GLfloat* v = (GLfloat*) glvbos.host_map_write_only(0);
    GLfloat* a = (GLfloat*) glvbos.host_map_write_only(1);
    for(int j=0;j<n_points;j++)
    {
      GLfloat phi = phi_base + (2*M_PI*j/n_points);
      GLfloat in[4] = { std::cos(phi)*0.5f
                      , std::sin(phi)*0.5f
                      , std::cos(phi*1.5f)*std::sin(phi*2.5f)*0.5f
                      , 1.0f };
      GLfloat out[4] = {0,0,0,0};
      camera.transform(in,out);
      for(int k=0;k<4;k++) v[j*4+k]=out[k];
      a[j] = -phi_base*10.0f;
    }
    glvbos.host_unmap(0); v=nullptr;
    glvbos.host_unmap(1); a=nullptr;

    eglm.vertex_buffers(buf_id).use();
    // equivalent to
    // eglm.vertex_buffer("vertex_attribs").use();

    glDrawArrays(GL_POINTS, 0, n_points);

    size_t pixel_data_sz = width * height * 4;
    auto pixel_data = std::make_unique_for_overwrite<char[]>(pixel_data_sz);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data.get() );
    std::string filename = std::format("eglscreen{}x{}", width,height);
    {
      std::ofstream fout(filename+".raw");
      fout.write( pixel_data.get() , pixel_data_sz );
      fout.close();
    }
    {
      std::ofstream fout(filename+".to-png");
      fout << std::format("convert -depth 8 -size {}x{}+0 rgba:{}.raw {}.png",width,height,filename,filename) << std::endl;
      fout.close();
    }
  }
  else
  {
    struct
    {
      GLfloat move[3] = { 0.0f , 0.0f , 0.0f };
      GLfloat tilt[2] = { 0.0f , 0.0f };
      int mouse_last_x = -1;
      int mouse_last_y = -1;
      int should_exit = false;
      int left_drag = false;
      int mid_drag = false;
      int right_drag = false;
    } uistate;

    int update_cam = 0;
    auto & ren_surf = eglm.surface("main_window");
    ren_surf.m_event_handler.on_key_release = [&uistate,f=ren_surf.m_event_handler.on_key_release](int key)
    {
       if( f ) f(key);
       if( key == 65307 ) uistate.should_exit = true;
    };
    ren_surf.m_event_handler.on_button_press = [&uistate,f=ren_surf.m_event_handler.on_button_press](int state, int b, int x,int y)
    {
      if( f ) f(state,b,x,y);
      switch( b )
      {
        case 1 : uistate.left_drag=true; break;
        case 2 : uistate.mid_drag=true; break;
        case 3 : uistate.right_drag=true; break;
      }
    };
    ren_surf.m_event_handler.on_button_release = [&uistate,f=ren_surf.m_event_handler.on_button_release](int state, int b, int x,int y)
    {
      if( f ) f(state,b,x,y);
      switch( b )
      {
        case 1 : uistate.left_drag=false; break;
        case 2 : uistate.mid_drag=false; break;
        case 3 : uistate.right_drag=false; break;
      }
    };
    ren_surf.m_event_handler.on_mouse_move = [&uistate,&camera,&update_cam,f=ren_surf.m_event_handler.on_mouse_move](int x,int y)
    {
      if( f ) f(x,y);
      int dx = x - uistate.mouse_last_x;
      int dy = y - uistate.mouse_last_y;
      if( uistate.left_drag )
      {
        uistate.tilt[0] += dx * 0.1f;
        uistate.tilt[1] += dy * 0.1f;
        update_cam = 1;        
      }
      if( uistate.mid_drag )
      {
        uistate.move[0] += dx * 0.02f;
        uistate.move[1] += dy * 0.02f;
        update_cam = 1;
      }
      if( uistate.right_drag )
      {
        uistate.move[2] += dy * 0.02f;
        update_cam = 1;
      }
      uistate.mouse_last_x = x;
      uistate.mouse_last_y = y;
    };

    int i=0;
    bool first = true;
    while( ! uistate.should_exit )
    {
      ren_surf.process_events();

      if(update_cam)
      {
        camera.tilt( uistate.tilt[0] , uistate.tilt[1] );
        camera.move( uistate.move[0] , uistate.move[1] , uistate.move[2] );
        uistate.tilt[0] = 0.0f;
        uistate.tilt[1] = 0.0f;
        uistate.move[0] = 0.0f;
        uistate.move[1] = 0.0f;
        uistate.move[2] = 0.0f;
        update_cam = 0;
      }

      ren_surf.make_current();

      camera.update_uniform();

      GLfloat phi_base = i*0.003f;
      auto & glvbos = eglm.vertex_buffers(buf_id);
      // equivalent to
      // auto & glvbos = eglm.vertex_buffer("vertex_attribs");
      GLfloat* v = (GLfloat*) glvbos.host_map_write_only(0);
      GLfloat* a = (GLfloat*) glvbos.host_map_write_only(1);
      for(int j=0;j<n_points;j++)
      {
        GLfloat phi = phi_base + (2*M_PI*j/n_points);
        GLfloat in[4] = { std::cos(phi)*0.5f
                        , std::sin(phi)*0.5f
                        , std::cos(phi*1.5f)*std::sin(phi*2.5f)*0.5f
                        , 1.0f };
        GLfloat out[4] = {0,0,0,0};
        //camera.transform(in,out);
        for(int k=0;k<4;k++) v[j*4+k]=in[k];
        a[j] = -phi_base*10.0f;
      }
      first=false;
      glvbos.host_unmap(0); v=nullptr;
      glvbos.host_unmap(1); a=nullptr;

      glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

      glvbos.use();
      glDrawArrays(GL_POINTS, 0, n_points);

      ren_surf.swap_buffers();
      ++i;
    }
  }

  return 0;
}
