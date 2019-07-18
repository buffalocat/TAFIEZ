#version 330 core
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D atlas;
uniform vec4 color;

void main()
{
    FragColor = texture(atlas, 0.25*TexCoord) * color;
}
