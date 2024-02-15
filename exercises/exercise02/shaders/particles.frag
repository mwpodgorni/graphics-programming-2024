#version 330 core

out vec4 FragColor;

// (todo) 02.5: Add Color input variable here


void main()
{
	// (todo) 02.3: Compute alpha using the built-in variable gl_PointCoord
	//FragColor = vec4(1, 1, 1, 1);
	float alpha = 1 - length(gl_PointCoord * 2 - 1);
	FragColor = vec4(1,1,1, alpha);
}
