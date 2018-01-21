#version 330 core

out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube environment;

void main()
{    
    FragColor = texture(environment, TexCoords);
}