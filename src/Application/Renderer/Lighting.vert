// GLSL vertex shader for headlight shading

varying vec4 diffuse, ambient_global, ambient; // Light components.
varying vec3 light_dir; // Light direction
varying vec3 half_vector; // Half vector between eye and light source. 
                                       // In the case of head light, it's the same as eye vector.
varying float dist; // Distance to light source.
varying vec3 normal; // The normal vector.
  
void compute_lighting()
{ 
  vec4 eye_coord_pos; // The position of the vertex in eye space
  vec3 aux;
  
  normal = normalize( gl_NormalMatrix * gl_Normal );
  
  eye_coord_pos = gl_ModelViewMatrix * gl_Vertex;

  // With headlight, light position is alway at the origin in eye space
  aux = vec3( -eye_coord_pos );
  light_dir = normalize( aux );
  dist = length(aux);

  half_vector = light_dir;
  
  diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;   
  ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
  ambient_global = gl_LightModel.ambient * gl_FrontMaterial.ambient;
} 
