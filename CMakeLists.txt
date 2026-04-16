# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#

cmake_minimum_required(VERSION 3.26)

# ========================================
# === Compiler toolchain configuration ===
# ========================================
# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==========================
# === project definition ===
# ==========================
project(EGLRender VERSION 1.0.0 LANGUAGES CXX)

find_package(OpenGL COMPONENTS EGL)

set(EGLRENDER_NATIVE_DISPLAY "X11" CACHE STRING "Native display for EGL on screen rendering")
set_property(CACHE EGLRENDER_NATIVE_DISPLAY PROPERTY STRINGS X11 GLES WIN)
if(EGLRENDER_NATIVE_DISPLAY STREQUAL "X11")
  find_package(X11)
  set(EGLRENDER_LIBS OpenGL::EGL X11::X11)
  set(EGLRENDER_DEFINITIONS EGLRENDER_USE_X11=1)
  set(EGLRENDER_PLATFORM_SRC egl_x11.cpp)
elseif(EGLRENDER_NATIVE_DISPLAY STREQUAL "GLES")
  message(FATAL_ERROR "GLES native display not supported")
elseif(EGLRENDER_NATIVE_DISPLAY STREQUAL "WIN")
  message(FATAL_ERROR "WIN native display not supported")
else()
  message(FATAL_ERROR "Unknown native display ${EGLRENDER_NATIVE_DISPLAY}")
endif()

option(EGL_CHECK_ERROR_ABORT_ON_FAILURE "EGL errors are fatal" ON)
if(EGL_CHECK_ERROR_ABORT_ON_FAILURE)
  list(APPEND EGLRENDER_DEFINITIONS EGL_CHECK_ERROR_ABORT_ON_FAILURE=1)
endif()

add_library(EGLRender
	src/egl_render_manager.cpp
	src/egl_error.cpp
	src/egl_surface.cpp
	src/egl_renderer.cpp
	src/egl_platform.cpp
	src/gl_shader.cpp
	src/gl_vbo.cpp
	src/gl_camera.cpp
	src/gl_element_buffer.cpp
	src/native_window_event_handler.cpp
	src/${EGLRENDER_PLATFORM_SRC})
target_compile_definitions(EGLRender PRIVATE ${EGLRENDER_DEFINITIONS})
target_include_directories(EGLRender PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(EGLRender PUBLIC ${EGLRENDER_LIBS})

add_executable(egl_test tests/egl_test.cpp)
target_link_libraries(egl_test EGLRender)

