#include "FinalProject.h"

#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/asset/ModelLoader.h>
#include <ituGL/asset/Texture2DLoader.h>
#include <ituGL/shader/Material.h>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <imgui.h>
#include <array>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stb_image.h>

struct Particle
{
	glm::vec3 position;
	float size;
	float birth;
	float duration;
	Color color;
	glm::vec2 velocity;
	glm::vec2 uv;
};

const std::array<VertexAttribute, 7> s_vertexAttributes =
{
	VertexAttribute(Data::Type::Float, 3), // position
	VertexAttribute(Data::Type::Float, 1), // size
	VertexAttribute(Data::Type::Float, 1), // birth
	VertexAttribute(Data::Type::Float, 1), // duration
	VertexAttribute(Data::Type::Float, 4), // color
	VertexAttribute(Data::Type::Float, 2), // velocity
	VertexAttribute(Data::Type::Float, 2),  // uv
};
FinalProject::FinalProject()
	: Application(1024, 1024, "Viewer demo")
	, m_cameraPosition(-0.01, 0.2, -5.5)
	, m_cameraTranslationSpeed(1.0f)
	, m_cameraRotationSpeed(0.5f)
	, m_cameraEnabled(false)
	, m_cameraEnablePressed(false)
	, m_mousePosition(GetMainWindow().GetMousePosition(true))
	, m_ambientColor(0.0f)
	, m_lightColor(0.0f)
	, m_lightIntensity(0.0f)
	, m_lightPosition(0.0f)
	, m_currentTimeUniform(0)
	, m_particleCount(0)
	, m_particleCapacity(2000)
{
}

void FinalProject::Initialize()
{
	std::cout << "Initialize" << std::endl;
	Application::Initialize();
	m_imGui.Initialize(GetMainWindow());
	InitializeModel();
	InitializeCamera();
	InitializeLights();
	InitializeGeometry();
	InitializeShaders();

	LoadTexture("textures/0049.png");
	GetDevice().EnableFeature(GL_DEPTH_TEST);
	GetDevice().EnableFeature(GL_PROGRAM_POINT_SIZE);
	GetDevice().EnableFeature(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	GetDevice().SetVSyncEnabled(true);
}

void FinalProject::Update()
{
	Application::Update();
	UpdateCamera();
	EmitParticle();
}
void FinalProject::UpdateCamera()
{
	Window& window = GetMainWindow();

	// Update if camera is enabled (controlled by SPACE key)
	{
		bool enablePressed = window.IsKeyPressed(GLFW_KEY_SPACE);
		if (enablePressed && !m_cameraEnablePressed)
		{
			m_cameraEnabled = !m_cameraEnabled;

			window.SetMouseVisible(!m_cameraEnabled);
			m_mousePosition = window.GetMousePosition(true);
		}
		m_cameraEnablePressed = enablePressed;
	}

	if (!m_cameraEnabled)
		return;

	glm::mat4 viewTransposedMatrix = glm::transpose(m_camera.GetViewMatrix());
	glm::vec3 viewRight = viewTransposedMatrix[0];
	glm::vec3 viewForward = -viewTransposedMatrix[2];

	// Update camera translation
	{
		glm::vec2 inputTranslation(0.0f);

		if (window.IsKeyPressed(GLFW_KEY_A))
			inputTranslation.x = -1.0f;
		else if (window.IsKeyPressed(GLFW_KEY_D))
			inputTranslation.x = 1.0f;

		if (window.IsKeyPressed(GLFW_KEY_W))
			inputTranslation.y = 1.0f;
		else if (window.IsKeyPressed(GLFW_KEY_S))
			inputTranslation.y = -1.0f;

		inputTranslation *= m_cameraTranslationSpeed;
		inputTranslation *= GetDeltaTime();

		// Double speed if SHIFT is pressed
		if (window.IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
			inputTranslation *= 2.0f;

		m_cameraPosition += inputTranslation.x * viewRight + inputTranslation.y * viewForward;
	}

	// Update camera rotation
	{
		glm::vec2 mousePosition = window.GetMousePosition(true);
		glm::vec2 deltaMousePosition = mousePosition - m_mousePosition;
		m_mousePosition = mousePosition;

		glm::vec3 inputRotation(-deltaMousePosition.x, deltaMousePosition.y, 0.0f);

		inputRotation *= m_cameraRotationSpeed;

		viewForward = glm::rotate(inputRotation.x, glm::vec3(0, 1, 0)) * glm::rotate(inputRotation.y, glm::vec3(viewRight)) * glm::vec4(viewForward, 0);
	}

	// Update view matrix
	m_camera.SetViewMatrix(m_cameraPosition, m_cameraPosition + viewForward);
}
void FinalProject::Render()
{
	// Clear the color and depth buffer
	GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f), true, 1.0);

	// Render the 3D model first
	GetDevice().EnableFeature(GL_DEPTH_TEST);
	GetDevice().DisableFeature(GL_BLEND);

	m_model.Draw();

	// Render particles
	GetDevice().DisableFeature(GL_DEPTH_TEST);
	GetDevice().EnableFeature(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	m_shaderProgram.Use();
	m_shaderProgram.SetUniform(m_currentTimeUniform, GetCurrentTime());

	// Set the view-projection matrix for the particles
	glm::mat4 viewProjMatrix = m_camera.GetViewProjectionMatrix();
	m_shaderProgram.SetUniform(m_viewProjMatrixUniform, viewProjMatrix);

	// Set the model matrix for the particles
	glm::mat4 modelMatrix = glm::translate(glm::vec3(0.0f, 0.0f, -5.0f)) * glm::scale(glm::vec3(0.1f));
	GLint modelMatrixUniform = m_shaderProgram.GetUniformLocation("ModelMatrix");
	m_shaderProgram.SetUniform(modelMatrixUniform, modelMatrix);

	m_vao.Bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureID);

	GLint textureLocation = m_shaderProgram.GetUniformLocation("particleTexture");
	glUniform1i(textureLocation, 0);

	glDrawArrays(GL_POINTS, 0, std::min(m_particleCount, m_particleCapacity));

	glBindTexture(GL_TEXTURE_2D, 0);
	VertexArrayObject::Unbind();

	RenderGUI();

	Application::Render();
}

