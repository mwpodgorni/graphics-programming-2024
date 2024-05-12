#version 330 core

out vec4 FragColor;

in vec4 Color;

void main()
{
	float alpha = 1.0 - length(gl_PointCoord * 2.0 - 1.0);
	vec3 fireColor = mix(vec3(1.0, 0.5, 0.0), vec3(1.0, 0.0, 0.0), gl_PointCoord.y);
	FragColor = vec4(fireColor, alpha);
}
