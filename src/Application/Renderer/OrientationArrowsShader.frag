// GLSL fragment shader for rendering the orientation arrows

vec4 compute_lighting();

void main()
{
  gl_FragColor = gl_Color * compute_lighting();
}
