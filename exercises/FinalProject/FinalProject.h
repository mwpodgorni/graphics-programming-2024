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
	void InitializeModel();
	void InitializeCamera();
	void InitializeLights();

	void RenderGUI();

	void InitializeGeometry();
	void InitializeShaders();
	void LoadAndCompileShader(Shader& shader, const char* path);
	void EmitParticle();
	void LoadTexture(const char* filePath);

	static float Random01();
	static float RandomRange(float from, float to);
	static glm::vec2 RandomDirection();
	static Color RandomColor();

private:
	DearImGui m_imGui;

	glm::vec2 m_mousePosition;

	Camera m_camera;
	glm::vec3 m_cameraPosition;
	float m_cameraTranslationSpeed;
	float m_cameraRotationSpeed;
	bool m_cameraEnabled;
	bool m_cameraEnablePressed;

	Model m_model;

	glm::vec3 m_ambientColor;
	glm::vec3 m_lightColor;
	float m_lightIntensity;
	glm::vec3 m_lightPosition;

	VertexBufferObject m_vbo;
	VertexArrayObject m_vao;
	ShaderProgram m_shaderProgram;
	ShaderProgram::Location m_currentTimeUniform;
	std::shared_ptr<Material> m_particleMaterial;
	GLuint m_textureID;

	unsigned int m_particleCount;
	const unsigned int m_particleCapacity;
};
