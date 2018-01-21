#version 330 core
  
in vec2 texCords;

uniform sampler2D image;
uniform bool hBlur;

out vec4 FragColor;
const float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{             
    vec2 texelSize = 1.0 / textureSize(image, 0);
    vec3 result = texture(image, texCords).rgb * weight[0];
    if(hBlur)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, texCords + vec2(texelSize.x * i, 0.0)).rgb * weight[i];
            result += texture(image, texCords - vec2(texelSize.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, texCords + vec2(0.0, texelSize.y * i)).rgb * weight[i];
            result += texture(image, texCords - vec2(0.0, texelSize.y * i)).rgb * weight[i];
        }
    }
    FragColor = vec4(result, 1.0);
}