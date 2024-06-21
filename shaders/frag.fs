#version 460 core

in vec4 outColor;
in vec2 outTexCoords;

out vec4 fragColor;

layout(location=0) uniform sampler2D tex;
layout(std140, binding=1) uniform Props
{
   vec4 tint;
};

void main()
{
   fragColor = outColor * texture(tex, outTexCoords) * tint;
}
