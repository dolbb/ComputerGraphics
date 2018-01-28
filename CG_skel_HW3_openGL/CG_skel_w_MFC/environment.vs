#version 330 core

in vec3 vPosition;

uniform mat4 projection;
uniform mat4 view;

out vec3 TexCoords;

void main()
{
    TexCoords = vPosition;
    gl_Position =  projection * view * vec4(vPosition, 1.0);
}  