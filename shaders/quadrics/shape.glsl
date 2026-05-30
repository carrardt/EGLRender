// suitable for spheres (rotation invariant objects)
mat4 quadrics_anisotropic_variance_matrix(vec4 center, float radius)
{
  mat4 T = { vec4( radius, 0., 0., 0. )
           , vec4( 0., radius, 0., 0. )
           , vec4( 0., 0., radius, 0. )
           , center };
  return T;
}

// suitable for object with an axis simetry (i.e. cones/cylinders): give axis direction and length
// radius is the scaling factor orthogonal to axis (i.e. cone base radius or cylinder diameter)
mat4 quadrics_oriented_variance_matrix(vec4 center, vec3 direction, float length, float radius)
{
  vec3 W = normalize(direction);
  vec3 upVec = vec3(0,1,0);
  if( abs(dot(W,upVec)) > 0.75) upVec = vec3(1,0,0);
  if( abs(dot(W,upVec)) > 0.75) upVec = vec3(0,0,1);
  vec3 V = normalize( cross( upVec, W ) );
  vec3 U = cross( W, V );
  mat4 T = { vec4( U*radius, 0 )
           , vec4( V*radius, 0 )
           , vec4( W*length*0.5, 0 )
           , center };
  return T;
}

mat4 quadrics_sphere_matrix()
{
	return mat4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, -1 );
}

mat4 quadrics_zcylinder_matrix()
{
	return mat4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, -1 );
}

mat4 quadrics_zcone_matrix()
{
	return mat4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, -1, 0,
			0, 0, 0, 0 );
}

mat4 quadrics_shape_transform( mat4 Q , mat4 T )
{
  mat4 Ti = inverse(T);
  mat4 Tit = transpose(Ti);
  return Tit * Q * Ti;
}
