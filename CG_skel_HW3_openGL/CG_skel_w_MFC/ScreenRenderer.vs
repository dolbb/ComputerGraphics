#version 330 core

in vec3 vPosition;

out vec2 texCoords

void main()
{
	gl_Position = vPosition;
	texCoords = vPosition.xy;
}