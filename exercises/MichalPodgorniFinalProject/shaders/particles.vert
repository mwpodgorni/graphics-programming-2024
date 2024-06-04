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
uniform mat4 ModelMatrix;

void main()
{
    float age = CurrentTime - ParticleBirth;
    float time = clamp(age / ParticleDuration, 0.0, 1.0);

    float sizeDecreaseFactor = time * 0.5;
    gl_PointSize = ParticleSize * (1.0 - sizeDecreaseFactor);

    float initialVelocity = ParticleVelocity.y;
    float acceleration = 0.05;
    float fadingSpeed = 0.005;

    float velocityFactor = min(1.0, age / ParticleDuration);

    vec3 velocity = vec3(0, initialVelocity + acceleration * velocityFactor, 0);
    velocity.y += initialVelocity + acceleration 
    * velocityFactor - fadingSpeed * (ParticleDuration - age);
    velocity.y /=1.8;
    vec3 position = ParticlePosition;
    
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

    vec4 worldPosition = ModelMatrix * vec4(position, 1.0);
    gl_Position = ViewProjMatrix * worldPosition;

    float t = clamp(age / ParticleDuration, 0.0, 1.0);
    Color = mix(vec4(1.0, 1.0, 0.0, 1.0), vec4(1.0, 0.5, 0.0, 1.0), smoothstep(0.0, 0.3, t));
    Color = mix(Color, vec4(1.0, 0.5, 0.0, 1.0), smoothstep(0.3, 0.5, t));
    Color = mix(Color, vec4(0.0, 0.0, 0.0, 1.0), smoothstep(0.5, 1.0, t));
}