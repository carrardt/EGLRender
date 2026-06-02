vec4 scalar_pv_colormap(float x, float lo, float hi)
{
  x = (x-lo)/(hi-lo);
  if( x < 0.5 )
  {
    x *= 2;
    return vec4( x , x , 1 , 1 );
  }
  else
  {
    x = (1-x)*2;
    return vec4( 1 , x , x , 1 );
  }
}
