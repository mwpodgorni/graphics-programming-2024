#version 330 core

layout (location = 0) in vec3 ParticlePosition;
layout (location = 1) in float ParticleSize;
layout (location = 2) in float ParticleBirth;
layout (location = 3) in float ParticleDuration;
layout (location = 4) in vec4 ParticleColor;
layout (location = 5) in vec2 ParticleVelocity;

out vec4 Color;

uniform float CurrentTime;
uniform mat4 ViewProjMatrix; // View-Projection Matrix
uniform mat4 ModelMatrix;    // Model Matrix

void main()
{
    float age = CurrentTime - ParticleBirth;
    gl_PointSize = age < ParticleDuration ? ParticleSize : 0;

    float initialVelocity = 0.2;
    float acceleration = 0.05;
    float fadingSpeed = 0.005;

    float velocityFactor = min(1.0, age / ParticleDuration);

    // Combine horizontal velocity from ParticleVelocity and a vertical component
    vec3 velocity = vec3(0, initialVelocity + acceleration * velocityFactor, 0);
    velocity.y += initialVelocity + acceleration * velocityFactor - fadingSpeed * (ParticleDuration - age);

    vec3 position = ParticlePosition;
    
    // Adjust x and z positions to move towards the center (0, 0)
    if (ParticlePosition.x < 0.0) {
        position.x += abs(ParticlePosition.x) * velocityFactor;
    } else if (ParticlePosition.x > 0.0) {
        position.x -= ParticlePosition.x * velocityFactor;
    }
    
    if (ParticlePosition.z < 0.0) {
        position.z += abs(ParticlePosition.z) * velocityFactor;
    } else if (ParticlePosition.z > 0.0) {
        position.z -= ParticlePosition.z * velocityFactor;
    }

    position += velocity * age;

    // Transform the position with the model matrix to maintain relative position to the model
    vec4 worldPosition = ModelMatrix * vec4(position, 1.0);

    // Transform to clip space using the view-projection matrix
    gl_Position = ViewProjMatrix * worldPosition;

    float t = clamp(age / ParticleDuration, 0.0, 1.0);
    Color = mix(vec4(1.0, 1.0, 0.0, 1.0), vec4(1.0, 0.5, 0.0, 1.0), smoothstep(0.0, 0.333, t));
    Color = mix(Color, vec4(1.0, 0.5, 0.0, 1.0), smoothstep(0.333, 0.666, t));
    Color = mix(Color, vec4(0.0, 0.0, 0.0, 1.0), smoothstep(0.666, 1.0, t));
}