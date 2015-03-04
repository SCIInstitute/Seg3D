// GLSL fragment shader for rendering an isosurface

uniform bool enable_lighting;
uniform bool use_colormap;
uniform sampler1D colormap;
varying float normalized_value;
uniform bool enable_fog;
uniform float opacity;

vec4 compute_lighting();
float compute_fog_factor();

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

  color.a = opacity;

  if ( enable_lighting )
  {
    color.rgb = ( color * compute_lighting() ).rgb;
  }

  if ( enable_fog )
  {
    color = mix( gl_Fog.color, color, compute_fog_factor() );
  }

  gl_FragColor = color;
}
