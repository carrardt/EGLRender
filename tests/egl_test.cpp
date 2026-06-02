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

  const int width = 800;
  const int height = 600;

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

  eglm.create_surface( "main_window", surf_type, width, height );
  eglm.surface("main_window").make_current();

  std::cout<<"OpenGL "<<gl_string_non_null(glGetString(GL_VERSION))<<std::endl;

  std::vector<GLShaderTypeSource> pass1_shader_sources = {
    { GL_VERTEX_SHADER , R"EOF(
    #version 460 core
    layout (location = 0) in vec4 aPos;
    layout (location = 1) in float aAngle;
    out float geomAngle;
    void main()
    {
      gl_Position = aPos;
      geomAngle = aAngle;
    }
    )EOF" } ,
    { GL_GEOMETRY_SHADER , R"EOF(
    #version 460 core
    #extension GL_ARB_shading_language_include : require
    #include <uniform/camera>
    #include <quadrics/convex_hull>
    #include <quadrics/shape>
    //#define CVH_DRAW_OUTLINE_ONLY 1
    layout (points) in;
    #ifdef CVH_DRAW_OUTLINE_ONLY
    layout (line_strip, max_vertices=CVH_LINE_STRIP_MAX_VERTICES) out;
    #else
    layout (line_strip, max_vertices=BOX_LINE_STRIP_MAX_VERTICES) out;
    #endif
    in float geomAngle[];
    out vec4 fColor;
    void main()
    {
      vec4 vPos = gl_in[0].gl_Position;
      float vAngle = geomAngle[0];
      fColor = vec4( clamp(vPos.x,0.0f,1.0f), clamp(vPos.y,0.0f,1.0f), clamp(vPos.x+vPos.y,0.0f,1.0f), 1.0f );
      mat4 mvp = projection[0] * modelview[0];
      mat4 rotz = { vec4(cos(vAngle*0.5),sin(vAngle*0.5),0,0) , vec4(-sin(vAngle*0.5),cos(vAngle*0.5),0,0) , vec4(0,0,1,0) , vec4(0,0,0,1) };
      mat4 rotx = { vec4(1,0,0,0) , vec4(0,cos(vAngle*0.2),sin(vAngle*0.2),0) , vec4(0,-sin(vAngle*0.2),cos(vAngle*0.2),0) , vec4(0,0,0,1) };
      mat4 T = quadrics_anisotropic_variance_matrix( vPos , 0.1 ) * rotz * rotx;
//      mat4 T = quadrics_oriented_variance_matrix( vPos , vec3(cos(vAngle*0.5),sin(vAngle*0.5)*-sin(vAngle*0.2),cos(vAngle*0.2)) , 0.5 , 0.02 );
      #ifdef CVH_DRAW_OUTLINE_ONLY
      ConvexHull2D hull = quadrics_2D_convex_hull( mvp, T );
      cvh_draw_lines( hull );
      #else
      quadrics_draw_box_lines( mvp, T );
      #endif
    }
    )EOF" } ,
    { GL_FRAGMENT_SHADER , R"EOF(
    #version 460 core
    #extension GL_ARB_shading_language_include : require
    #include <uniform/camera>
    #include <quadrics/raytracing>
    in vec4 fColor;
    layout (location = 0) out vec4 FragColor; // first attached output buffer
    void main()
    {
      FragColor = fColor;
    }
    )EOF" }
    };


  std::vector<GLShaderTypeSource> pass2_shader_sources = {
    { GL_VERTEX_SHADER , R"EOF(
    #version 460 core
    layout (location = 0) in vec4 aPos;
    layout (location = 1) in float aAngle;
    out float geomAngle;
    void main()
    {
      gl_Position = aPos;
      geomAngle = aAngle;
    }
    )EOF" } ,
    { GL_GEOMETRY_SHADER , R"EOF(
    #version 460 core
    #extension GL_ARB_shading_language_include : require
    #include <uniform/camera>
    #include <quadrics/convex_hull>
    #include <quadrics/shape>
    layout (points) in;
    layout (triangle_strip, max_vertices=CVH_TRIANGLE_STRIP_MAX_VERTICES) out;
    in float geomAngle[];
    out vec4 fColor;
    out mat4 fQuadricsMatrix;
    out mat4 fModelViewVarianceInverse;
    void main()
    {
      vec4 vPos = gl_in[0].gl_Position;
      float vAngle = geomAngle[0];
      fColor = vec4( clamp(vPos.x,0.0f,1.0f), clamp(vPos.y,0.0f,1.0f), clamp(vPos.x+vPos.y,0.0f,1.0f), 1.0f );
      mat4 mvp = projection[0] * modelview[0];
      mat4 rotz = { vec4(cos(vAngle*0.5),sin(vAngle*0.5),0,0) , vec4(-sin(vAngle*0.5),cos(vAngle*0.5),0,0) , vec4(0,0,1,0) , vec4(0,0,0,1) };
      mat4 rotx = { vec4(1,0,0,0) , vec4(0,cos(vAngle*0.2),sin(vAngle*0.2),0) , vec4(0,-sin(vAngle*0.2),cos(vAngle*0.2),0) , vec4(0,0,0,1) };
      mat4 T = quadrics_anisotropic_variance_matrix( vPos , 0.1 ) * rotz * rotx;
      mat4 D = quadrics_sphere_matrix();
//      mat4 T = quadrics_oriented_variance_matrix( vPos , vec3(cos(vAngle*0.5),sin(vAngle*0.5)*-sin(vAngle*0.2),cos(vAngle*0.2)) , 0.5 , 0.02 );
//      mat4 D = quadrics_zcylinder_matrix();
      mat4 Ti = inverse( T );
      mat4 Tit = transpose( Ti );
      mat4 modelviewInverse = inverse( modelview[0] );
      mat4 modelviewInverseTranspose = transpose( modelviewInverse );
      fModelViewVarianceInverse = Ti * modelviewInverse;
      fQuadricsMatrix = modelviewInverseTranspose * Tit * D * Ti * modelviewInverse;
      ConvexHull2D hull = quadrics_2D_convex_hull( mvp, T );
      cvh_draw_triangles( hull );
    }
    )EOF" } ,
    { GL_FRAGMENT_SHADER , R"EOF(
    #version 460 core
    #extension GL_ARB_shading_language_include : require
    #include <uniform/camera>
    #include <quadrics/raytracing>
    in vec4 fColor;
    in mat4 fQuadricsMatrix;
    in mat4 fModelViewVarianceInverse;
    layout (location = 0) out vec4 FragColor; // first attached output buffer
    layout (depth_any) out float gl_FragDepth; // free to set depth to arbitrary value
    void main()
    {
      mat4 fProjectionInverse = projection[1];
      vec2 vp = viewport[0];
      vec2 vp_rcp = viewport[1];
      vec3 fc = gl_FragCoord.xyz;
      fc.xy /= vp;
      fc *= 2.0;
      fc -= 1.0;
      vec4 p = fProjectionInverse * vec4(fc, 1.);
      vec3 P = rayQuadricsIntersection( fQuadricsMatrix , fModelViewVarianceInverse , vec3(0,0,0), p.xyz / p.w );
      vec3 N = quadricsSurfaceNormal( fQuadricsMatrix , P );
      vec4 projP = projection[0] * vec4( P, 1. );
      gl_FragDepth = 0.5 * (projP.z / projP.w + 1.0);
      vec4 color;
      vec3 ambientColor = vec3(0.1,0.1,0.1);
      vec3 diffuseColor = fColor.xyz;
      vec3 specularColor = vec3(1,1,1);
      vec3 lightPosition = vec3(1,1,1); 
      float d = max( dot( N , normalize(lightPosition) ) , 0 );
      float s = pow( d , 3 );
      color.xyz = diffuseColor.xyz * d + specularColor * s + ambientColor;
      color.w = fColor.w;
      FragColor = color;
    }
    )EOF" }
    };

  const auto pass1_shader_id = eglm.create_shader_program( "quadrics_outline" , pass1_shader_sources, { .m_enable_flags = { GL_PROGRAM_POINT_SIZE , GL_DEPTH_TEST } } );
  auto & shader1 = eglm.shader_program(pass1_shader_id);
  
  const auto pass2_shader_id = eglm.create_shader_program( "quadrics_raytrace" , pass2_shader_sources, { .m_enable_flags = { GL_PROGRAM_POINT_SIZE , GL_DEPTH_TEST } } );
  auto & shader2 = eglm.shader_program(pass2_shader_id);

  std::cout << "Shader1 Pipeline config :" << std::boolalpha << std::endl;
  shader1.m_pipeline_config.to_stream( std::cout );    

  std::cout << "Shader2 Pipeline config :" << std::boolalpha << std::endl;
  shader2.m_pipeline_config.to_stream( std::cout );    

  auto camera_id = eglm.create_camera("pov");
  auto & camera = eglm.camera(camera_id);
  camera.look_at( {0,5,10} , {0,0,0} );
  camera.perspective(60,width*1.0f/height,0.1f,100.0f); // disable projection
  camera.viewport(width,height);
  camera.attach_shader( eglm.shader_program_ptr(pass1_shader_id) );
  camera.attach_shader( eglm.shader_program_ptr(pass2_shader_id) );
  camera.update_uniform();

  const int n_points = 64;
  const auto elbuf_id = eglm.create_element_buffer("half_elements",n_points/2);
  auto & half_elements_buffer = eglm.element_buffer(elbuf_id);
  auto * el_buf_ptr = half_elements_buffer.map_buffer_write_only();
  for(int i=0;i<n_points;i+=2) { el_buf_ptr[i/2] = i; }
  half_elements_buffer.unmap_buffer();
  
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

    shader1.use();
    glDrawArrays(GL_POINTS, 0, n_points);

    shader2.use();
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
      int motion = true;
      int half_elements = false;
      int left_drag = false;
      int mid_drag = false;
      int right_drag = false;
    } uistate;

    int update_cam = 0;
    auto & ren_surf = eglm.surface("main_window");
    ren_surf.m_event_handler.on_key_release = [&uistate,f=ren_surf.m_event_handler.on_key_release](int key)
    {
       if( f ) f(key);
       if( key == 32 ) uistate.motion = ! uistate.motion;
       if( key == 104 ) uistate.half_elements = ! uistate.half_elements;
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

      shader1.use();
      if( uistate.half_elements ) half_elements_buffer.draw(GL_POINTS);
      else glDrawArrays(GL_POINTS, 0, n_points);
      
      shader2.use();
      if( uistate.half_elements ) half_elements_buffer.draw(GL_POINTS);
      else glDrawArrays(GL_POINTS, 0, n_points);

      ren_surf.swap_buffers();
      if( uistate.motion ) ++i;
    }
  }

  return 0;
}
