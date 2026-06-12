mat3 quaternion_to_matrix(vec4 q)
{
  return mat3(
    vec3( 1.0f - 2.0f*q.y*q.y - 2.0f*q.z*q.z , 2.0f*q.x*q.y - 2.0f*q.z*q.w        , 2.0f*q.x*q.z + 2.0f*q.y*q.w )
  , vec3( 2.0f*q.x*q.y + 2.0f*q.z*q.w        , 1.0f - 2.0f*q.x*q.x - 2.0f*q.z*q.z , 2.0f*q.y*q.z - 2.0f*q.x*q.w )
  , vec3( 2.0f*q.x*q.z - 2.0f*q.y*q.w        , 2.0f*q.y*q.z + 2.0f*q.x*q.w        , 1.0f - 2.0f*q.x*q.x - 2.0f*q.y*q.y )
  );
}
