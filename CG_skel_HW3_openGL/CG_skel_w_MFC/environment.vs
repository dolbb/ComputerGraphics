#version 330 core

in vec3 vPosition;

out vec3 TexCoords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = vPosition;
    view = mat4(mat3(view));
    gl_Position = projection * view * vec4(vPosition, 1.0);
}  