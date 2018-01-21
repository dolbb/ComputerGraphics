#version 330

in vec3 vPosition;
in vec3 vNormal;

uniform mat3 normalTransform;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragNormal;
out vec3 fragPos;
out vec3 fragPosNDC;

void main()
{
	gl_Position = projection * view * model * vec4(vPosition,1.0);
	fragPos = vec3(model * vec4(vPosition,1.0));
	fragNormal = normalTransform * vNormal;
	fragPosNDC = vec3( projection * view * model * vec4(vPosition,1.0) );
}