#version 330 core

layout (location = 0) in vec3 ParticlePosition;
layout (location = 1) in float ParticleSize;
layout (location = 2) in float ParticleBirth;
layout (location = 3) in float ParticleDuration;
layout (location = 4) in vec4 ParticleColor;
layout (location = 5) in vec2 ParticleVelocity;

out vec4 Color;

uniform float CurrentTime;
uniform mat4 ViewProjMatrix;

void main()
{
    Color = ParticleColor;
    float age = CurrentTime - ParticleBirth;
    gl_PointSize = age < ParticleDuration ? ParticleSize : 0;

    // Calculate the current position
    vec3 position = ParticlePosition;
    position.xy += ParticleVelocity * age;

    // Transform the position with the View and Projection matrix
    gl_Position = ViewProjMatrix * vec4(position, 1.0);
}