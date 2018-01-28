#version 400

in vec3 vPosition;
in vec3 vNormal;
in vec2 textureCoords;

uniform mat3 normalTransform;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragNormal;
out vec4 fragPos;
out vec3 rawPosVal;
out vec2 TexCoord;

void main()
{
    //gl_Position = vec4(vPosition,1.0);
    gl_Position = projection * view * model * vec4(vPosition,1.0);
	fragNormal = normalTransform * vNormal;
	fragPos  = (model * vec4(vPosition,1.0));
	rawPosVal = vPosition;
    TexCoord = textureCoords;
}