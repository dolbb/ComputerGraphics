#version 150


in  vec3 vPosition;

void main()
{
    gl_Position = vec4(vPosition.x,vPosition.y,vPosition.z,1.0);
}