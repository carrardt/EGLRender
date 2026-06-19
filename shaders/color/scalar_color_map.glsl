vec4 scalar_pv_colormap(float x, float lo, float hi)
{
  x = clamp( (x-lo)/(hi-lo) , 0.0 , 1.0 );
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
