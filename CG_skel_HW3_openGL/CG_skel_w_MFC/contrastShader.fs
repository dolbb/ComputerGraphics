#version 330

in vec2 texCoords;

uniform sampler2D image;

out vec4 fragColor;

const float contrast = 0.4;

void main()
{
	fragColor = texture(image, texCoords);
	fragColor.rgb = (fragColor.rgb - 0.5) * (1.0 + contrast) + 0.5;
}