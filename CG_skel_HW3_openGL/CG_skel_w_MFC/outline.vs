
#version 330
 
in vec3 vPosition
in vec3 vNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalTransform;

const float nudge = 0.5;

void main()
{
	vec3 worldPos = model * vPosition;
	vec3 worldNormal = normalize(normalTransform * vNormal);
	gl_Position = projection * view * vec4(worldPos + worldNormal * nudge);
}