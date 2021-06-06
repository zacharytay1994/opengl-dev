#version 450 core

layout (location=0) out vec4 fFragColor;

uniform vec3 uColor;

void main () 
{
	fFragColor=vec4(uColor, 1.0);
}