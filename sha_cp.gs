#version 150

layout(points) in;
layout(line_strip, max_vertices = 64) out;

uniform int mode; // regular=0, hilight=1 
uniform sampler2D highlightTex;
uniform int highlightTexStridePow;

in vec3 vColor[];
in float vSides[];
in vec2 radii[];
out vec3 fgcolor;

const float PI = 3.1415926;

void main()
{
  if(mode == 1 && texelFetch(highlightTex, ivec2(gl_PrimitiveIDIn & ((1 << highlightTexStridePow) - 1), gl_PrimitiveIDIn >> highlightTexStridePow), 0).x == .0)
    return;

  fgcolor = vColor[0];

  for (int i = 0; i <= vSides[0]; i++) {
    float ang = PI * 2.0 / (vSides[0] - 1) * i;

    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    
    vec4 offset = vec4(cos(ang) * radii[0].x, -sin(ang) * radii[0].y, 0.0, 0.0);
    gl_Position = gl_in[0].gl_Position + offset;
    EmitVertex();
  }

  EndPrimitive();
}
