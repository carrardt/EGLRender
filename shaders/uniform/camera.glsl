// standardized transformation matrices for all shaders
// Camera tools in EGLRender will refer to this uniform block
uniform camera
{
  mat4 modelview;
  mat4 projection;
  uint npasses;
};

