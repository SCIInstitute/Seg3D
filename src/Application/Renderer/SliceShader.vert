// GLSL vertex shader for rendering a slice
#version 110

varying vec4 diffuse, ambient_global, ambient; // Light components.
varying vec3 light_dir; // Light direction
varying vec3 half_vector; // Half vector between eye and light source. 
                                       // In the case of head light, it's the same as eye vector.
varying float dist; // Distance to light source.
varying vec3 normal; // The normal vector. It's the same all over the slice.
uniform bool enable_lighting;
  
void main()
{ 
  if ( enable_lighting )
  {
    vec4 eye_coord_pos; // The position of the vertex in eye space
    vec3 aux;
    
    // NOTE: No need to normalize since we are not scaling the slice
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
  
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_TexCoord[1] = gl_MultiTexCoord1;
  gl_Position = ftransform();
} 