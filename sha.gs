#version 150

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform int mode; // regular=0, hilight=1 
uniform sampler2D highlightTex;
uniform int highlightTexStridePow;


in vec3 color[];

out vec3 fgcolor;

void main()
{
  if(mode == 1 && texelFetch(highlightTex, ivec2(gl_PrimitiveIDIn & ((1 << highlightTexStridePow) - 1), gl_PrimitiveIDIn >> highlightTexStridePow), 0).x == .0)
    return;
  
  for(int i=0;i<3;i++) {
    fgcolor = color[i];
    gl_Position = gl_in[i].gl_Position;
    EmitVertex();
  }

  EndPrimitive();
}
