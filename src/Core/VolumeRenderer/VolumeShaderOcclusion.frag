// GLSL fragment shader for rendering a slice

uniform sampler3D vol_tex;
uniform sampler1D diffuse_lut;
uniform sampler1D specular_lut;
uniform sampler1D occlusion_sample_lut;
uniform sampler2D occlusion_buffer;

uniform vec3 texel_size; // texel size in texture space
uniform vec3 voxel_size; // voxel size in world space

// Number of samples contained in the occlusion_sample_lut
uniform int num_of_occlusion_samples;
// Occlusion extent (radius of sampling disk) in world space
uniform float occlusion_extent;
// Slice distance normalized by volume unit length
uniform float normalized_slice_distance; 

varying vec2 correction_factor;
varying vec4 clip_space_pos;

// Vertex postion in world coordinates
varying vec4 world_coord_pos;
// Clipping planes in world coordinates
uniform vec4 clip_plane[ 6 ];
uniform bool enable_clip_plane[ 6 ];
uniform bool enable_clipping;

// Half vector between eye and light source.
varying vec3 half_vector; 
// The light direction is always in +Z in eye space
const vec3 light_dir = vec3( 0.0, 0.0, 1.0 ); 

vec4 diffuse_color, secondary_color;

vec4 compute_lighting( vec3 normal )
{
  vec3 half_v;
  float n_dot_l, n_dot_hv;
  vec4 material_ambient = diffuse_color * secondary_color[0];
  vec4 material_specular = vec4( secondary_color[1] );
  vec4 color = gl_LightModel.ambient * material_ambient;
  
  n_dot_l = dot ( normal, light_dir );
  if ( n_dot_l < 0.0 )
  {
    n_dot_l = -n_dot_l;
    normal = -normal;
  }

  if ( n_dot_l > 0.0 ) 
  {
    color += ( gl_LightSource[0].diffuse * diffuse_color * n_dot_l +
      gl_LightSource[0].ambient * material_ambient );
    
    half_v = normalize( half_vector );
    n_dot_hv = max( dot ( normal, half_v ), 0.0 );
    color += material_specular * gl_LightSource[0].specular * 
            pow( n_dot_hv, secondary_color[2] * 255.0 );
  }

  return color;
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
//  if ( diffuse_color.a == 0.0 ) discard;
  vec2 device_space_pos = clip_space_pos.xy / clip_space_pos.w;

  float transparency, alpha;
  if ( diffuse_color.a == 0.0 )
  {
    transparency = 1.0;
    alpha = 0.0;
    gl_FragData[ 0 ] = vec4( 0.0 );
  }
  else
  {
    transparency = pow( 1.0 - diffuse_color.a, normalized_slice_distance );
    //float transparency = exp( -diffuse_color.a * normalized_slice_distance );
    alpha = 1.0 - transparency;
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
    vec3 voxel_color = compute_lighting( gl_NormalMatrix * gradient ).rgb;

    // Render to the eye buffer
    vec2 tex_space_pos = device_space_pos * 0.5 + vec2( 0.5 );
    float occlusion = texture2D( occlusion_buffer, tex_space_pos ).r;
    gl_FragData[ 0 ] = vec4( voxel_color * occlusion * alpha, alpha );
  }

  // Render to the next occlusion buffer
  float occlusion = 0.0;
  for ( int i = 0; i < num_of_occlusion_samples; ++i )
  {
    vec2 sample_pos = texture1D( occlusion_sample_lut, 
      ( float( i ) + 0.5 ) / float( num_of_occlusion_samples ) ).xy;
    sample_pos *= occlusion_extent;
    sample_pos = device_space_pos + correction_factor * sample_pos;
    sample_pos = sample_pos * 0.5 + vec2( 0.5 );
    occlusion += texture2D( occlusion_buffer, sample_pos ).r;
  }
  occlusion = occlusion / float( num_of_occlusion_samples ) * transparency;
  gl_FragData[ 1 ] = vec4( occlusion );
}
