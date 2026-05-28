
// on what side of vector ab does the point p lies ?
// 1.0 => left, 0.0 => aligned , -1.0 => right
float vec2side( const vec2 a, const vec2 b, const vec2 p )
{
  const vec2 ab = b - a;
  const vec2 ap = p - a;
  return sign( cross( vec3(ab.x,ab.y,0) , vec3(ap.x,ap.y,0) ).z );
}

bool leftOfOrAligned( const vec2 a, const vec2 b, const vec2 p ) { return vec2side(a,b,p) >= 0.0; }
bool leftOf( const vec2 a, const vec2 b, const vec2 p ) { return vec2side(a,b,p) == 1.0; }
bool rightOfOrAligned( const vec2 a, const vec2 b, const vec2 p ) { return vec2side(a,b,p) <= 0.0; }
bool rightOf( const vec2 a, const vec2 b, const vec2 p ) { return vec2side(a,b,p) == -1.0; }

struct ConvexHull2D
{
  vec2 v[8];
  float z,w;
  int n;
};

ConvexHull2D g_cvh;


int cvh_bottom_right()
{
  int r = 0;
  for(int i=1;i<8;i++)
  {
    if( g_cvh.v[i].y < g_cvh.v[r].y || ( g_cvh.v[i].y == g_cvh.v[r].y && g_cvh.v[i].x > g_cvh.v[r].x ) ) r=i;
  }
  return r;
}

// Swap
void cvh_swap(int i,int j)
{
  vec2 temp = g_cvh.v[i];
  g_cvh.v[i] = g_cvh.v[j];
  g_cvh.v[j] = temp;   
}


// if d = 1 => ascending order, returns true if i is less than (or before) j
// if d = -1 => descending order , returns true if i is greater than (or after) j
// if s = 1 => strict ordering, equal elements return false
// if s = 0 => weak ordering, equal returns true
bool cvh_order(const int i, const int j, const int d, const int s)
{
  // first point is the reference one and is not sorted,
  // it is placed at position 0 before we start sorting the remaing 7 points in [1-7]
  vec2 vi = g_cvh.v[i+1] - g_cvh.v[0];
  vec2 vj = g_cvh.v[j+1] - g_cvh.v[0];
  float side = sign( cross( vec3(vi.x,vi.y,0) , vec3(vj.x,vj.y,0) ).z );
  if( side == d ) return true;
  if( side == 0 )
  {
    float lo = sign( dot(vj,vj) - dot(vi,vi) );
    if( lo == d || ( (s==0) && (lo==0) ) ) return true;
  }
  return false;
}

bool cvh_less         (int i,int j) { return cvh_order(i,j, 1,1); }
bool cvh_less_equal   (int i,int j) { return cvh_order(i,j, 1,0); }
bool cvh_greater      (int i,int j) { return cvh_order(i,j,-1,1); }
bool cvh_greater_equal(int i,int j) { return cvh_order(i,j,-1,0); }

void cvh_sift_down(int root, const int bottom)
{
  for(int passes=0;passes<4;passes++)
  {  
    int maxChild = root * 2 + 1;
   
    // Find the biggest child
    if(maxChild < bottom)
    {
      int otherChild = maxChild + 1;
      // Reversed for stability
      maxChild = cvh_greater(otherChild,maxChild) ? otherChild : maxChild;
    }
    else
    {
      // Don't overflow
      if(maxChild > bottom) return;
    }
   
    // If we have the correct ordering, we are done.
    if( cvh_greater_equal(root,maxChild) ) return;
   
    // Swap
    cvh_swap( root+1 , maxChild+1 );
   
    // Tail queue recursion. Will be compiled as a loop with correct compiler switches.
    //cvh_sift_down(maxChild, bottom);
    root = maxChild;
  }
}

void cvh_heap_sort()
{
  const int array_size = 7;
  for (int i = (array_size / 2); i >= 0; i--)
  {
    cvh_sift_down(i, array_size - 1);
  }
 
  for (int i = array_size-1; i >= 1; i--)
  {
    // Swap
    cvh_swap( 0+1 , i+1 );
    cvh_sift_down(0, i-1);
  }
}

//------------------------------------------------------------------------------
// Generic bounding box computation, works with any quadric type by splatting
// in clip space the bounding box in parameter space;
// in most cases you'll have to use a point scaling factor from 1.05 to 1.5
ConvexHull2D quadricsProj2DConvexHull( const mat4 modelViewProjMatrix, const mat4 varianceMatrix )
{
  const mat4 M = modelViewProjMatrix * varianceMatrix;

  const float dxm = -1;
  const float dxp =  1;
  const float dym = -1;
  const float dyp =  1;
  const float dzm = -1;
  const float dzp =  1;

  vec4 v0 = M * vec4( dxm, dym, dzm, 1. );
  vec4 v1 = M * vec4( dxp, dym, dzm, 1. );
  vec4 v2 = M * vec4( dxp, dyp, dzm, 1. );
  vec4 v3 = M * vec4( dxm, dyp, dzm, 1. );
  vec4 v4 = M * vec4( dxm, dym, dzp, 1. );
  vec4 v5 = M * vec4( dxp, dym, dzp, 1. );
  vec4 v6 = M * vec4( dxp, dyp, dzp, 1. );
  vec4 v7 = M * vec4( dxm, dyp, dzp, 1. );

  g_cvh.v[0] = v0.xy / v0.w;
  g_cvh.v[1] = v1.xy / v1.w;
  g_cvh.v[2] = v2.xy / v2.w;
  g_cvh.v[3] = v3.xy / v3.w;
  g_cvh.v[4] = v4.xy / v4.w;
  g_cvh.v[5] = v5.xy / v5.w;
  g_cvh.v[6] = v6.xy / v6.w;
  g_cvh.v[7] = v7.xy / v7.w;
  g_cvh.n = 8;

  int first = cvh_bottom_right();
  cvh_swap(0,first);
  cvh_heap_sort();

  g_cvh.n=2;
  for(int i=2;i<8;i++)
  {
    if( rightOfOrAligned( g_cvh.v[g_cvh.n-2] , g_cvh.v[g_cvh.n-1] , g_cvh.v[i] ) )
    {
      g_cvh.v[g_cvh.n-1] = g_cvh.v[i];
    }
    else
    {
      g_cvh.v[g_cvh.n] = g_cvh.v[i];
      ++ g_cvh.n;
    }
  }
  
  return g_cvh;
}
