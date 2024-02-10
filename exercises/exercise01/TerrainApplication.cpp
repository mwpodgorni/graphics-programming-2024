#include "TerrainApplication.h"

// (todo) 01.1: Include the libraries you need
#include <ituGL/geometry/VertexAttribute.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <list>
// Helper structures. Declared here only for this exercise
struct Vector2
{
	Vector2() : Vector2(0.f, 0.f) {}
	Vector2(float x, float y) : x(x), y(y) {}
	float x, y;
};

struct Vector3
{
	Vector3() : Vector3(0.f, 0.f, 0.f) {}
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
	float x, y, z;

	Vector3 Normalize() const
	{
		float length = std::sqrt(1 + x * x + y * y);
		return Vector3(x / length, y / length, z / length);
	}
};

// (todo) 01.8: Declare an struct with the vertex format



TerrainApplication::TerrainApplication()
	: Application(1024, 1024, "Terrain demo"), m_gridX(16), m_gridY(16), m_shaderProgram(0)
{
}

void TerrainApplication::Initialize()
{
	Application::Initialize();

	// Build shaders and store in m_shaderProgram
	BuildShaders();

	// (todo) 01.1: Create containers for the vertex position
	std::vector<Vector3>  positions;
	std::vector<Vector2> texturePositions;

	// Fill in vertex data
	for (float x = 0.0f; x < m_gridX; x++) {
		for (float y = 0.0f; y < m_gridY; y++) {
			float textureX = x / (m_gridX / 4);
			float textureY = y / (m_gridY / 4);
			// position data
			positions.push_back(Vector3(x / m_gridX - 0.5f, y / m_gridY - 0.5f, 0.0f));
			positions.push_back(Vector3(x / m_gridX - 0.5f, (y + 1) / m_gridY - 0.5f, 0.0f));
			positions.push_back(Vector3((x + 1) / m_gridX - 0.5f, (y + 1) / m_gridY - 0.5f, 0.0f));

			positions.push_back(Vector3(x / m_gridX - 0.5f, y / m_gridY - 0.5f, 0.0f));
			positions.push_back(Vector3((x + 1) / m_gridX - 0.5f, (y + 1) / m_gridY - 0.5f, 0.0f));
			positions.push_back(Vector3((x + 1) / m_gridX - 0.5f, y / m_gridY - 0.5f, 0.0f));

			// Texture coordinate data
			texturePositions.push_back(Vector2(textureX, textureY));
			texturePositions.push_back(Vector2(textureX, textureY + 1.0f / m_gridY));
			texturePositions.push_back(Vector2(textureX + 1.0f / m_gridX, textureY + 1.0f / m_gridY));

			texturePositions.push_back(Vector2(textureX, textureY));
			texturePositions.push_back(Vector2(textureX + 1.0f / m_gridX, textureY + 1.0f / m_gridY));
			texturePositions.push_back(Vector2(textureX + 1.0f / m_gridX, textureY));
		}
	}
	size_t positionsSize = positions.size() * 3 * sizeof(Vector3); // 3 floats per position
	size_t textureCoordsSize = texturePositions.size() * 2 * sizeof(Vector2); // 2 floats per texture coordinate
	std::cout << "1:" << positionsSize << std::endl;
	std::cout << "2:" << textureCoordsSize << std::endl;

	size_t totalSize = positionsSize + textureCoordsSize;
	vbo.Bind();
	vbo.AllocateData(totalSize);

	size_t textureCoordsOffset = positionsSize;

	vbo.UpdateData(std::span(positions));
	size_t currentIndex = 0;
	vbo.UpdateData(std::span(texturePositions), textureCoordsOffset);


	vao.Bind();
	vbo.Bind();
	vertexCount = positions.size();

	VertexAttribute positionAttribute(Data::Type::Float, 3);
	vao.SetAttribute(0, positionAttribute, 0);

	VertexAttribute textureAttribute(Data::Type::Float, 2);
	vao.SetAttribute(1, textureAttribute, positionsSize);

	vao.Unbind();
	vbo.Unbind();

	// uncomment this call to draw in wireframe polygons.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

}

void TerrainApplication::Update()
{
	Application::Update();

	UpdateOutputMode();
}

void TerrainApplication::Render()
{
	Application::Render();

	// Clear color and depth
	GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

	// Set shader to be used
	glUseProgram(m_shaderProgram);

	// (todo) 01.1: Draw the grid
	vao.Bind();
	//std::cout << vertexCount << std::endl;
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

void TerrainApplication::Cleanup()
{
	Application::Cleanup();
}


void TerrainApplication::BuildShaders()
{
	const char* vertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 1) in vec2 aTexCoord;\n"
		"layout (location = 2) in vec3 aColor;\n"
		"layout (location = 3) in vec3 aNormal;\n"
		"uniform mat4 Matrix = mat4(1);\n"
		"out vec2 texCoord;\n"
		"out vec3 color;\n"
		"out vec3 normal;\n"
		"void main()\n"
		"{\n"
		"   texCoord = aTexCoord;\n"
		"   color = aColor;\n"
		"   normal = aNormal;\n"
		"   gl_Position = Matrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"}\0";
	const char* fragmentShaderSource = "#version 330 core\n"
		"uniform uint Mode = 0u;\n"
		"in vec2 texCoord;\n"
		"in vec3 color;\n"
		"in vec3 normal;\n"
		"out vec4 FragColor;\n"
		"void main()\n"
		"{\n"
		"   switch (Mode)\n"
		"   {\n"
		"   default:\n"
		"   case 0u:\n"
		"       FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
		"       break;\n"
		"   case 1u:\n"
		"       FragColor = vec4(fract(texCoord), 0.0f, 1.0f);\n"
		"       break;\n"
		"   case 2u:\n"
		"       FragColor = vec4(color, 1.0f);\n"
		"       break;\n"
		"   case 3u:\n"
		"       FragColor = vec4(normalize(normal), 1.0f);\n"
		"       break;\n"
		"   case 4u:\n"
		"       FragColor = vec4(color * max(dot(normalize(normal), normalize(vec3(1,0,1))), 0.2f), 1.0f);\n"
		"       break;\n"
		"   }\n"
		"}\n\0";

	// vertex shader
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// fragment shader
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// check for shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// link shaders
	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	m_shaderProgram = shaderProgram;
}

void TerrainApplication::UpdateOutputMode()
{
	for (int i = 0; i <= 4; ++i)
	{
		if (GetMainWindow().IsKeyPressed(GLFW_KEY_0 + i))
		{
			int modeLocation = glGetUniformLocation(m_shaderProgram, "Mode");
			glUseProgram(m_shaderProgram);
			glUniform1ui(modeLocation, i);
			break;
		}
	}
	if (GetMainWindow().IsKeyPressed(GLFW_KEY_TAB))
	{
		const float projMatrix[16] = { 0, -1.294f, -0.721f, -0.707f, 1.83f, 0, 0, 0, 0, 1.294f, -0.721f, -0.707f, 0, 0, 1.24f, 1.414f };
		int matrixLocation = glGetUniformLocation(m_shaderProgram, "Matrix");
		glUseProgram(m_shaderProgram);
		glUniformMatrix4fv(matrixLocation, 1, false, projMatrix);
	}
}
