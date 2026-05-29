
// test only z component, as the only primitives exteinding over the box are z-aligned cylinders and cones
bool pointInUnitBox( mat4 modelViewVarianceInverse, vec3 p )
{
	const vec4 pu = modelViewVarianceInverse * vec4( p, 1. ) ;
	return ( pu.x >= -1.01f ) && ( pu.x <= 1.01f ) && ( pu.y >= -1.01f ) && ( pu.y <= 1.01f ) && ( pu.z >= -1.01f ) && ( pu.z <= 1.01f );
}

vec3 rayQuadricsIntersection(mat4 Q, mat4 modelViewVarianceInverse, const vec3 rayOrigin, const vec3 rayDirection)
{
  //////////////////
	// | a d e g |
	// | d b f h |
	// | e f c i |
	// | g h i j |
	// ax^2 + by^2 + cz^2 + 2dxy +2exz + 2fyz + 2gx + 2hy + 2iz + j = 0
  float a,b,c,d,e,f,g,h,i,j;
	a = Q[ 0 ][ 0 ];
	b = Q[ 1 ][ 1 ];
	c = Q[ 2 ][ 2 ];
	d = Q[ 1 ][ 0 ];
	e = Q[ 2 ][ 0 ];
	f = Q[ 2 ][ 1 ];
	g = Q[ 3 ][ 0 ];
	h = Q[ 3 ][ 1 ];
	i = Q[ 3 ][ 2 ];
	j = Q[ 3 ][ 3 ];

  const vec3 P = rayOrigin;
  const vec3 D = rayDirection;
  
	float A = dot(vec3(a, b, c), D * D) + 2. * dot(vec3(d, e, f), D.xxy * D.yzz);
	float B = 2. * dot(vec3(g, h, i), D);
	float C = j;

	float delta = B * B - 4. * A * C;
	if (delta < 0.0)
	{
		discard;
		return vec3(0,0,0);
	}

	float eq_d = sqrt(delta);
	A = 1. / A;
	A *= 0.5;
	float t2 = A * (-B + eq_d);
	float t1 = A * (-B - eq_d);

	vec3 P1 = P + D * min( t1, t2 );
	if( pointInUnitBox( modelViewVarianceInverse, P1 ) )
	{
		return P1;
	}

	vec3 P2 = P + D * max( t1, t2 );
	if( pointInUnitBox( modelViewVarianceInverse, P2 ) )
	{
		return P2;
	}

	discard;
	return vec3(0,0,0);
}


// compute unit normal from gradient
vec3 quadricsSurfaceNormal(mat4 Q, vec3 P)
{
  //////////////////
	// | a d e g |
	// | d b f h |
	// | e f c i |
	// | g h i j |
	// ax^2 + by^2 + cz^2 + 2dxy +2exz + 2fyz + 2gx + 2hy + 2iz + j = 0
  float a,b,c,d,e,f,g,h,i,j;
	a = Q[ 0 ][ 0 ];
	b = Q[ 1 ][ 1 ];
	c = Q[ 2 ][ 2 ];
	d = Q[ 1 ][ 0 ];
	e = Q[ 2 ][ 0 ];
	f = Q[ 2 ][ 1 ];
	g = Q[ 3 ][ 0 ];
	h = Q[ 3 ][ 1 ];
	i = Q[ 3 ][ 2 ];
	j = Q[ 3 ][ 3 ];

  vec3 gradient = vec3( dot(vec4(a, d, e, 1.), vec4(P, g)) *2
                      , dot(vec4(d, b, f, 1.), vec4(P, h)) *2
                      , dot(vec4(e, f, c, 1.), vec4(P, i)) *2 );

	return normalize( gradient );
}

