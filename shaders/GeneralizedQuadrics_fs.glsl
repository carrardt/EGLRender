//#define DEBUG 1

uniform vec4 viewport; // vtk renderer's viewport
uniform float pointSizeThreshold; // minimum point size

// quadric coefficients
// | a d e g |
// | d b f h |
// | e f c i |
// | g h i j |
// ax^2 + by^2 + cz^2 + 2dxy +2exz + 2fyz + 2gx + 2hy + 2iz + j = 0
varying float a;
varying float b;
varying float c;
varying float d;
varying float e;
varying float f;
varying float g;
varying float h;
varying float i;
varying float j;

// inverse of variance matrix
varying vec4 ModelViewVarianceInverseR3;

// surface diffuse color
varying vec4 diffuseColor;

// point size
varying float pointSize;

// constants
#define FLAT_SHADE_POINT_SIZE 4.0f //if point size < 1 use flat shading
#define MAX_LIGHTS 1

bool InBounds( vec3 P )
{
	float z = dot( ModelViewVarianceInverseR3 , vec4( P, 1. ) );
	return ( z >= -1.01f ) && ( z <= 1.01f );
}

vec3 ComputeRayQuadricIntersection(vec3 P, vec3 D)
{
	float A = dot(vec3(a, b, c), D * D) + 2. * dot(vec3(d, e, f), D.xxy * D.yzz);
	float B = 2. * dot(vec3(g, h, i), D);
	float C = j;

	float delta = B * B - 4. * A * C;
	if (delta < 0.0)
	{
		discard;
		return vec3(0,0,0);
	}

	float d = sqrt(delta);
	A = 1. / A;
	A *= 0.5;
	float t2 = A * (-B + d);
	float t1 = A * (-B - d);

	vec3 P1 = P + D * min( t1, t2 );
	if( InBounds( P1 ) )
	{
		return P1;
	}

	vec3 P2 = P + D * max( t1, t2 );
	if( InBounds( P2 ) )
	{
		return P2;
	}

	discard;
	return vec3(0,0,0);
}

// compute unit normal from gradient
vec3 ComputeNormal(vec3 P)
{
	return normalize(vec3(dot(vec4(a, d, e, 1.), vec4(P, g)), // should multiply by 2 for actual gradient
			dot(vec4(d, b, f, 1.), vec4(P, h)), // should multiply by 2 for actual gradient
			dot(vec4(e, f, c, 1.), vec4(P, i)) // should multiply by 2 for actual gradient
			));
}

//------------------------------------------------------------------------------
// MAIN
void propFuncFS(void)
{
	vec3 fc = gl_FragCoord.xyz;
	fc.xy /= viewport.xy;
	fc *= 2.0;
	fc -= 1.0;
	
	vec4 p = gl_ProjectionMatrixInverse * vec4(fc, 1.);

	// compute intersection point
	vec3 P = ComputeRayQuadricIntersection( vec3(0,0,0), p.xyz / p.w );

	// compute surface normal
	vec3 N = ComputeNormal( P );

	// compute pixel depth
	float z = dot(vec4(P, 1.), gl_ProjectionMatrixTranspose[2]);
	float w = dot(vec4(P, 1.), gl_ProjectionMatrixTranspose[3]);
#ifndef DEBUG
	gl_FragDepth = 0.5 * (z / w + 1.0);
#endif

	// shading
	vec4 color;
	vec3 specularColor = vec3(1,1,1);
	color.xyz = vec3(0,0,0);
	for(int li=0;li<MAX_LIGHTS;li++)
	{
		float d = max( dot( N , normalize(gl_LightSource[li].position) ) , 0 );
		float s = pow( d , gl_FrontMaterial.shininess );
		color.xyz = color.xyz + diffuseColor.xyz * d + gl_LightSource[li].specular.xyz * s + gl_LightSource[li].ambient.xyz;
	}
	color.w = diffuseColor.w;

	// fall back to constant color if point too small
	if ( pointSize < FLAT_SHADE_POINT_SIZE )
	{
		color.xyz = diffuseColor.xyz;
	}

#ifdef DEBUG
	gl_FragColor = diffuseColor;
#else
	gl_FragColor = color;
#endif
}

