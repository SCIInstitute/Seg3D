// GLSL fragment shader for rendering a slice

uniform sampler2D tex;
uniform bool mask_mode;
uniform float opacity;
uniform float contrast;
uniform float brightness;
uniform int color_index;

vec4 shade_data_slice()
{
  vec3 color = texture2D( tex, gl_TexCoord[0].st ).rgb * brightness;
  return vec4( color, opacity );
}

vec4 shade_mask_slice()
{
  float alpha = texture2D( tex, gl_TexCoord[0].st ).a;
  if ( alpha == 0.0 ) discard;
  return vec4( 0.0, 0.5, 0.5, alpha * opacity );
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