void FinalProject::Cleanup()
{
	std::cout << "Cleanup" << std::endl;
	// Cleanup DearImGUI
	m_imGui.Cleanup();

	Application::Cleanup();
}
void FinalProject::LoadTexture(const char* filePath)
{
	int width, height, channels;
	unsigned char* data = stbi_load(filePath, &width, &height, &channels, 4);
	if (data)
	{
		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(data);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
}
void FinalProject::InitializeModel()
{
	Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/blinn-phong.vert");
	Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/blinn-phong.frag");
	std::shared_ptr<ShaderProgram> shaderProgram = std::make_shared<ShaderProgram>();
	shaderProgram->Build(vertexShader, fragmentShader);

	ShaderUniformCollection::NameSet filteredUniforms;
	filteredUniforms.insert("WorldMatrix");
	filteredUniforms.insert("ViewProjMatrix");
	filteredUniforms.insert("AmbientColor");
	filteredUniforms.insert("LightColor");

	std::shared_ptr<Material> material = std::make_shared<Material>(shaderProgram, filteredUniforms);
	material->SetUniformValue("Color", glm::vec4(1.0f));
	material->SetUniformValue("AmbientReflection", 1.0f);
	material->SetUniformValue("DiffuseReflection", 1.0f);
	material->SetUniformValue("SpecularReflection", 1.0f);
	material->SetUniformValue("SpecularExponent", 100.0f);

	ShaderProgram::Location worldMatrixLocation = shaderProgram->GetUniformLocation("WorldMatrix");
	ShaderProgram::Location viewProjMatrixLocation = shaderProgram->GetUniformLocation("ViewProjMatrix");
	ShaderProgram::Location ambientColorLocation = shaderProgram->GetUniformLocation("AmbientColor");
	ShaderProgram::Location lightColorLocation = shaderProgram->GetUniformLocation("LightColor");
	ShaderProgram::Location lightPositionLocation = shaderProgram->GetUniformLocation("LightPosition");
	ShaderProgram::Location cameraPositionLocation = shaderProgram->GetUniformLocation("CameraPosition");
	material->SetShaderSetupFunction([=](ShaderProgram& shaderProgram)
		{
			glm::mat4 modelMatrix = glm::translate(glm::vec3(0.0f, 0.0f, -5.0f)) * glm::scale(glm::vec3(0.1f));
			shaderProgram.SetUniform(worldMatrixLocation, modelMatrix);
			shaderProgram.SetUniform(viewProjMatrixLocation, m_camera.GetViewProjectionMatrix());

			shaderProgram.SetUniform(ambientColorLocation, m_ambientColor);
			shaderProgram.SetUniform(lightColorLocation, m_lightColor * m_lightIntensity);
			shaderProgram.SetUniform(lightPositionLocation, m_lightPosition);
			shaderProgram.SetUniform(cameraPositionLocation, m_cameraPosition);
		});

	ModelLoader loader(material);
	loader.SetCreateMaterials(true);
	loader.SetMaterialAttribute(VertexAttribute::Semantic::Position, "VertexPosition");
	loader.SetMaterialAttribute(VertexAttribute::Semantic::Normal, "VertexNormal");
	loader.SetMaterialAttribute(VertexAttribute::Semantic::TexCoord0, "VertexTexCoord");

	// Load model
	m_model = loader.Load("models/campfire2/source/campfire.obj");

	Texture2DLoader textureLoader(TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA8);
	textureLoader.SetFlipVertical(true);
	m_model.GetMaterial(0).SetUniformValue("ColorTexture", textureLoader.LoadShared("models/campfire2/source/campfire.png"));
}

void FinalProject::InitializeCamera()
{
	std::cout << "InitializeCamera" << std::endl;
	// Set view matrix, from the camera position looking to the origin
	m_camera.SetViewMatrix(m_cameraPosition, glm::vec3(0.0f));

	// Set perspective matrix
	float aspectRatio = GetMainWindow().GetAspectRatio();
	m_camera.SetPerspectiveProjectionMatrix(1.0f, aspectRatio, 0.1f, 1000.0f);

}

void FinalProject::InitializeLights()
{
	std::cout << "InitializeLights" << std::endl;
	// Initialize light variables
	m_ambientColor = glm::vec3(0.25f);
	m_lightColor = glm::vec3(1.0f);
	m_lightIntensity = 1.0f;
	m_lightPosition = glm::vec3(-10.0f, 20.0f, 10.0f);
}

void FinalProject::RenderGUI()
{
	//std::cout << "RenderGUI" << std::endl;
	m_imGui.BeginFrame();

	// Add debug controls for light properties
	ImGui::ColorEdit3("Ambient color", &m_ambientColor[0]);
	ImGui::Separator();
	ImGui::DragFloat3("Light position", &m_lightPosition[0], 0.1f);
	ImGui::ColorEdit3("Light color", &m_lightColor[0]);
	ImGui::DragFloat("Light intensity", &m_lightIntensity, 0.05f, 0.0f, 100.0f);
	ImGui::DragFloat("xPos", &xPos, 0.0f, -10.0f, 10.0f);
	ImGui::DragFloat("yPos", &yPos, 0.0f, -10.0f, 10.0f);
	ImGui::DragFloat("zPos", &zPos, 0.0f, -10.0f, 10.0f);
	ImGui::Separator();

	m_imGui.EndFrame();
}

void FinalProject::InitializeGeometry()
{
	m_vbo.Bind();

	m_vbo.AllocateData(m_particleCapacity * sizeof(Particle), BufferObject::Usage::DynamicDraw);

	m_vao.Bind();

	GLsizei stride = sizeof(Particle);
	GLint offset = 0;
	GLuint location = 0;
	for (const VertexAttribute& attribute : s_vertexAttributes)
	{
		m_vao.SetAttribute(location++, attribute, offset, stride);
		offset += attribute.GetSize();
	}

	VertexArrayObject::Unbind();
	VertexBufferObject::Unbind();
}

void FinalProject::InitializeShaders()
{
	Shader vertexShader(Shader::VertexShader);
	LoadAndCompileShader(vertexShader, "shaders/particles.vert");

	Shader fragmentShader(Shader::FragmentShader);
	LoadAndCompileShader(fragmentShader, "shaders/particles.frag");

	if (!m_shaderProgram.Build(vertexShader, fragmentShader))
	{
		std::cout << "Error linking shaders" << std::endl;
	}
	m_currentTimeUniform = m_shaderProgram.GetUniformLocation("CurrentTime");
	m_viewProjMatrixUniform = m_shaderProgram.GetUniformLocation("ViewProjMatrix");
}

void FinalProject::EmitParticle()
{
	float size = RandomRange(30.0f, 70.0f);
	float duration = RandomRange(1.0f, 3.0f);
	Color color = RandomColor();
	float xOffset = RandomRange(-0.35f, 0.35f);
	float zOffset = RandomRange(-0.35f, 0.35f);
	Particle particle;
	particle.position = glm::vec3(xOffset+xPos, yPos, zOffset+zPos);
	particle.size = size;
	particle.birth = GetCurrentTime();
	particle.duration = duration;
	particle.color = color;
	particle.velocity = RandomDirection() * RandomRange(0.5f, 5.0f);
	particle.velocity.y = abs(particle.velocity.y) + 0.5f; // Ensure upward movement
	particle.uv = glm::vec2(0.0f, 0.0f);
	std::cout << "EmitParticle Position: (" << particle.position.x << ", " << particle.position.y << ", " << particle.position.z << ")" << std::endl;

	unsigned int particleIndex = m_particleCount % m_particleCapacity;

	m_vbo.Bind();

	int offset = particleIndex * sizeof(Particle);
	m_vbo.UpdateData(std::span(&particle, 1), offset);

	VertexBufferObject::Unbind();

	m_particleCount++;
}

void FinalProject::LoadAndCompileShader(Shader& shader, const char* path)
{
	// Open the file for reading
	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cout << "Can't find file: " << path << std::endl;
		std::cout << "Is your working directory properly set?" << std::endl;
		return;
	}

	// Dump the contents into a string
	std::stringstream stringStream;
	stringStream << file.rdbuf() << '\0';

	// Set the source code from the string
	shader.SetSource(stringStream.str().c_str());

	// Try to compile
	if (!shader.Compile())
	{
		// Get errors in case of failure
		std::array<char, 256> errors;
		shader.GetCompilationErrors(errors);
		std::cout << "Error compiling shader: " << path << std::endl;
		std::cout << errors.data() << std::endl;
	}
}

float FinalProject::Random01()
{
	return static_cast<float>(rand()) / RAND_MAX;
}

float FinalProject::RandomRange(float from, float to)
{
	return Random01() * (to - from) + from;
}

glm::vec2 FinalProject::RandomDirection()
{
	return glm::normalize(glm::vec2(Random01() - 0.5f, Random01() - 0.5f));
}

Color FinalProject::RandomColor()
{
	return Color(Random01(), Random01(), Random01());
}