// standardized transformation matrices for all shaders
// Camera tools in EGLRender will refer to this uniform block
uniform camera
{
  mat4 modelview;
  mat4 projection;
  float aspect_ratio;
  vec2 viewport[2]; // fist couple is viewport w and h, second is 1/w, 1/h
};

