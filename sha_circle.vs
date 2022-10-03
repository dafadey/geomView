#version 150

uniform mat4 view_matrix, proj_matrix;
uniform vec2 aspect;

in vec3 point_pos;
in float radius;
in vec3 point_color;
//in float circle_resolution;

out vec3 vColor;
out float vSides; 
out vec2 radii; 

void main()
{
    vColor = point_color;
    vSides = 8;//circle_resolution;
    radii = vec2(radius*aspect.x, radius*aspect.y);
    vec4 pos = vec4(point_pos.x, point_pos.y, point_pos.z, 1);
    gl_Position = proj_matrix * (view_matrix * pos);
}
