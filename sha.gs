#version 150

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform int mode; // regular=0, hilight=1 
uniform sampler1D highlightTex;

in vec3 color[];

out vec3 fgcolor;

void main()
{
  if(mode == 1 && texelFetch(highlightTex, gl_PrimitiveIDIn, 0).x == .0)
    return;
  
  for(int i=0;i<3;i++) {
    fgcolor = color[i];
    gl_Position = gl_in[i].gl_Position;
    EmitVertex();
  }

  EndPrimitive();
}
