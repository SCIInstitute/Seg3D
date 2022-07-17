// GLSL vertex shader for slice based volume rendering

uniform vec3 tex_bbox_min; // Minimum values of texture position in world space
uniform vec3 tex_bbox_size; // Size of texture in world space

varying vec2 correction_factor;
varying vec4 clip_space_pos;

// Vertex position in world coordinates
varying vec4 world_coord_pos;

// Half vector between eye and light source.
varying vec3 half_vector; 
// The light direction is always in +Z in eye space
const vec3 light_dir = vec3( 0.0, 0.0, 1.0 ); 

void main()
{
  // The position of the vertex in eye space
  vec4 eye_coord_pos = gl_ModelViewMatrix * gl_Vertex;

  vec3 eye_vec = vec3( -eye_coord_pos );
  eye_vec = normalize( eye_vec );
  half_vector = normalize( ( eye_vec + light_dir ) * 0.5 );

  vec4 proj_scale = gl_ProjectionMatrix * vec4( 1.0, 1.0, eye_coord_pos.z, 1.0 );
  correction_factor = proj_scale.xy / proj_scale.w;

  world_coord_pos = gl_Vertex;
  gl_TexCoord[0].stp = (gl_Vertex.xyz - tex_bbox_min)/tex_bbox_size;
  gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
  clip_space_pos = ftransform();
  gl_Position = clip_space_pos;
} 
