#version 150


in  vec4 vPosition;
in  vec3 vColor;

out vec4 c;

void main()
{
    gl_Position = vPosition;
	c = vec4(vColor,1.0);
}