// GLSL vertex shader for rendering an isosurface

uniform bool enable_lighting;
uniform bool enable_fog;
uniform bool use_colormap;
uniform float min_val;
uniform float val_range;
attribute float value;
varying float normalized_value;

void compute_lighting();
void compute_fog_depth();

void main()
{
  if ( enable_lighting )
  {
    compute_lighting();
  }

  if ( enable_fog )
  {
    compute_fog_depth();
  }

  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_TexCoord[1] = gl_MultiTexCoord1;
  gl_FrontColor = gl_Color;
  gl_BackColor = gl_Color;
  if ( use_colormap )
  {
    normalized_value = ( value - min_val ) / val_range;
  }
#ifndef DISABLE_CLIPPING
  gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
#endif
  gl_Position = ftransform();
} 
