#version 460 core
in float LightIntensity;
in vec2 TexCoord;
in vec3 Color;

out vec4 FragColor;

uniform sampler2D atlas;

void main()
{
    FragColor = vec4(LightIntensity * texture(atlas, TexCoord).xyz * Color, 1);
}
