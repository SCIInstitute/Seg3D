// GLSL vertex shader for rendering a slice
#version 110

uniform bool enable_lighting;

void compute_lighting();

void main()
{ 
  if ( enable_lighting )
  {
    compute_lighting();
  }
  
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_TexCoord[1] = gl_MultiTexCoord1;
  gl_Position = ftransform();
} 