#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main(){

    vec4 texColor = texture(texture1, TexCoords);
    if(texColor.a < 0.6)
        discard;
    FragColor = texColor;
    BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

}