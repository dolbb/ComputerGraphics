#version 400

in vec3 fragNormal;
in vec4 fragPos;

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
uniform bool isUniformMat;
uniform bool fogFlag;

out vec4 fragColor;

vec3 calculateDirectionalLight(DirectionalLight directionalLight, vec3 normal, vec3 viewDirection, Material material);
vec3 calculatePointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDirection, Material material);

void main()
{
	vec3 normal = normalize(fragNormal);
	vec3 localFragPos = vec3(fragPos);
	vec3 viewDirection = normalize(eye - localFragPos);
	vec3 outColor = material.emissive;
	int i=0;

	for(; i<activeDirectionalLights; i++)
	{
		outColor += calculateDirectionalLight(directionalLights[i], normal, viewDirection, material);
	}
	i=0;
	for(; i<activePointLights; i++)
	{
		outColor += calculatePointLight(pointLights[i], normal, viewDirection,localFragPos, material);
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
}

vec3 calculateDirectionalLight(DirectionalLight directionalLight, vec3 normal, vec3 viewDirection, Material material)
{

	vec3 nLightDirection = normalize(-directionalLight.direction);
	

	//ambient
	vec3 ambient = material.ambient * directionalLight.ambient;

	//diffuse
	float diffuseFactor = max( dot(normal , nLightDirection) , 0.0 );
	vec3 diffuse = directionalLight.diffuse * (diffuseFactor * material.diffuse);

	//specular
	vec3 nReflectDirection = reflect( -nLightDirection, normal );
	float specFactor = pow( max( dot( viewDirection,nReflectDirection ) , 0.0 ) , material.shininess);
	vec3 specular = directionalLight.specular * (specFactor * material.specular);

	return ambient + diffuse + specular;
}

vec3 calculatePointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDirection, Material material)
{
	//light direction
	vec3 nLightDirection = normalize(pointLight.position - fragPos);

	//light attenuation
	float d = length(pointLight.position - fragPos);
	float attenuation = 1.0 / (pointLight.constant + pointLight.linear * d + pointLight.quadratic * d * d);

	//ambient
	vec3 ambient=pointLight.ambient*(material.ambient*attenuation);

	//diffuse
	float diffuseFactor = max(dot(normal,nLightDirection),0.0);
	vec3 diffuse = (pointLight.diffuse*(diffuseFactor*material.diffuse))*attenuation;

	//specular
	vec3 nReflectDirection = reflect(-nLightDirection, normal);
	float specFactor = pow( max( dot( viewDirection,nReflectDirection ),0.0 ),material.shininess );
	vec3 specular=(pointLight.specular*(specFactor*material.specular))*attenuation;

	return ambient + diffuse + specular;
}