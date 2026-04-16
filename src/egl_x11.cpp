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

#ifdef EGLRENDER_USE_X11

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include <iostream>

namespace EGLRender
{

  EGLNativeDisplayType platform_get_native_display()
  {
    /* open standard display (primary screen) */
    Display * xdisplay = XOpenDisplay ( NULL );
    if ( xdisplay == NULL )
    {
      std::cerr<< "Error opening X display" << std::endl;
    }
    return xdisplay;
  }

  EGLNativeWindowType platform_get_native_window(EGLNativeDisplayType native_display, int w, int h, bool fullscreen, std::string_view name )
  {
    Display* xdisplay = (Display*) native_display;
    Window root = DefaultRootWindow( xdisplay );
    if( fullscreen ) return root;

    // list all events this window accepts
    XSetWindowAttributes swa;
    swa.event_mask =
    StructureNotifyMask |
    ExposureMask        |
    PointerMotionMask   |
    KeyPressMask        |
    KeyReleaseMask      |
    ButtonPressMask     |
    ButtonReleaseMask;

    // Xlib's window creation
    Window win  =  XCreateWindow (
        xdisplay, root, 0, 0, w, h,   0,
        CopyFromParent, InputOutput, CopyFromParent, CWEventMask,
       &swa );

    XMapWindow ( xdisplay , win );         // make window visible
    XStoreName ( xdisplay , win , name.data() );
    return win;
  }

  void platform_native_window_process_events(EGLNativeDisplayType dpy, EGLNativeWindowType win, const NativeWindowEventHandler& cb)
  {
    static const long global_event_mask =
      StructureNotifyMask |
      ExposureMask        |
      PointerMotionMask   |
      KeyPressMask        |
      KeyReleaseMask      |
      ButtonPressMask     |
      ButtonReleaseMask;

    Display* xdisplay = (Display*) dpy;
    if ( XPending ( xdisplay ) )
    {
      XEvent  xev;
      while (XCheckWindowEvent(xdisplay, win, global_event_mask, &xev))
      {
        switch (xev.type)
        {
          case MotionNotify:
            cb.on_mouse_move( xev.xbutton.x, xev.xbutton.y );
            break;
          case KeyRelease:
            cb.on_key_release( XLookupKeysym (&xev.xkey, 0) );
            break;
          case KeyPress:
            cb.on_key_press( XLookupKeysym (&xev.xkey, 0) );
            break;
          case ButtonPress:
            cb.on_button_press( xev.xbutton.state, xev.xbutton.button, xev.xbutton.x, xev.xbutton.y );
            break;
          case ButtonRelease:
            cb.on_button_release( xev.xbutton.state, xev.xbutton.button, xev.xbutton.x, xev.xbutton.y );
            break;
        }
      }
    }
  }

}

#endif


