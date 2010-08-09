// GLSL vertex shader for rendering an isosurface
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
  gl_FrontColor = gl_Color;
  gl_BackColor = gl_Color;
  gl_Position = ftransform();
} 