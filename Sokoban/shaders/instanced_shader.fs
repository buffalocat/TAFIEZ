#version 330 core
in float LightIntensity;
in vec2 TexCoord;
in vec4 Color;

out vec4 FragColor;

uniform sampler2D atlas;

void main()
{
  FragColor = texture(atlas, TexCoord) * Color * vec4(LightIntensity, LightIntensity, LightIntensity, 1.0);
}
