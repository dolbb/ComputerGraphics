#version 150

in vec3 vPosition;
in vec3 vNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalTransform;
uniform bool vToonFlag;
uniform float toonFactor;

out vec4 vColor;

void main()
{
	float tmpToonFactor = toonFactor;
	vec4 tmpColor = vec4(1.0,0.0,0.0,1.0); 
	vec3 n =  normalTransform * vNormal;
	vec4 pos = vec4(vPosition.x,vPosition.y,vPosition.z,1.0);
	if(vToonFlag){
		tmpColor = vec4(0.0,0.0,0.0,1.0);
		if(n.z <= 0){
			pos += vec4(tmpToonFactor * vNormal , 0.0);
		}
		else{
			pos -= vec4(tmpToonFactor * vNormal , 0.0);
		}
	}
    gl_Position = projection * view * model * pos;
    vColor = tmpColor;
}