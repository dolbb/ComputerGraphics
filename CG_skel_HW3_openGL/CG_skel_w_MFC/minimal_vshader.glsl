#version 150


in  vec3 vPosition;

out vec4 c;

void main()
{
    gl_Position = vec4(vPosition,1.0);
	c = vec4(1.0,0.0,0.0,1.0);
}