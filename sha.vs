#version 150
 
uniform mat4 view_matrix, proj_matrix;
uniform vec3 light_dir;
uniform float shiny;

in vec3 vertex_pos;
in vec3 normal_pos;
in vec3 triangle_color;

out vec3 color;
 
void main()
{
  float factor = abs(dot(normal_pos, light_dir));
  float factor4 = factor * factor;
  factor4 *= factor4;
  color = triangle_color * factor * (1.f - shiny * factor4) + shiny * factor4 * factor4 * vec3(1.f, 1.f, 1.f);
  vec4 pos = vec4(vertex_pos.x, vertex_pos.y, vertex_pos.z, 1);
  gl_Position = proj_matrix * (view_matrix * pos);
}
