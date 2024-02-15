#version 330 core

layout (location = 0) in vec2 ParticlePosition;
// (todo) 02.X: Add more vertex attributes
layout (location = 1) in float size;
layout (location = 2) in float particleBirth;
layout (location = 3) in float particleDuration;
layout (location = 4) in vec4 particleColor;
layout (location = 6) in vec2 particleVelocity;

// (todo) 02.5: Add Color output variable here
out vec4 Color;

// (todo) 02.X: Add uniforms
uniform float CurrentTime;
uniform float Gravity;

void main()
{
	//gl_Position = vec4(ParticlePosition, 0.0, 1.0);
	
	Color = particleColor;
	float age = CurrentTime - particleBirth;
	
	gl_PointSize = age< particleDuration ? size : 0;
	
	vec2 position = ParticlePosition;
				// x + vt + 1/2at^2
	position = ParticlePosition + particleVelocity * age +  0.5f * vec2(0, Gravity) * age * age;

	gl_Position = vec4(position, 0.0, 1.0);
}

