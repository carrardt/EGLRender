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

cmake_minimum_required(VERSION 3.28)

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

# make cache variables for install destinations
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(OpenGL COMPONENTS EGL OpenGL)
set(EGLRENDER_LIBS OpenGL::OpenGL OpenGL::EGL)

set(EGLRENDER_NATIVE_DISPLAY "X11" CACHE STRING "Native display for EGL on screen rendering")
set_property(CACHE EGLRENDER_NATIVE_DISPLAY PROPERTY STRINGS X11 GLES WIN)
if(EGLRENDER_NATIVE_DISPLAY STREQUAL "X11")
  find_package(X11)
  list(APPEND EGLRENDER_LIBS X11::X11)
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

set(EGLRENDER_GPU_COMPUTE_API "NONE" CACHE STRING "Interoperability with GPU compute api")
set_property(CACHE EGLRENDER_GPU_COMPUTE_API PROPERTY STRINGS NONE CUDA HIP)
if(NOT EGLRENDER_GPU_COMPUTE_API STREQUAL "NONE")
  if(EGLRENDER_GPU_COMPUTE_API STREQUAL "CUDA")
    find_package(CUDAToolkit REQUIRED)
    list(APPEND EGLRENDER_LIBS CUDA::cudart)
    list(APPEND EGLRENDER_DEFINITIONS EGLRENDER_GPU_COMPUTE_API=1 EGLRENDER_USE_CUDA=1)
  elseif(EGLRENDER_GPU_COMPUTE_API STREQUAL "HIP")
    find_package(ROCmCMakeBuildTools REQUIRED)
    list(APPEND EGLRENDER_LIBS REQUIRED hip::hiprt)
    list(APPEND EGLRENDER_DEFINITIONS EGLRENDER_GPU_COMPUTE_API=1 EGLRENDER_USE_HIP=1)
  else()
    message(FATAL_ERROR "Unsupported GPU compute API ${EGLRENDER_GPU_COMPUTE_API}")
  endif()
endif()

add_library(EGLRender SHARED
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
target_include_directories(EGLRender PUBLIC
   "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
   "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(EGLRender PUBLIC ${EGLRENDER_LIBS})
set_property(TARGET EGLRender PROPERTY VERSION ${PROJECT_VERSION})
set_property(TARGET EGLRender PROPERTY SOVERSION ${PROJECT_VERSION_MAJOR})
set_property(TARGET EGLRender PROPERTY INTERFACE_EGLRender_MAJOR_VERSION ${PROJECT_VERSION_MAJOR})
set_property(TARGET EGLRender APPEND PROPERTY COMPATIBLE_INTERFACE_STRING EGLRender_MAJOR_VERSION)

# install targets that are exportable with a package file
install(TARGETS EGLRender
        EXPORT EGLRenderTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
install(DIRECTORY include/EGLRender TYPE INCLUDE)
install(EXPORT EGLRenderTargets NAMESPACE EGLRender:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake)

# sample test with vertex, geometry and fragment shader
option(EGLRENDER_BUILD_TESTS "Build tests" OFF)
if(EGLRENDER_BUILD_TESTS)
  add_executable(egl_test tests/egl_test.cpp)
  target_link_libraries(egl_test EGLRender)
  install(TARGETS egl_test DESTINATION ${CMAKE_INSTALL_BINDIR})
  if(EGLRENDER_GPU_COMPUTE_API STREQUAL "CUDA")
    enable_language(CUDA)
    add_executable(egl_cugraphis tests/egl_cugraphics.cu)
    target_compile_features(egl_cugraphis PUBLIC cuda_std_20)
    target_link_libraries(egl_cugraphis EGLRender)
    install(TARGETS egl_cugraphis DESTINATION ${CMAKE_INSTALL_BINDIR})
  endif()
endif()

# Generates and exports cmake package files for import into another project
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/EGLRenderConfig.cmake" INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake)
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/EGLRenderConfigVersion.cmake" VERSION "${PROJECT_VERSION}" COMPATIBILITY AnyNewerVersion)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/EGLRenderConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/EGLRenderConfigVersion.cmake" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake)
