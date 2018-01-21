#version 330 core
  
in vec2 TexCoords;

uniform sampler2D sceneImage;
uniform sampler2D bloomImage;
uniform float exposure;

out vec4 FragColor;

void main()
{             
    const float gamma = 2.2;
    vec3 outColor = texture(sceneImage, TexCoords).rgb;      
    vec3 bloomColor = texture(bloomImage, TexCoords).rgb;
    outColor += bloomColor;
    FragColor = vec4(outColor,1.0);
}