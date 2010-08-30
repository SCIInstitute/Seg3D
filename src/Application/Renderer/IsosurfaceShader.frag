// GLSL fragment shader for rendering an isosurface
#version 110

uniform bool enable_lighting;
uniform bool use_colormap;
uniform sampler1D colormap;
varying float normalized_value;

vec4 compute_lighting();

void main()
{
  vec4 color;
  if ( use_colormap )
  {
    color = texture1D( colormap, normalized_value );
  }
  else
  {
    color = gl_Color;
  }

  if ( enable_lighting )
  {
    color.rgb = ( color * compute_lighting() ).rgb;
  }

  gl_FragColor = color;
}
