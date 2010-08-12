// GLSL fragment shader for headlight shading

varying vec4 diffuse, ambient_global, ambient; // Light components.
varying vec3 light_dir; // Light direction
varying vec3 half_vector; // Half vector between eye and light source. 
                                       // In the case of head light, it's the same as eye vector.
varying float dist; // Distance to light source.
varying vec3 normal; // The normal vector.

vec4 compute_lighting()
{
  vec3 n, half_v;
  float n_dot_l, n_dot_hv;
  vec4 color = ambient_global;
  
  n = normalize( normal );
  if ( !gl_FrontFacing )
  {
    n = -n;
  }
  
  n_dot_l = max( dot ( n, normalize( light_dir ) ), 0.0 );

  if ( n_dot_l > 0.0 ) 
  {
    color += ( diffuse * n_dot_l + ambient );
    
    half_v = normalize(half_vector);
    n_dot_hv = max( dot ( n, half_v ), 0.0 );
    color += gl_FrontMaterial.specular * gl_LightSource[0].specular * 
            pow( n_dot_hv, gl_FrontMaterial.shininess );
  }

  return color;
}
