// GLSL fragment shader for rendering a slice
#version 120

uniform sampler2D slice_tex;
uniform sampler2D pattern_tex;

// Mask drawing mode
// 0: border only
// 1: border + pattern
// 2: fill
uniform int mask_mode;

// Volume type
// 1: data volume
// 2: mask volume
uniform int volume_type; 

uniform vec3 mask_color;  // color of mask
uniform float opacity;
uniform vec2 scale_bias;
uniform int border_width; // width of the mask border
uniform vec2 pixel_size; // pixel size in texture space

varying vec4 diffuse, ambient_global, ambient; // Light components.
varying vec3 light_dir; // Light direction
varying vec3 half_vector; // Half vector between eye and light source. 
                                       // In the case of head light, it's the same as eye vector.
varying float dist; // Distance to light source.
varying vec3 normal; // The normal vector. It's the same all over the slice.
uniform bool enable_lighting;

vec4 shade_data_slice()
{
  float value = texture2D( slice_tex, gl_TexCoord[0].st ).r;
  value = value * scale_bias[0] + scale_bias[1];
  return vec4( vec3( value ), opacity );
}

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
  if ( texture2D( slice_tex, vec2( left,  gl_TexCoord[0].t ) ).a == 0.0 )
    return true;

  // test the pixel to the right
  if ( texture2D( slice_tex, vec2( right,  gl_TexCoord[0].t ) ).a == 0.0 )
    return true;

  // test the pixel below
  if ( texture2D( slice_tex, vec2( gl_TexCoord[0].s, bottom ) ).a == 0.0 )
    return true;

  // test the pixel above
  if ( texture2D( slice_tex, vec2( gl_TexCoord[0].s, top ) ).a == 0.0 )
    return true;

  // test the pixel on the bottom left corner
  if ( texture2D( slice_tex, vec2( left, bottom ) ).a == 0.0 )
    return true;

  // test the pixel on the bottom right corner
  if ( texture2D( slice_tex, vec2( right, bottom ) ).a == 0.0 )
    return true;

  // test the pixel on the top right corner
  if ( texture2D( slice_tex, vec2( right, top ) ).a == 0.0 )
    return true;

  // test the pixel on the top left corner
  if ( texture2D( slice_tex, vec2( left, top ) ).a == 0.0 )
    return true;

  return false;
}

vec4 shade_mask_slice()
{
  float mask = texture2D( slice_tex, gl_TexCoord[0].st ).a;
  if ( mask == 0.0 ) discard;

  if ( mask_mode == 1 ) // border + pattern
  {
    if ( border_width > 0 )
    {
      if ( edge_test() )
        return vec4( mask_color, opacity );
    }
    
    float pattern = texture2D( pattern_tex, gl_TexCoord[1].st ).a;
    return vec4( mask_color, pattern * opacity );
  }
  else if ( mask_mode == 0 ) // border only
  {
    if ( border_width > 0 )
    {
      if ( edge_test() )
        return vec4( mask_color, opacity );
    }
    discard;
  }
  else // fill
  {
    return vec4( mask_color, opacity );
  }
}

vec4 compute_lighting()
{
  vec3 n, half_v;
  float n_dot_l, n_dot_hv;
  vec4 color = ambient_global;
  
  n = normalize(normal);
  
  n_dot_l = abs( dot ( n, normalize( light_dir ) ) );

  if ( n_dot_l > 0.0 ) 
  {
    color += ( diffuse * n_dot_l + ambient );
    
    half_v = normalize(half_vector);
    n_dot_hv = abs( dot ( n, half_v ) );
    color += gl_FrontMaterial.specular * gl_LightSource[0].specular * 
            pow( n_dot_hv, gl_FrontMaterial.shininess );
  }

  return color;
}

void main()
{
  // Discard the fragment if it's completely transparent
  if ( opacity == 0.0 ) discard;

  // Discard if the slice texture is out of boundary
  if ( gl_TexCoord[0].s < 0.0 ||
    gl_TexCoord[0].s > 1.0 ||
    gl_TexCoord[0].t < 0.0 ||
    gl_TexCoord[0].t > 1.0 )
    discard;

  vec4 slice_color;
  if ( volume_type == 2 )
  {
    slice_color = shade_mask_slice();
  }
  else
  {
    slice_color = shade_data_slice();
  }

  if ( enable_lighting )
  {
    slice_color.rgb = ( slice_color * compute_lighting() ).rgb;
  }

  gl_FragColor = slice_color;
}
