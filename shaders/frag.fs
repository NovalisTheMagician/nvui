#version 460 core
in vec4 outColor;
in vec2 outTexCoords;
out vec4 fragColor;
layout(location=1) uniform sampler2D tex;
layout(location=2) uniform vec4 tint;
void main()
{
   fragColor = outColor * texture(tex, outTexCoords) * tint;
}
