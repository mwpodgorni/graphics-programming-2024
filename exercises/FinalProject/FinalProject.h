#pragma once

#include <ituGL/application/Application.h>
#include <ituGL/geometry/VertexBufferObject.h>
#include <ituGL/geometry/VertexArrayObject.h>
#include <ituGL/shader/ShaderProgram.h>
#include <ituGL/camera/Camera.h>
#include <ituGL/geometry/Model.h>
#include <ituGL/utils/DearImGui.h>

class Texture2DObject;

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

	void UpdateCamera();

	void RenderGUI();




	// Initialize the VBO and VAO
	void InitializeGeometry();

	// Load, compile and link shaders
	void InitializeShaders();

	// Helper function to encapsulate loading and compiling a shader
	void LoadAndCompileShader(Shader& shader, const char* path);

	// Emit a new particle
	void EmitParticle(const glm::vec2& position, float size, float duration, const Color& color, const glm::vec2& velocity);

	// Helper methods for random values
	static float Random01();
	static float RandomRange(float from, float to);
	static glm::vec2 RandomDirection();
	static Color RandomColor();

private:
	// Helper object for debug GUI
	DearImGui m_imGui;

	// Mouse position for camera controller
	glm::vec2 m_mousePosition;

	// Camera controller parameters
	Camera m_camera;
	glm::vec3 m_cameraPosition;
	float m_cameraTranslationSpeed;
	float m_cameraRotationSpeed;
	bool m_cameraEnabled;
	bool m_cameraEnablePressed;

	// Loaded model
	Model m_model;

	// Add light variables
	glm::vec3 m_ambientColor;
	glm::vec3 m_lightColor;
	float m_lightIntensity;
	glm::vec3 m_lightPosition;

	// Specular exponent debug
	float m_specularExponentGrass;

	// All particles stored in a single VBO with interleaved attributes
	VertexBufferObject m_vbo;

	// VAO that represents the particle system
	VertexArrayObject m_vao;

	// Particles shader program
	ShaderProgram m_shaderProgram;

	// Location of the "CurrentTime" uniform
	ShaderProgram::Location m_currentTimeUniform;

	// Location of the "Gravity" uniform
	ShaderProgram::Location m_gravityUniform;


	// Total number of particles created
	unsigned int m_particleCount;

	// Max number of particles that can exist at the same time
	const unsigned int m_particleCapacity;
};
