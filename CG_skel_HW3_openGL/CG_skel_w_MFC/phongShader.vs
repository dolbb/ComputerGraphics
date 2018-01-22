#version 400

in vec3 vPosition;
in vec3 vNormal;

uniform mat3 normalTransform;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragNormal;
out vec3 fragPos;

void main()
{
	gl_Position = projection * view * model * vec4(vPosition,1.0);
	vec4 tmp  = (model * vec4(vPosition,1.0));
	if(tmp.w != 0){
		fragPos = vec3(tmp.x/tmp.w, tmp.y/tmp.w, tmp.z/tmp.w);	
	}
	else{
		fragPos = vec3(tmp.x, tmp.y, tmp.z); 	
	}
	fragNormal = normalTransform * vNormal;
}