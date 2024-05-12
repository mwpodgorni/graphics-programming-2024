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
// Structure defining that Particle data
struct Particle
{
	glm::vec2 position;
	float size;
	float birth;
	float duration;
	Color color;
	glm::vec2 velocity;
};

// List of attributes of the particle. Must match the structure above
const std::array<VertexAttribute, 6> s_vertexAttributes =
{
	VertexAttribute(Data::Type::Float, 2), // position
	VertexAttribute(Data::Type::Float, 1), // size
	VertexAttribute(Data::Type::Float, 1), // birth
	VertexAttribute(Data::Type::Float, 1), // duration
	VertexAttribute(Data::Type::Float, 4), // color
	VertexAttribute(Data::Type::Float, 2), // velocity
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
	, m_specularExponentGrass(100.0f)
	, m_currentTimeUniform(0)
	, m_gravityUniform(0)
	, m_particleCount(0)
	, m_particleCapacity(2048)  // You can change the capacity here to have more particles
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

	DeviceGL& device = GetDevice();
	device.EnableFeature(GL_DEPTH_TEST);
	device.SetVSyncEnabled(true);


	// Initialize the mouse position with the current position of the mouse
	m_mousePosition = GetMainWindow().GetMousePosition(true);

	// Enable GL_PROGRAM_POINT_SIZE to have variable point size per-particle
	GetDevice().EnableFeature(GL_PROGRAM_POINT_SIZE);

	// Enable GL_BLEND to have blending on the particles, and configure it as additive blending
	GetDevice().EnableFeature(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	// We need to enable V-sync, otherwise the framerate would be too high and spawn multiple particles in one click
	GetDevice().SetVSyncEnabled(true);

	// Get "CurrentTime" uniform location in the shader program
	m_currentTimeUniform = m_shaderProgram.GetUniformLocation("CurrentTime");

	// Get "Gravity" uniform location in the shader program
	m_gravityUniform = m_shaderProgram.GetUniformLocation("Gravity");
}

void FinalProject::Update()
{
	Application::Update();
	//std::cout << "Update" << std::endl;
	UpdateCamera();
	const Window& window = GetMainWindow();

	// Get the mouse position this frame
	glm::vec2 mousePosition = window.GetMousePosition(true);

	// Emit particles while the left button is pressed
	//if (window.IsMouseButtonPressed(Window::MouseButton::Left))
	//{
	float size = RandomRange(10.0f, 30.0f);
	float duration = RandomRange(1.0f, 2.0f);
	Color color = RandomColor();
	glm::vec2 velocity = 0.5f * (mousePosition - m_mousePosition) / GetDeltaTime();

	EmitParticle(mousePosition, size, duration, color, velocity);
	//}

	// save the mouse position (to compare next frame and obtain velocity)
	m_mousePosition = mousePosition;
}

void FinalProject::Render()
{
	//std::cout << "Render" << std::endl;

	// Clear color and depth
	GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

	m_model.Draw();
	// Set our particles shader program
	m_shaderProgram.Use();

	// Set CurrentTime uniform
	m_shaderProgram.SetUniform(m_currentTimeUniform, GetCurrentTime());

	// Set Gravity uniform
	m_shaderProgram.SetUniform(m_gravityUniform, -9.8f);

	// Bind the particle system VAO
	m_vao.Bind();

	// Draw points. The amount of points can't exceed the capacity
	glDrawArrays(GL_POINTS, 0, std::min(m_particleCount, m_particleCapacity));
	// Render the debug user interface
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

void FinalProject::InitializeModel()
{
	// Load and build shader
	Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/blinn-phong.vert");
	Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/blinn-phong.frag");
	std::shared_ptr<ShaderProgram> shaderProgram = std::make_shared<ShaderProgram>();
	shaderProgram->Build(vertexShader, fragmentShader);

	// Filter out uniforms that are not material properties
	ShaderUniformCollection::NameSet filteredUniforms;
	filteredUniforms.insert("WorldMatrix");
	filteredUniforms.insert("ViewProjMatrix");
	filteredUniforms.insert("AmbientColor");
	filteredUniforms.insert("LightColor");

	// Create reference material
	std::shared_ptr<Material> material = std::make_shared<Material>(shaderProgram, filteredUniforms);
	material->SetUniformValue("Color", glm::vec4(1.0f));
	material->SetUniformValue("AmbientReflection", 1.0f);
	material->SetUniformValue("DiffuseReflection", 1.0f);
	material->SetUniformValue("SpecularReflection", 1.0f);
	material->SetUniformValue("SpecularExponent", 100.0f);

	// Setup function
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

			// Set camera and light uniforms
			shaderProgram.SetUniform(ambientColorLocation, m_ambientColor);
			shaderProgram.SetUniform(lightColorLocation, m_lightColor * m_lightIntensity);
			shaderProgram.SetUniform(lightPositionLocation, m_lightPosition);
			shaderProgram.SetUniform(cameraPositionLocation, m_cameraPosition);
		});

	// Configure loader
	ModelLoader loader(material);
	loader.SetCreateMaterials(true);
	loader.SetMaterialAttribute(VertexAttribute::Semantic::Position, "VertexPosition");
	loader.SetMaterialAttribute(VertexAttribute::Semantic::Normal, "VertexNormal");
	loader.SetMaterialAttribute(VertexAttribute::Semantic::TexCoord0, "VertexTexCoord");

	// Load model
	m_model = loader.Load("models/campfire2/source/campfire.obj");

	// Load and set textures
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
	ImGui::Separator();
	ImGui::DragFloat("Specular exponent (grass)", &m_specularExponentGrass, 1.0f, 0.0f, 1000.0f);

	m_imGui.EndFrame();
}

