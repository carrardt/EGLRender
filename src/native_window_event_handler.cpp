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

#include <EGLRender/native_window_event_handler.h>
#include <iostream>

namespace EGLRender
{

  NativeWindowEventHandler native_window_event_printer()
  {
    return { [](int x,int y){ std::cout<<"on_mouse_move "<<x<<" "<<y<<std::endl; }
           , [](int k){ std::cout<<"on_key_release "<<k<<std::endl; }
           , [](int k){ std::cout<<"on_key_press "<<k<<std::endl; }
           , [](int state, int b, int x,int y){ std::cout<<"on_button_press "<<state<<" "<<b<<" "<<x<<" "<<y<<std::endl; }
           , [](int state, int b, int x,int y){ std::cout<<"on_button_press "<<state<<" "<<b<<" "<<x<<" "<<y<<std::endl; }
           };
  }

}
