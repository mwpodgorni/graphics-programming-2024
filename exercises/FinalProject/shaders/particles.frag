#version 330 core

out vec4 FragColor;

in vec4 Color;

uniform sampler2D particleTexture;

void main()
{
    vec2 TexCoords = gl_PointCoord;
    vec4 texColor = texture(particleTexture, TexCoords);
    FragColor = vec4(Color.rgb, Color.a * texColor.a);
}