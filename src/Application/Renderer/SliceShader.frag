// GLSL fragment shader for rendering a slice

uniform sampler2D slice_tex;
uniform sampler3D pattern_tex;
uniform bool mask_mode;
uniform float opacity;
uniform vec2 scale_bias;
uniform ivec2 size;
uniform int color_index;

vec4 shade_data_slice()
{
  float value = texture2D( slice_tex, gl_TexCoord[0].st ).r;
  value = value * scale_bias[0] + scale_bias[1];
  return vec4( vec3( value ), opacity );
}

vec4 shade_mask_slice()
{
  float mask = texture2D( slice_tex, gl_TexCoord[0].st ).a;
  if ( mask == 0.0 ) discard;
  float pattern = texture3D( pattern_tex, gl_TexCoord[1].stp ).a;
  return vec4( 1.0, 0.6, 0.0, pattern * opacity );
}

void main()
{
  // Discard the fragment if it's completely transparent
  if ( opacity == 0.0 ) discard;

  if ( mask_mode )
  {
    gl_FragColor = shade_mask_slice();
  }
  else
  {
    gl_FragColor = shade_data_slice();
  }
}
