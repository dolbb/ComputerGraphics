#version 150

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;

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
	
	float const;
	float linear;
	float quadratic;
};

uniform vec3 eye;
uniform mat3 normalTransform;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform Material material;
uniform DirectionalLight directionalLight;
//TODO:		add array of point lights

out vec4 vColor;

vec3 calculateDirectionalLight(DirectionalLight directionalLight, vec3 normal, vec3 viewDirection, Material material);
vec3 calculatePointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDirection, Material material);

void main()
{
	vec3 normal=normalize(normalTransform*vNormal);
	vec3 worldPos=model*vPosition;
	vec3 viewDirection=normalize(eye-worldPos);
	vec3 tmpColor=vec3(0.0);
	tmpColor+=calculateDirectionalLight(directionalLight, normal, viewDirection, material);
	//TODO:		calculate each point light's light contribution
	vColor=vec4(tmpColor,1.0);
	gl_position=projection*view*model*vec4(vPosition,1.0);
}

vec3 calculateDirectionalLight(DirectionalLight directionalLight, vec3 normal, vec3 viewDirection, Material material)
{
	vec3 nLightDirection=normalize(-directionalLight.direction);
	float diffuseFactor=max(dot(normal,nLightDirection),0.0);
	vec3 nReflectDirection=reflect(-nLightDirection, normal);
	float specFactor=pow(max(dot(viewDirection,nReflectDirection),0.0),material.shininess);
	
	vec3 emissive=material.emissive;
	vec3 ambient=directionalLight.ambient*material.ambient;
	vec3 diffuse=directionalLight.diffuse*(diffuseFactor*material.diffuse);
	vec3 specular=directionalLight.specular*(specFactor*material.specular);
	
	return emissive+ambient+diffuse+specular;
}

vec3 calculatePointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDirection,Material material)
{
	vec3 nLightDirection=normalize(pointLight.position-fragPos);
	float diffuseFactor=max(dot(normal,nLightDirection),0.0);
	vec3 nReflectDirection=reflect(-nLightDirection, normal);
	float specFactor=pow(max(dot(viewDirection,nReflectDirection),0.0),material.shininess);
	//attenuation
	float d=length(pointLight.position-fragPos);
	float attenuation=1/(pointLight.const+pointLight.linear*d+pointLight.quadratic*d*d);
	
	vec3 emissive=material.emissive;
	vec3 ambient=pointLight.ambient*material.ambient*attenuation;
	vec3 diffuse=(pointLight.diffuse*(diffuseFactor*material.diffuse))*attenuation;
	vec3 specular=(pointLight.specular*(specFactor*material.specular))*attenuation;
	
	return emissive+ambient+diffuse+specular;
}
