#version 330

in vec3 vPosition;
in vec3 vNormal;

uniform mat3 normalTransform;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragNormal;
out vec3 fragPos;
out float visibility;

const float density=0.007;
const float gradient=1.5;

void main()
{
	vec4 positionRelitiveToCamera = view * model * vec4(vPosition,1.0);
	float distance = length(positionRelitiveToCamera.xyz);
	visibility = exp(-pow(distance*density,gradient));
	visibility = clamp(visibility, 0.0, 1.0);
	gl_Position = projection * view * model * vec4(vPosition,1.0);
	fragPos = vec3(model * vec4(vPosition,1.0));
	fragNormal = normalTransform * vNormal;
}