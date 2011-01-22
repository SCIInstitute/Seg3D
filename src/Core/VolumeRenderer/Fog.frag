// GLSL fragment shader for per-fragment fog computation

float compute_fog_factor()
{
  float z = gl_FragCoord.z / gl_FragCoord.w;
  float fog_factor = exp( -gl_Fog.density * gl_Fog.density * z * z );
  
  return clamp( fog_factor, 0.0, 1.0 );
}
