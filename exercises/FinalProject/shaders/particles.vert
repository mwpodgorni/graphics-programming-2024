#version 330 core

layout (location = 0) in vec2 ParticlePosition;
layout (location = 1) in float ParticleSize;
layout (location = 2) in float ParticleBirth;
layout (location = 3) in float ParticleDuration;
layout (location = 4) in vec4 ParticleColor;
layout (location = 5) in vec2 ParticleVelocity;

out vec4 Color;

uniform float CurrentTime;
uniform float Gravity;

void main()
{
    float age = CurrentTime - ParticleBirth;
    gl_PointSize = age < ParticleDuration ? ParticleSize : 0;

    float initialVelocity = 0.1;
    float acceleration = 0.05;
    float fadingSpeed = 0.005;

    float velocityFactor = min(1.0, age / ParticleDuration);

    vec2 velocity = vec2(0, initialVelocity + acceleration * velocityFactor);
    velocity -= vec2(0, fadingSpeed * (ParticleDuration - age));

    vec2 position = ParticlePosition;
    
    // Adjust x position based on initial x position
    if (ParticlePosition.x < 0.0) {
        // Move particles to the right until x = 0
        position.x += abs(ParticlePosition.x) * velocityFactor;
    } else if (ParticlePosition.x > 0.0) {
        // Move particles to the left until x = 0
        position.x -= ParticlePosition.x * velocityFactor;
    }
    
    position += velocity * age;
    position += 0.5 * vec2(0, Gravity) * age * age;
    gl_Position = vec4(position, -1.0, 1);

    float t = clamp(age / ParticleDuration, 0.0, 1.0);
    Color = mix(vec4(1.0, 1.0, 0.0, 1.0), vec4(1.0, 0.5, 0.0, 1.0), smoothstep(0.0, 0.333, t));
    Color = mix(Color, vec4(1.0, 0.5, 0.0, 1.0), smoothstep(0.333, 0.666, t));
    Color = mix(Color, vec4(0.0, 0.0, 0.0, 1.0), smoothstep(0.666, 1.0, t));
}