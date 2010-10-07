// GLSL vertex shader for rendering the orientation arrows
#version 110

void compute_lighting();

void main()
{ 
  compute_lighting();
  
  gl_FrontColor = gl_Color;
  gl_BackColor = gl_Color;

  gl_Position = ftransform();
} 