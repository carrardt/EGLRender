// standardized transformation matrices for all shaders
// Camera tools in EGLRender will refer to this uniform block
uniform camera
{
  mat4 modelview[2]; // modelview and modelviewInverse
  mat4 projection[2]; // projection and projectionInverse
  float aspect_ratio;
  vec2 viewport[2]; // fist couple is viewport w and h, second is 1/w, 1/h
};

