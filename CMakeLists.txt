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

find_package(OpenGL COMPONENTS EGL)

set(ONIKA_EGL_NATIVE_DISPLAY "X11" CACHE STRING "Native display for EGL on screen rendering")
set_property(CACHE ONIKA_EGL_NATIVE_DISPLAY PROPERTY STRINGS X11 GLES WIN)
if(ONIKA_EGL_NATIVE_DISPLAY STREQUAL "X11")
  find_package(X11)
  set(ONIKA_EGL_LIBS OpenGL::EGL X11::X11)
  set(ONIKA_EGL_DEFINITIONS ONIKA_EGL_USE_X11=1)
  set(ONIKA_EGL_PLATFORM_SRC egl_x11.cpp)
elseif(ONIKA_EGL_NATIVE_DISPLAY STREQUAL "GLES")
  message(FATAL_ERROR "GLES native display not supported")
elseif(ONIKA_EGL_NATIVE_DISPLAY STREQUAL "WIN")
  message(FATAL_ERROR "WIN native display not supported")
else()
  message(FATAL_ERROR "Unknown native display ${ONIKA_EGL_NATIVE_DISPLAY}")
endif()

option(EGL_CHECK_ERROR_ABORT_ON_FAILURE "EGL errors are fatal" ON)
if(EGL_CHECK_ERROR_ABORT_ON_FAILURE)
  list(APPEND ONIKA_EGL_DEFINITIONS EGL_CHECK_ERROR_ABORT_ON_FAILURE=1)
endif()

#add_executable(egl_test egl_test.cpp egl_render_manager.cpp egl_error.cpp egl_surface.cpp gl_shader.cpp gl_vbo.cpp egl_renderer.cpp native_window_event_handler.cpp ${ONIKA_EGL_PLATFORM_SRC})
#target_compile_options(egl_test PUBLIC ${ONIKA_COMPILE_OPTIONS})
#target_compile_definitions(egl_test PUBLIC ${ONIKA_COMPILE_DEFINITIONS} ${ONIKA_EGL_DEFINITIONS})
#target_include_directories(egl_test PUBLIC ${ONIKA_INCLUDE_DIRS})
#target_link_directories(egl_test PUBLIC ${ONIKA_LIBRARY_DIRS})
#target_link_libraries(egl_test PUBLIC ${ONIKA_LIBRARIES} ${ONIKA_EGL_LIBS})

