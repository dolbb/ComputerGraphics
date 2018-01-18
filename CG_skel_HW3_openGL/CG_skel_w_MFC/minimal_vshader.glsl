#version 150

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in  vec3 vPosition;

void main()
{
    gl_Position = projection * view * model * vec4(vPosition.x,vPosition.y,vPosition.z,1.0);
}