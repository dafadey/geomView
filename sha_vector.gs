#version 150

layout(lines) in;
layout(line_strip, max_vertices = 6) out;

uniform int mode; // regular=0, hilight=1 
uniform sampler2D highlightTex;
uniform int highlightTexStridePow;

uniform vec2 aspect;

in vec3 vertex_pos[];
in vec3 vColor[];

out vec3 fgcolor;

const float arrowSideLength = 13.f; // pixels
const float arrowAngle = .3f; // radians

void main()
{
  if(mode == 1 && texelFetch(highlightTex, ivec2(gl_PrimitiveIDIn & ((1 << highlightTexStridePow) - 1), gl_PrimitiveIDIn >> highlightTexStridePow), 0).x == .0)
    return;

  fgcolor = vColor[0];
  gl_Position = gl_in[0].gl_Position;
  EmitVertex();

  fgcolor = vColor[1];
  gl_Position = gl_in[1].gl_Position;
  EmitVertex();

  vec2 dir = (gl_in[0].gl_Position - gl_in[1].gl_Position).xy;
  dir = vec2(dir.x/aspect.x, dir.y/aspect.y);
  float dirlen = sqrt(dir.x*dir.x+dir.y*dir.y);
  dir = arrowSideLength / dirlen * dir;
  float f = cos(arrowAngle);
  float g = sin(arrowAngle);
  vec2 dir1 = vec2(dir.x * f + dir.y * g, dir.y * f - dir.x * g);
  vec2 dir2 = vec2(dir.x * f - dir.y * g, dir.y * f + dir.x * g);

  dir1 = vec2(dir1.x*aspect.x, dir1.y*aspect.y);
  dir2 = vec2(dir2.x*aspect.x, dir2.y*aspect.y);

  gl_Position = gl_in[1].gl_Position;
  EmitVertex();
  gl_Position = gl_in[1].gl_Position + vec4(dir1, .0f, .0f);
  EmitVertex();
  gl_Position = gl_in[1].gl_Position;
  EmitVertex();
  gl_Position = gl_in[1].gl_Position + vec4(dir2, .0f, .0f);
  EmitVertex();

  EndPrimitive();
}
