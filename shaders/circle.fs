#version 460 core
in vec4 outColor;
in vec2 outTexCoords;
out vec4 fragColor;
layout(location=1) uniform sampler2D tex;
layout(location=2) uniform vec4 tint;
layout(location=3) uniform vec2 circleCenter;
layout(location=4) uniform float circleRadius;
void main() 
{
    float dist = length(gl_FragCoord.xy - circleCenter);
    if(dist > circleRadius) discard;
    fragColor = outColor * texture(tex, outTexCoords) * tint;
}
