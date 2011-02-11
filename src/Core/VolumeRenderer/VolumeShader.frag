// GLSL fragment shader for rendering a slice

uniform sampler3D vol_tex;

uniform vec3 texel_size; // texel size in texture space
uniform vec3 voxel_size; // voxel size in world space
uniform vec2 scale_bias;
uniform float sample_rate; // Number of samples per voxel

uniform bool enable_lighting;
uniform bool enable_fog;

varying vec4 diffuse, ambient_global, ambient; // Light components.
varying vec3 light_dir; // Light direction
varying vec3 half_vector; // Half vector between eye and light source. 
                                       // In the case of head light, it's the same as eye vector.
varying float dist; // Distance to light source.

float compute_fog_factor();

vec4 compute_lighting( vec3 normal )
{
  vec3 half_v;
  float n_dot_l, n_dot_hv;
  vec4 color = ambient_global;
  
  n_dot_l = dot ( normal, normalize( light_dir ) );
  if ( n_dot_l < 0.0 )
  {
    n_dot_l = -n_dot_l;
    normal = -normal;
  }

  if ( n_dot_l > 0.0 ) 
  {
    color += ( diffuse * n_dot_l + ambient );
    
    half_v = normalize(half_vector);
    n_dot_hv = max( dot ( normal, half_v ), 0.0 );
    color += gl_FrontMaterial.specular * gl_LightSource[0].specular * 
            pow( n_dot_hv, gl_FrontMaterial.shininess );
  }

  return color;
}

float texture_lookup( vec3 tex_coord )
{
  float val = texture3D( vol_tex, tex_coord ).a;
  val = clamp( val * scale_bias[0] + scale_bias[1], 0.0, 1.0 );
  return val;
}

void main()
{
  float voxel_val = texture_lookup( gl_TexCoord[0].stp );
  vec4 voxel_color = vec4( voxel_val, voxel_val, voxel_val, 
    1.0 - pow( 1.0- voxel_val, 1.0 / sample_rate ) );

  if ( enable_lighting )
  {
    vec3 gradient;
    gradient.x = ( texture_lookup( gl_TexCoord[0].stp + vec3( texel_size.x, 0.0, 0.0 ) ) -
      texture_lookup( gl_TexCoord[0].stp - vec3( texel_size.x, 0.0, 0.0 ) ) ) / ( 2.0 * voxel_size.x );
    gradient.y = ( texture_lookup( gl_TexCoord[0].stp + vec3( 0.0, texel_size.y, 0.0 ) ) -
      texture_lookup( gl_TexCoord[0].stp - vec3( 0.0, texel_size.y, 0.0 ) ) ) / ( 2.0 * voxel_size.y );
    gradient.z = ( texture_lookup( gl_TexCoord[0].stp + vec3( 0.0, 0.0, texel_size.z ) ) -
      texture_lookup( gl_TexCoord[0].stp - vec3( 0.0, 0.0, texel_size.z ) ) ) / ( 2.0 * voxel_size.z );
    gradient = normalize( gradient );
    voxel_color.rgb = ( voxel_color * compute_lighting( gradient ) ).rgb;
  }

  if ( enable_fog )
  {
    voxel_color.rgb = mix( gl_Fog.color, voxel_color, compute_fog_factor() ).rgb;
  }

  gl_FragColor = voxel_color;
}
