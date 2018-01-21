#version 330

in vec3 vPosition;
in vec3 vNormal;

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

#define NUMBER_OF_POINT_LIGHTS 6

uniform int activePointLights;
uniform vec3 eye;
uniform mat3 normalTransform;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform Material material;
uniform DirectionalLight directionalLight;
uniform PointLight[NUMBER_OF_POINT_LIGHTS] pointLights;

out vec4 vColor;

vec3 calculateDirectionalLight(DirectionalLight directionalLight, vec3 normal, vec3 viewDirection, Material material);
vec3 calculatePointLight(PointLight pointLight, vec3 normal, vec3 vertexPosition, vec3 viewDirection, Material material);

void main()
{
	vec3 normal = normalize(normalTransform * vNormal);
	vec3 worldPos = vec3(model * vec4(vPosition,1.0));
	vec3 viewDirection = normalize(eye - worldPos);
	vec3 outColor = vec3(0.0);
	outColor += calculateDirectionalLight(directionalLight, normal, viewDirection, material);
	for(int i=0; i<activePointLights; i++)
	{
		outColor += calculatePointLight(pointLights[i], normal, worldPos,viewDirection, material);
	}
	vColor = vec4(outColor,1.0);
	gl_Position = projection * view * model * vec4(vPosition,1.0);
}

vec3 calculateDirectionalLight(DirectionalLight directionalLight, vec3 normal, vec3 viewDirection, Material material)
{

	vec3 nLightDirection = normalize(-directionalLight.direction);
	
	//emmissive
	vec3 emissive = material.emissive;

	//ambient
	vec3 ambient = material.ambient * directionalLight.ambient;

	//diffuse
	float diffuseFactor = max( dot(normal , nLightDirection) , 0.0 );
	vec3 diffuse = directionalLight.diffuse * (diffuseFactor * material.diffuse);

	//specular
	vec3 nReflectDirection = reflect( -nLightDirection, normal );
	float specFactor = pow( max( dot( viewDirection,nReflectDirection ) , 0.0 ) , material.shininess);
	vec3 specular = directionalLight.specular * (specFactor * material.specular);

	return emissive + ambient + diffuse + specular;
}

vec3 calculatePointLight(PointLight pointLight, vec3 normal, vec3 vertexPosition, vec3 viewDirection, Material material)
{
	//light direction
	vec3 nLightDirection = normalize(pointLight.position - vertexPosition);

	//light attenuation
	float d = length(pointLight.position - vertexPosition);
	float attenuation = 1 / (pointLight.constant + pointLight.linear * d + pointLight.quadratic * d * d);

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