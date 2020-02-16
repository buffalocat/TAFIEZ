#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

uniform float shadow = 1.0f;

void main()
{
  FragColor = shadow * texture(screenTexture, TexCoords);
}