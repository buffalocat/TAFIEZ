#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 iPos;
layout (location = 4) in vec3 iScale;
layout (location = 5) in vec2 iTexOffset;
layout (location = 6) in vec4 iColor;

out vec3 Normal;
out vec2 TexCoord;
out vec4 Color;

uniform float texScale;
uniform mat4 PV; //Projection * View

void main()
{
    gl_Position = PV * vec4(iPos + (iScale * aPos), 1.0);
	Normal = aNormal;
    TexCoord = texScale * (iTexOffset + aTexCoord);
	Color = iColor;
}
