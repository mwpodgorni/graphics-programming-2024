#version 330 core

in vec4 Color;
in vec2 TexCoords;  // Receive UV coordinates

out vec4 FragColor;

uniform sampler2D TextureSampler;  // Texture sampler for particle texture

void main()
{
    vec4 textureColor = texture(TextureSampler, TexCoords);  // Sample the texture
    FragColor = textureColor * Color;  // Blend texture color with vertex color
}