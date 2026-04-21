//#define DEBUG 1

//************  shader global parameters **************
uniform vec4 viewport; // renderer viewport
uniform float pointSizeThreshold; // minimum point size

uniform float PointRadiusBias;
uniform float PointRadiusScale;

uniform float LineRadiusBias;
uniform float LineRadiusScale;
//****************************************************



//************  shader output ************************
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

// inverse of (ModelView*variance) matrix
varying vec4 ModelViewVarianceInverseR3;

// surface diffuse color
varying vec4 diffuseColor;

// point size
varying float pointSize;
//****************************************************


//************  shader constants *********************
// max point size
#define MaxPixelSize 512

// epsilon
#define FEPS 0.000001f

// front Z value
#define DEF_Z (1.0f - FEPS)

// uncomment to displace vertex center according to projected bounding box
//#define CENTER_POSITION_FROM_BBOX
//****************************************************


//************  Vertex attribute mapping *************
// Auxiliary Array 0 : gl_Vertex.w
//****************************************************


//************ forward declaration of external functions *************
// forward declaration of variance matrix function
float GetPointRadius(float auxiliary);
float GetLineRadius(float auxiliary);

// forward declaration of color function
vec4 GetColor(vec4 color, float auxiliary);
//********************************************************************



//------------------------------------------------------------------------------
// Generic bounding box computation, works with any quadric type by splatting
// in clip space the bounding box in parameter space;
// in most cases you'll have to use a point scaling factor from 1.05 to 1.5
void ComputePointSizeAndPositionWithProjection( const vec4 p, const mat4 T)
{
  mat4 M = gl_ModelViewProjectionMatrix * T;

  const float dxm = -1.;
  const float dxp =  1.;
  const float dym = -1.;
  const float dyp =  1.;
  const float dzm = -1.;
  const float dzp =  1.;

  vec4 P1 = M * vec4( dxm, dym, dzm, 1. );
  vec4 P2 = M * vec4( dxp, dym, dzm, 1. );
  vec4 P3 = M * vec4( dxp, dyp, dzm, 1. );
  vec4 P4 = M * vec4( dxm, dyp, dzm, 1. );
  vec4 P5 = M * vec4( dxm, dym, dzp, 1. );
  vec4 P6 = M * vec4( dxp, dym, dzp, 1. );
  vec4 P7 = M * vec4( dxp, dyp, dzp, 1. );
  vec4 P8 = M * vec4( dxm, dyp, dzp, 1. );

  P1 /= P1.w;
  P2 /= P2.w;
  P3 /= P3.w;
  P4 /= P4.w;
  P5 /= P5.w;
  P6 /= P6.w;
  P7 /= P7.w;
  P8 /= P8.w;

  float xmin =  min( min( min(P1.x,P2.x) , min(P3.x,P4.x) ) ,  min( min(P5.x,P6.x) , min(P7.x,P8.x) ) );
  float xmax =  max( max( max(P1.x,P2.x) , max(P3.x,P4.x) ) ,  max( max(P5.x,P6.x) , max(P7.x,P8.x) ) );
  float ymin =  min( min( min(P1.y,P2.y) , min(P3.y,P4.y) ) ,  min( min(P5.y,P6.y) , min(P7.y,P8.y) ) );
  float ymax =  max( max( max(P1.y,P2.y) , max(P3.y,P4.y) ) ,  max( max(P5.y,P6.y) , max(P7.y,P8.y) ) );

  float sx = ( xmax - xmin ) * 0.5 * viewport.x;
  float sy = ( ymax - ymin ) * 0.5 * viewport.y;

  // gl_PointSize = ceil( pointScaling * max( sx, sy ) )
  pointSize = ceil( max( sx, sy ) );
#ifdef CENTER_POSITION_FROM_BBOX
  gl_Position = vec4( .5 * ( xmin + xmax ), .5 * ( ymin + ymax ), DEF_Z, 1. );
#else
  gl_Position = gl_ModelViewProjectionMatrix * p;  
#endif

  // for debug only
#ifdef DEBUG
  vec4 pointCenter = gl_ModelViewProjectionMatrix * gl_Vertex;
  gl_Position.z = pointCenter.z / pointCenter.w;
#endif
}

//------------------------------------------------------------------------------
// MAIN
void propFuncVS()
{
	// get center
	vec4 center;
	center.xyz = gl_Vertex.xyz;
	center.w = 1.0f;

	// shape matrix defaults to sphere. set different values for other shapes
	mat4 D = mat4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, -1 );

	// T is the variance matrix
	mat4 T;

	// default scaling for x,y and z axis
	float radius;

	// zero length normal vector indicates we want a cylinder shape,
	// it also tells us the scaling function to choose is the 'Line' version
	float cyl_length2 = dot(gl_Normal, gl_Normal);

	// TODO use negativegl_Vertex.w value to indicate cone shape
	if (cyl_length2 < FEPS) // sphere case
	{
		radius = PointRadiusBias + GetPointRadius( gl_Vertex.w ) * PointRadiusScale;
		T[0] = vec4( radius, 0., 0., 0. );
		T[1] = vec4( 0., radius, 0., 0. );
		T[2] = vec4( 0., 0., radius, 0. );
	}
	else // cylinder case
	{
		float cyl_length = length(gl_Normal);
		vec3 cyl_direction = normalize(gl_Normal);
		radius = LineRadiusBias + GetLineRadius( gl_Vertex.w ) * LineRadiusScale;

		D[2][2] = 0; // zero-ing the 3rd component transform a sphere shape matrix to a z-aligned cylinder shape matrix

		// center of cylinder is the middle of line segment
		center.xyz += gl_Normal * 0.5;

		vec3 W = cyl_direction;
		vec3 upVec = vec3(0,1,0);
		if( abs(dot(W,upVec)) > 0.75) upVec = vec3(1,0,0);
		if( abs(dot(W,upVec)) > 0.75) upVec = vec3(0,0,1);
		vec3 V = normalize( cross( upVec, W ) );
		vec3 U = cross( W, V );

		T[0] = vec4( U*radius, 0 );
		T[1] = vec4( V*radius, 0 );
		T[2] = vec4( W*cyl_length*0.5, 0 );
	}
	T[3] = center; // translation to the vertex position

	// inverse variance matrix
	mat4 Ti = inverse( T );

	// 3rd row of transpose inverse matrix (to get z coordinate in parameter space from tranformed object position)
	ModelViewVarianceInverseR3 = transpose( Ti * gl_ModelViewMatrixInverse )[2];

    // transposed inverse of variance matrix
	mat4 Tit = transpose( Ti );

	// transform quadric matrix into world coordinates and
	// assign values to coefficients to be passed to fragment shader
	mat4 Q = gl_ModelViewMatrixInverseTranspose * Tit * D * Ti * gl_ModelViewMatrixInverse;
    //////////////////
	// | a d e g |
	// | d b f h |
	// | e f c i |
	// | g h i j |
	// ax^2 + by^2 + cz^2 + 2dxy +2exz + 2fyz + 2gx + 2hy + 2iz + j = 0
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

	// set surface color
	diffuseColor = GetColor( gl_Color , gl_Vertex.w );

	ComputePointSizeAndPositionWithProjection( center, T );
	
	if( pointSize > MaxPixelSize )
	{
		pointSize = MaxPixelSize;
	}
	if( radius <= FEPS )
	{
		pointSize = 0;
		diffuseColor.w = 0;
	}
	gl_PointSize = pointSize;
}
