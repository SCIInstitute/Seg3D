// GLSL fragment shader for per-fragment fog computation

// Fog depth in eye space. 
// This is a workaround for Macs with ATI X1600/X1900. 
// On those cards accessing gl_FragCoord would cause fallback to software renderer. 
varying float fog_depth;

float compute_fog_factor()
{
  float z = fog_depth;
  float fog_factor = exp( -gl_Fog.density * gl_Fog.density * z * z );
  fog_factor = clamp( fog_factor, 0.0, 1.0 );
  
  return fog_factor;
}
