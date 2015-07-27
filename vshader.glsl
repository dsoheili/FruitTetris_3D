#version 130

in vec4 vPosition, vColor;
out vec4 color;

uniform int xsize, ysize;
uniform mat4 vMVP;

void main()
{
	gl_Position = vMVP * vPosition;
	color = vColor;
} 