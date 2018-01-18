#version 150

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;

uniform mat3 normalTransform;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragPos;
out vec3 fragNormal;

void main()
{
	gl_position=projection*view*model*vec4(vPosition,1.0);
	fragPos=vec3(model*vec4(vPosition,1.0));
	fragNormal=normalTransform*vNormal;
}