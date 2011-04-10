// GLSL fragment shader for rendering a slice

uniform sampler3D vol_tex;
uniform sampler1D diffuse_lut;
uniform sampler1D specular_lut;

uniform vec3 texel_size; // texel size in texture space
uniform vec3 voxel_size; // voxel size in world space
uniform float slice_distance; // Slice distance normalized by volume unit length

uniform bool enable_lighting;
uniform bool enable_fog;

varying vec3 light_dir; // Light direction
varying vec3 half_vector; // Half vector between eye and light source. 
                                       // In the case of head light, it's the same as eye vector.
varying float fog_depth;

// Fragment position in world coordinates
varying vec4 world_coord_pos;

// Clipping planes in world coordinates
uniform vec4 clip_plane[ 6 ];
uniform bool enable_clip_plane[ 6 ];
uniform bool enable_clipping;

vec4 diffuse_color, secondary_color;

vec4 compute_lighting( vec3 normal )
{
  vec3 half_v;
  float n_dot_l, n_dot_hv;
  vec4 material_ambient = diffuse_color * secondary_color[0];
  vec4 material_specular = vec4( secondary_color[1] );
  vec4 color = gl_LightModel.ambient * material_ambient;
  
  n_dot_l = dot ( normal, normalize( light_dir ) );
  if ( n_dot_l < 0.0 )
  {
    n_dot_l = -n_dot_l;
    normal = -normal;
  }

  if ( n_dot_l > 0.0 ) 
  {
    color += ( gl_LightSource[0].diffuse * diffuse_color * n_dot_l +
      gl_LightSource[0].ambient * material_ambient );
    
    half_v = normalize(half_vector);
    n_dot_hv = max( dot ( normal, half_v ), 0.0 );
    color += material_specular * gl_LightSource[0].specular * 
            pow( n_dot_hv, secondary_color[2] * 255.0 );
  }

  return color;
}

float compute_fog_factor()
{
  float z = fog_depth;
  float fog_factor = exp( -gl_Fog.density * gl_Fog.density * z * z );
  fog_factor = clamp( fog_factor, 0.0, 1.0 );
  
  return fog_factor;
}

float volume_lookup( vec3 tex_coord )
{
  float val = texture3D( vol_tex, tex_coord ).a;
  return val;
}

bool clipping_test( in vec4 pos, out vec3 clip_norm )
{
  bool result = true;

  if ( enable_clip_plane[ 0 ] && dot( clip_plane[ 0 ], pos ) < 0.0 )
  {
    result = false;
    clip_norm = clip_plane[ 0 ].xyz;
  }
  else if ( enable_clip_plane[ 1 ] && dot( clip_plane[ 1 ], pos ) < 0.0 )
  {
    result = false;
    clip_norm = clip_plane[ 1 ].xyz;
  }
  else if ( enable_clip_plane[ 2 ] && dot( clip_plane[ 2 ], pos ) < 0.0 )
  {
    result = false;
    clip_norm = clip_plane[ 2 ].xyz;
  }
  else if ( enable_clip_plane[ 3 ] && dot( clip_plane[ 3 ], pos ) < 0.0 )
  {
    result = false;
    clip_norm = clip_plane[ 3 ].xyz;
  }
  else if ( enable_clip_plane[ 4 ] && dot( clip_plane[ 4 ], pos ) < 0.0 )
  {
    result = false;
    clip_norm = clip_plane[ 4 ].xyz;
  }
  else if ( enable_clip_plane[ 5 ] && dot( clip_plane[ 5 ], pos ) < 0.0 )
  {
    result = false;
    clip_norm = clip_plane[ 5 ].xyz;
  }

  return result;
}

void main()
{
  float voxel_val = volume_lookup( gl_TexCoord[0].stp );
  diffuse_color = texture1D( diffuse_lut, voxel_val );
  if ( diffuse_color.a == 0.0 ) discard;
  float alpha = 1.0 - pow( 1.0 - diffuse_color.a, slice_distance );
  vec4 voxel_color;

  if ( enable_lighting )
  {
    secondary_color = texture1D( specular_lut, voxel_val );
    vec3 gradient;

    vec3 clip_norm;
    if ( enable_clipping && ( !clipping_test( world_coord_pos + vec4( voxel_size.x, 0.0, 0.0, 0.0 ), clip_norm ) ||
      !clipping_test( world_coord_pos - vec4( voxel_size.x, 0.0, 0.0, 0.0 ), clip_norm ) ||
      !clipping_test( world_coord_pos + vec4( 0.0, voxel_size.y, 0.0, 0.0 ), clip_norm ) ||
      !clipping_test( world_coord_pos - vec4( 0.0, voxel_size.y, 0.0, 0.0 ), clip_norm ) ||
      !clipping_test( world_coord_pos + vec4( 0.0, 0.0, voxel_size.z, 0.0 ), clip_norm ) ||
      !clipping_test( world_coord_pos - vec4( 0.0, 0.0, voxel_size.z, 0.0 ), clip_norm ) ) )
    {
      gradient = -clip_norm;
    }
    else
    {
      gradient.x = ( volume_lookup( gl_TexCoord[0].stp + vec3( texel_size.x, 0.0, 0.0 ) ) -
        volume_lookup( gl_TexCoord[0].stp - vec3( texel_size.x, 0.0, 0.0 ) ) ) / ( 2.0 * voxel_size.x );      
      gradient.y = ( volume_lookup( gl_TexCoord[0].stp + vec3( 0.0, texel_size.y, 0.0 ) ) -
        volume_lookup( gl_TexCoord[0].stp - vec3( 0.0, texel_size.y, 0.0 ) ) ) / ( 2.0 * voxel_size.y );
      gradient.z = ( volume_lookup( gl_TexCoord[0].stp + vec3( 0.0, 0.0, texel_size.z ) ) -
        volume_lookup( gl_TexCoord[0].stp - vec3( 0.0, 0.0, texel_size.z ) ) ) / ( 2.0 * voxel_size.z );
      gradient = normalize( gradient );
    }
    voxel_color = compute_lighting( gl_NormalMatrix * gradient );
  }
  else
  {
    voxel_color = diffuse_color;
  }

  if ( enable_fog )
  {
    voxel_color = mix( gl_Fog.color, voxel_color, compute_fog_factor() );
  }

  voxel_color.a = alpha;
  gl_FragColor = voxel_color;
}
