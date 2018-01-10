#version 150


in  vec4 vPosition;
in  vec3 c;

out vec4 color;

void main()
{
    gl_Position = vPosition;
	color = vec4(c,1.0);
}