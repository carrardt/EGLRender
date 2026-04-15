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
#include <EGLRender/egl_renderer.h>
#include <EGLRender/native_window_event_handler.h>
#include <string>

namespace EGLRender
{

  enum class EGLRenderSurfaceClass
  {
    PBUFFER ,
    FULLSCREEN ,
    WINDOW
  };

  const char* render_surface_type_as_string(EGLRenderSurfaceClass surf_type);

  struct EGLRenderSurface
  {
    const EGLRenderer * m_egl = nullptr;

    EGLRenderSurfaceClass m_surface_type = EGLRenderSurfaceClass::PBUFFER;

    EGLint m_width = 1280;
    EGLint m_height = 720;

    EGLint m_color_bits = 8;
    EGLint m_alpha_bits = 8;
    EGLint m_depth_bits = 24;
    EGLint m_stencil_bits = 8;

    std::string m_window_title = "EGL";

#ifndef NDEBUG
    NativeWindowEventHandler m_event_handler = native_window_event_printer();
#else
    NativeWindowEventHandler m_event_handler = {};
#endif

    EGLNativeWindowType m_native_win = init_native_window(m_egl,m_surface_type,m_width,m_height, m_window_title ) ;

    EGLConfig m_eglCfg = init_config(m_egl,m_surface_type,m_color_bits,m_alpha_bits,m_stencil_bits,m_depth_bits);
    EGLContext m_eglCtx = init_surface_ctx(m_egl,m_eglCfg);
    EGLSurface m_eglSurf = init_surface(m_egl,m_eglCfg,m_width,m_height,m_surface_type,m_native_win);

    static EGLNativeWindowType init_native_window(const EGLRenderer * egl, EGLRenderSurfaceClass surf_type, EGLint w, EGLint h, std::string_view name = "EGL" );
    static EGLConfig init_config(const EGLRenderer * egl, EGLRenderSurfaceClass surf_type, EGLint colbits, EGLint alphabits, EGLint stencilbits, EGLint zbits);
    static EGLContext init_surface_ctx(const EGLRenderer * egl, EGLConfig cfg);
    static EGLSurface init_surface(const EGLRenderer * egl, EGLConfig cfg, EGLint& w, EGLint& h, EGLRenderSurfaceClass surf_type, EGLNativeWindowType win);

    void process_events();
    void make_current();
    void swap_buffers();

    inline GLint width() const { return m_width; }
    inline GLint height() const { return m_height; }
    inline EGLNativeWindowType native_window() const { return m_native_win; }

    ~EGLRenderSurface();
  };

}
