// GLSL fragment shader for rendering an isosurface
#version 110

uniform bool enable_lighting;

vec4 compute_lighting();

void main()
{
  vec4 color = gl_Color;
  if ( enable_lighting )
  {
    color.rgb = ( color * compute_lighting() ).rgb;
  }

  gl_FragColor = color;
}