void FinalProject::UpdateCamera()
{
	std::cout << "UpdateCamera" << std::endl;
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
	std::cout << m_cameraPosition.x << ", " << m_cameraPosition.y << ", " << m_cameraPosition.z << std::endl;
	m_camera.SetViewMatrix(m_cameraPosition, m_cameraPosition + viewForward);
}

// Nothing to do in this method for this exercise.
// Change s_vertexAttributes and the Particle struct to add new vertex attributes
void FinalProject::InitializeGeometry()
{
	m_vbo.Bind();

	// Allocate enough data for all the particles
	// Notice the DynamicDraw usage, because we will update the buffer every time we emit a particle
	m_vbo.AllocateData(m_particleCapacity * sizeof(Particle), BufferObject::Usage::DynamicDraw);

	m_vao.Bind();

	// Automatically iterate through the vertex attributes, and set the pointer
	// We use interleaved attributes, so the offset is local to the particle, and the stride is the size of the particle
	GLsizei stride = sizeof(Particle);
	GLint offset = 0;
	GLuint location = 0;
	for (const VertexAttribute& attribute : s_vertexAttributes)
	{
		m_vao.SetAttribute(location++, attribute, offset, stride);
		offset += attribute.GetSize();
	}

	// Unbind VAO and VBO
	VertexArrayObject::Unbind();
	VertexBufferObject::Unbind();
}

// Load, compile and Build shaders
void FinalProject::InitializeShaders()
{
	// Load and compile vertex shader
	Shader vertexShader(Shader::VertexShader);
	LoadAndCompileShader(vertexShader, "shaders/particles.vert");

	// Load and compile fragment shader
	Shader fragmentShader(Shader::FragmentShader);
	LoadAndCompileShader(fragmentShader, "shaders/particles.frag");

	// Attach shaders and link
	if (!m_shaderProgram.Build(vertexShader, fragmentShader))
	{
		std::cout << "Error linking shaders" << std::endl;
	}
}

void FinalProject::EmitParticle(const glm::vec2& position, float size, float duration, const Color& color, const glm::vec2& velocity)
{
	// Initialize the particle
	Particle particle;
	particle.position = glm::vec2(-0.02f, -0.45f);
	particle.size = size;
	particle.birth = GetCurrentTime();
	particle.duration = duration;
	particle.color = color;
	particle.velocity = RandomDirection() * RandomRange(0.5f, 2.0f);

	// Get the index in the circular buffer
	unsigned int particleIndex = m_particleCount % m_particleCapacity;

	// Bind the VBO before updating data
	m_vbo.Bind();

	// Update the particle data in the VBO
	int offset = particleIndex * sizeof(Particle);
	m_vbo.UpdateData(std::span(&particle, 1), offset);

	// Unbind the VBO
	VertexBufferObject::Unbind();

	// Increment the particle count
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