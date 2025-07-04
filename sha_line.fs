#version 150
 
in vec3 fgcolor;
out vec4 outputF;

uniform int mode;

void main()
{
  if(mode == 0)
    outputF = vec4(fgcolor, 1.0);
  if(mode == 1)
    outputF = vec4(0, 0, 0, 1.0);
}
