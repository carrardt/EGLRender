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

#include <EGLRender/egl_surface.h>
#include <EGLRender/egl_error.h>

#include <iostream>

namespace EGLRender
{

  const char* render_surface_type_as_string(EGLRenderSurfaceClass surf_type)
  {
    switch( surf_type )
    {
      case EGLRenderSurfaceClass::PBUFFER : return "PBuffer" ; break;
      case EGLRenderSurfaceClass::FULLSCREEN : return "FullScreen"; break;
      case EGLRenderSurfaceClass::WINDOW : return "Window"; break;
    }
    return "unknown";
  }

  EGLNativeWindowType EGLRenderSurface::init_native_window(const EGLRenderer * egl, EGLRenderSurfaceClass surf_type, EGLint w, EGLint h, std::string_view name)
  {
    if( egl == nullptr || surf_type == EGLRenderSurfaceClass::PBUFFER ) return {};
    else return platform_get_native_window( egl->m_native_dpy, w, h, surf_type == EGLRenderSurfaceClass::FULLSCREEN, name );
  }

  EGLConfig EGLRenderSurface::init_config(const EGLRenderer * egl, EGLRenderSurfaceClass surf_type, EGLint colbits, EGLint alphabits, EGLint stencilbits, EGLint zbits)
  {
    const EGLint config_attribs[] =
      { EGL_SURFACE_TYPE, ( surf_type == EGLRenderSurfaceClass::PBUFFER ) ? EGL_PBUFFER_BIT : EGL_WINDOW_BIT
      , EGL_BLUE_SIZE, colbits
      , EGL_GREEN_SIZE, colbits
      , EGL_RED_SIZE, colbits
      , EGL_ALPHA_SIZE , alphabits
      , EGL_DEPTH_SIZE, zbits
      , EGL_STENCIL_SIZE, stencilbits
      , EGL_RENDERABLE_TYPE, egl->m_gles ? EGL_OPENGL_ES_BIT : EGL_OPENGL_BIT
      , EGL_NONE };

    EGLConfig cfg;
    // Select an appropriate configuration
//    std::cout << "EGL config : color="<<colbits<<", alpha="<<alphabits<<", stencil="<<stencilbits<<", depth="<<zbits<<std::endl;
    EGLint numConfigs = 0;
    if( ! eglChooseConfig( egl->m_eglDpy, config_attribs, &cfg, 1, &numConfigs) )
    {
      std::cerr << "Failed to find EGL config" << std::endl;
      std::abort();
    }
//    std::cout << "Found "<<numConfigs<<" valid config(s)" << std::endl;
    return cfg;
  }

  EGLContext EGLRenderSurface::init_surface_ctx(const EGLRenderer * egl, EGLConfig cfg)
  {
    static const EGLint context_attribs[] = { EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 0, EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT, EGL_NONE };
    EGLContext ctx;
//    std::cout << "Create surface context ..." << std::flush;
    ctx = eglCreateContext( egl->m_eglDpy, cfg, EGL_NO_CONTEXT, context_attribs );
    EGL_CHECK_ERROR();
//    std::cout << " ok" << std::endl;
    return ctx;
  }

  EGLSurface EGLRenderSurface::init_surface(const EGLRenderer * egl, EGLConfig cfg, EGLint& w, EGLint& h, EGLRenderSurfaceClass surf_type, EGLNativeWindowType win)
  {
    EGLSurface surf;
//    std::cout << "Window = "<<win<<std::endl;
//    std::cout << "Create surface ..." << std::flush;
    if( surf_type == EGLRenderSurfaceClass::PBUFFER )
    {
      const EGLint surface_attribs[] = { EGL_WIDTH, w, EGL_HEIGHT, h, EGL_NONE };
      surf = eglCreatePbufferSurface( egl->m_eglDpy, cfg, surface_attribs );
    }
    else
    {
      surf = eglCreateWindowSurface(egl->m_eglDpy, cfg, win, NULL );
      EGL_CHECK_ERROR();
      eglQuerySurface(egl->m_eglDpy,surf,EGL_WIDTH,&w);
      eglQuerySurface(egl->m_eglDpy,surf,EGL_HEIGHT,&h);
    }
    EGL_CHECK_ERROR();
//    std::cout << " type="<<render_surface_type_as_string(surf_type)<<" size="<<w<<"x"<<h<<std::endl;

    return surf;
  }

  void EGLRenderSurface::process_events()
  {
    platform_native_window_process_events(m_egl->native_display(), native_window(), m_event_handler);
  }

  void EGLRenderSurface::make_current()
  {
    eglMakeCurrent( m_egl->m_eglDpy, m_eglSurf, m_eglSurf, m_eglCtx );
    EGL_CHECK_ERROR();
  }

  void EGLRenderSurface::swap_buffers()
  {
    eglSwapBuffers( m_egl->m_eglDpy, m_eglSurf);
    EGL_CHECK_ERROR();
  }

  EGLRenderSurface::~EGLRenderSurface()
  {
    eglDestroySurface(m_egl->m_eglDpy,m_eglSurf);
    eglDestroyContext(m_egl->m_eglDpy,m_eglCtx);
  }

}
