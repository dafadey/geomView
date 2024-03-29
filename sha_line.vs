#version 150

uniform mat4 view_matrix, proj_matrix;
uniform vec2 aspect;


in vec3 vertex_pos;
in vec3 line_color;

out vec3 color;

void main()
{
    color = line_color;
    vec4 pos = vec4(vertex_pos.x, vertex_pos.y, vertex_pos.z, 1);
    gl_Position = proj_matrix * (view_matrix * pos);
}
