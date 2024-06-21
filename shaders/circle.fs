#version 460 core

in vec4 outColor;
in vec2 outTexCoords;

out vec4 fragColor;

layout(location=0) uniform sampler2D tex;
layout(std140, binding=1) uniform Props
{
   vec4 tint;
};
layout(std140, binding=2) uniform CircleProps
{
    vec2 circleCenter;
    vec2 circleRadius;
};

void main() 
{
    float dist = length(gl_FragCoord.xy - circleCenter);
    if(dist > circleRadius.r) discard;
    fragColor = outColor * texture(tex, outTexCoords) * tint;
}
