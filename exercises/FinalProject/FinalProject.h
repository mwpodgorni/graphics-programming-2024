#pragma once

#include <ituGL/application/Application.h>
#include <ituGL/geometry/VertexBufferObject.h>
#include <ituGL/geometry/VertexArrayObject.h>
#include <ituGL/shader/ShaderProgram.h>
#include <ituGL/camera/Camera.h>
#include <ituGL/geometry/Model.h>
#include <ituGL/utils/DearImGui.h>
#include <ituGL/shader/Material.h>

class FinalProject : public Application
{
public:
	FinalProject();

protected:
	void Initialize() override;
	void Update() override;
	void Render() override;
	void Cleanup() override;

private:
	void LoadTexture(const char* filePath);
	void LoadAndCompileShader(Shader& shader, const char* path);
	void InitializeModel();
	void InitializeCamera();
	void InitializeGeometry();
	void InitializeShaders();
	void RenderGUI();
	void EmitParticle();
	void UpdateCamera();

	static float Random01();
	static float RandomRange(float from, float to);
	static glm::vec2 RandomDirection();
	static Color RandomColor();
private:
	DearImGui m_imGui;
	GLuint m_textureID;
	glm::vec2 m_mousePosition;
	Model m_model;

	Camera m_camera;
	glm::vec3 m_cameraPosition;
	float m_cameraTranslationSpeed;
	float m_cameraRotationSpeed;
	bool m_cameraEnabled;
	bool m_cameraEnablePressed;



	VertexBufferObject m_vbo;
	VertexArrayObject m_vao;
	ShaderProgram m_shaderProgram;
	ShaderProgram::Location m_currentTimeUniform;
	ShaderProgram::Location m_viewProjMatrixUniform;

	unsigned int m_particleCount = 0;
	const unsigned int m_particleCapacity = 2000;

	// Adjustable variables
	glm::vec3 aStartingPosition = glm::vec3(0.0f, 0.2f, 0.0f);
	glm::vec2 aSize = glm::vec2(30.0f, 70.0f);
	glm::vec2 aDuration = glm::vec2(1.0f, 2.5f);
	glm::vec2 aSpawnRange = glm::vec2(-0.4f, 0.4f);
	glm::vec2 aVelocityRange = glm::vec2(0.0f, 0.3f);

	glm::vec3 m_ambientColor = glm::vec3(0.25f);
	glm::vec3 m_lightColor = glm::vec3(1.0f);
	float m_lightIntensity = 0.5f;
	glm::vec3 m_lightPosition = glm::vec3(0.0f, 30.0f, 0.0f);
};
