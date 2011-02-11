// GLSL fragment shader for rendering a slice

uniform sampler2D tex;
uniform vec3 color; // color of mask
uniform float opacity;
uniform int border_width; // width of the mask border
uniform vec2 pixel_size; // pixel size in texture space

// Test for mask edges
bool edge_test()
{
  vec2 offset = pixel_size * float( border_width );
  float left = gl_TexCoord[0].s - offset[0];
  float right = gl_TexCoord[0].s + offset[0];
  float bottom = gl_TexCoord[0].t - offset[1];
  float top = gl_TexCoord[0].t + offset[1];
  
  // test for texture boundary
  if ( left < 0.0 || right > 1.0 || bottom < 0.0 || top > 1.0 )
    return true;
  
  // test the pixel to the left
  if ( texture2D( tex, vec2( left,  gl_TexCoord[0].t ) ).a == 0.0 )
    return true;

  // test the pixel to the right
  if ( texture2D( tex, vec2( right,  gl_TexCoord[0].t ) ).a == 0.0 )
    return true;

  // test the pixel below
  if ( texture2D( tex, vec2( gl_TexCoord[0].s, bottom ) ).a == 0.0 )
    return true;

  // test the pixel above
  if ( texture2D( tex, vec2( gl_TexCoord[0].s, top ) ).a == 0.0 )
    return true;

  // test the pixel on the bottom left corner
  if ( texture2D( tex, vec2( left, bottom ) ).a == 0.0 )
    return true;

  // test the pixel on the bottom right corner
  if ( texture2D( tex, vec2( right, bottom ) ).a == 0.0 )
    return true;

  // test the pixel on the top right corner
  if ( texture2D( tex, vec2( right, top ) ).a == 0.0 )
    return true;

  // test the pixel on the top left corner
  if ( texture2D( tex, vec2( left, top ) ).a == 0.0 )
    return true;

  return false;
}

vec4 shade_mask()
{
  float mask = texture2D( tex, gl_TexCoord[0].st ).a;
  if ( mask == 0.0 ) discard;

  if ( border_width > 0 )
  {
    if ( edge_test() )
      return vec4( color, opacity );
    else
      discard;
  }
  else
  {
    return vec4( color, opacity );
  }
}

void main()
{
  // Discard the fragment if it's completely transparent
  if ( opacity == 0.0 ) discard;

  gl_FragColor = shade_mask();
}
