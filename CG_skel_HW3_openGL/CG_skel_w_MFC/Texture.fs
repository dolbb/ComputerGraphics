#version 400
  
in vec3 fragNormal;
in vec4 fragPos;
in vec3 rawPosVal;
in vec2 TexCoord;

struct Material
{
	vec3 emissive;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct DirectionalLight
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float constant;
	float linear;
	float quadratic;
};

#define MAX_NUM_OF_LIGHTS 4

uniform int activePointLights;
uniform int activeDirectionalLights;
uniform vec3 eye;
uniform Material material;
uniform DirectionalLight directionalLights[MAX_NUM_OF_LIGHTS];
uniform PointLight pointLights[MAX_NUM_OF_LIGHTS];
uniform bool isUniformFlag;
uniform bool fogFlag;
uniform bool fToonFlag;
uniform sampler2D ourTexture;

out vec4 fragColor;

vec3 calculateDirectionalLight(DirectionalLight directionalLight, vec3 normal, vec3 viewDirection, Material material);
vec3 calculatePointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDirection, Material material);

void main()
{
	Material innerMaterial = material;
	vec3 normal = normalize(fragNormal);
	vec3 localFragPos = vec3(fragPos);
	vec3 viewDirection = normalize(eye - localFragPos);
	vec3 outColor = material.emissive;
	int i=0;
	vec3 tmpV;
	if(isUniformFlag != true){
		if(rawPosVal.x > 0){
			if(rawPosVal.y > 0){
				tmpV = vec3(0,0,1);
			}else{
				tmpV = vec3(1,0,1);
			}
		}else{
			if(rawPosVal.y > 0){
				tmpV = vec3(1,1,0);
			}else{
				tmpV = vec3(1,0,0);
			}
		}
		innerMaterial.emissive 	= tmpV * 0.05;
		innerMaterial.ambient 	= tmpV * 0.2;
		innerMaterial.diffuse 	= tmpV * 0.5;
		innerMaterial.specular 	= tmpV * 0.7;
	}
	for(; i<activeDirectionalLights; i++)
	{
		outColor += calculateDirectionalLight(directionalLights[i], normal, viewDirection, innerMaterial);
	}
	i=0;
	for(; i<activePointLights; i++)
	{
		outColor += calculatePointLight(pointLights[i], normal, viewDirection,localFragPos, innerMaterial);
	}
	if(fogFlag)
	{
		float dist = gl_FragCoord.z;
		float visibility = (1 - dist)/2;
		fragColor = mix(vec4(0.0),vec4(outColor,1.0),visibility);
		fragColor = clamp(fragColor, 0.0, 1.0);	
	}
	else
	{
		fragColor = vec4(outColor,1.0);
		fragColor = clamp(fragColor, 0.0, 1.0);	
	}
	fragColor *= texture(ourTexture, TexCoord);
}

vec3 calculateDirectionalLight(DirectionalLight directionalLight, vec3 normal, vec3 viewDirection, Material innerMaterial)
{

	vec3 nLightDirection = normalize(-directionalLight.direction);
	

	//ambient
	vec3 ambient = innerMaterial.ambient * directionalLight.ambient;

	//diffuse
	float diffuseFactor = max( dot(normal , nLightDirection) , 0.0 );
	if(fToonFlag){
		float diffLevel = floor(diffuseFactor * 3);
		diffuseFactor = diffLevel / 3;
	}
	vec3 diffuse = directionalLight.diffuse * (diffuseFactor * innerMaterial.diffuse);

	//specular
	vec3 nReflectDirection = reflect( -nLightDirection, normal );
	float specFactor = pow( max( dot( viewDirection,nReflectDirection ) , 0.0 ) , innerMaterial.shininess);
	vec3 specular = directionalLight.specular * (specFactor * innerMaterial.specular);

	return ambient + diffuse + specular;
}

vec3 calculatePointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDirection, Material innerMaterial)
{
	//light direction
	vec3 nLightDirection = normalize(pointLight.position - fragPos);

	//light attenuation
	float d = length(pointLight.position - fragPos);
	float attenuation = 1.0; // (pointLight.constant + pointLight.linear * d + pointLight.quadratic * d * d);

	//ambient
	vec3 ambient=pointLight.ambient*(innerMaterial.ambient*attenuation);

	//diffuse
	float diffuseFactor = max(dot(normal,nLightDirection),0.0);
		if(fToonFlag){
		float diffLevel = floor(diffuseFactor * 3);
		diffuseFactor = diffLevel / 3;
	}
	vec3 diffuse = (pointLight.diffuse*(diffuseFactor*innerMaterial.diffuse))*attenuation;

	//specular
	vec3 nReflectDirection = reflect(-nLightDirection, normal);
	float specFactor = pow( max( dot( viewDirection,nReflectDirection ),0.0 ),innerMaterial.shininess );
	vec3 specular=(pointLight.specular*(specFactor*innerMaterial.specular))*attenuation;

	return ambient + diffuse + specular;
}