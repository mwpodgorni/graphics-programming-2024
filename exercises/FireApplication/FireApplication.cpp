#include "FireApplication.h"

#include <ituGL/asset/TextureCubemapLoader.h>
#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/asset/ModelLoader.h>

#include <ituGL/camera/Camera.h>
#include <ituGL/scene/SceneCamera.h>

#include <ituGL/renderer/ForwardRenderPass.h>
#include <ituGL/renderer/SkyboxRenderPass.h>
#include <ituGL/scene/RendererSceneVisitor.h>
#include <ituGL/asset/Texture2DLoader.h>
#include <ituGL/shader/ShaderUniformCollection.h>
#include <ituGL/shader/Material.h>
#include <ituGL/geometry/Model.h>
#include <ituGL/scene/SceneModel.h>

#include <ituGL/scene/ImGuiSceneVisitor.h>
#include <imgui.h>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <fstream>
#include <sstream>


FireApplication::FireApplication()
	: Application(1024, 1024, "Fire demo")
	, m_renderer(GetDevice())
{
	std::cout << "constructor" << std::endl;
}

void FireApplication::Initialize()
{
	std::cout << "initialize" << std::endl;
	Application::Initialize();
	// Initialize DearImGUI
	m_imGui.Initialize(GetMainWindow());

	InitializeCamera();
	InitializeLights();
	InitializeMaterials();
	InitializeModels();
	InitializeRenderer();
	// Enable GL_PROGRAM_POINT_SIZE to have variable point size per-particle
	GetDevice().EnableFeature(GL_PROGRAM_POINT_SIZE);

	// Enable GL_BLEND to have blending on the particles, and configure it as additive blending
	GetDevice().EnableFeature(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	// We need to enable V-sync, otherwise the framerate would be too high and spawn multiple particles in one click
	GetDevice().SetVSyncEnabled(true);
}

void FireApplication::Update()
{
	Application::Update();
	m_cameraController.Update(GetMainWindow(), GetDeltaTime());
	// Add the scene nodes to the renderer
	RendererSceneVisitor rendererSceneVisitor(m_renderer);
	m_scene.AcceptVisitor(rendererSceneVisitor);
}

void FireApplication::Render()
{
	Application::Render();
	GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 0.0f), true, 1.0f);
	m_renderer.Render();
	//RenderGUI();
}

void FireApplication::Cleanup()
{
	// Cleanup DearImGUI
	m_imGui.Cleanup();

	Application::Cleanup();
}

void FireApplication::InitializeCamera()
{
	// Create the main camera
	std::shared_ptr<Camera> camera = std::make_shared<Camera>();
	camera->SetViewMatrix(glm::vec3(-1, 1, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	camera->SetPerspectiveProjectionMatrix(1.0f, 1.0f, 0.1f, 100.0f);

	// Create a scene node for the camera
	std::shared_ptr<SceneCamera> sceneCamera = std::make_shared<SceneCamera>("camera", camera);

	// Add the camera node to the scene
	m_scene.AddSceneNode(sceneCamera);

	// Set the camera scene node to be controlled by the camera controller
	m_cameraController.SetCamera(sceneCamera);
}
void FireApplication::InitializeLights()
{
	std::cout << "InitializeLights" << std::endl;
}
void FireApplication::InitializeMaterials()
{
	std::cout << "InitializeMaterial" << std::endl;
	// Load and build shader
	std::vector<const char*> vertexShaderPaths;
	vertexShaderPaths.push_back("shaders/version330.glsl");
	vertexShaderPaths.push_back("shaders/default.vert");
	Shader vertexShader = ShaderLoader(Shader::VertexShader).Load(vertexShaderPaths);

	std::vector<const char*> fragmentShaderPaths;
	fragmentShaderPaths.push_back("shaders/version330.glsl");
	fragmentShaderPaths.push_back("shaders/utils.glsl");
	fragmentShaderPaths.push_back("shaders/lambert-ggx.glsl");
	fragmentShaderPaths.push_back("shaders/lighting.glsl");
	fragmentShaderPaths.push_back("shaders/default_pbr.frag");
	Shader fragmentShader = ShaderLoader(Shader::FragmentShader).Load(fragmentShaderPaths);

	std::shared_ptr<ShaderProgram> shaderProgramPtr = std::make_shared<ShaderProgram>();
	shaderProgramPtr->Build(vertexShader, fragmentShader);

	// Get transform related uniform locations
	ShaderProgram::Location cameraPositionLocation = shaderProgramPtr->GetUniformLocation("CameraPosition");
	ShaderProgram::Location worldMatrixLocation = shaderProgramPtr->GetUniformLocation("WorldMatrix");
	ShaderProgram::Location viewProjMatrixLocation = shaderProgramPtr->GetUniformLocation("ViewProjMatrix");

	// Register shader with renderer
	m_renderer.RegisterShaderProgram(shaderProgramPtr,
		[=](const ShaderProgram& shaderProgram, const glm::mat4& worldMatrix, const Camera& camera, bool cameraChanged)
		{
			if (cameraChanged)
			{
				shaderProgram.SetUniform(cameraPositionLocation, camera.ExtractTranslation());
				shaderProgram.SetUniform(viewProjMatrixLocation, camera.GetViewProjectionMatrix());
			}
			shaderProgram.SetUniform(worldMatrixLocation, worldMatrix);
		},
		m_renderer.GetDefaultUpdateLightsFunction(*shaderProgramPtr)
	);

	// Filter out uniforms that are not material properties
	ShaderUniformCollection::NameSet filteredUniforms;
	filteredUniforms.insert("CameraPosition");
	filteredUniforms.insert("WorldMatrix");
	filteredUniforms.insert("ViewProjMatrix");
	filteredUniforms.insert("LightIndirect");
	filteredUniforms.insert("LightColor");
	filteredUniforms.insert("LightPosition");
	filteredUniforms.insert("LightDirection");
	filteredUniforms.insert("LightAttenuation");

	// Create reference material
	assert(shaderProgramPtr);
	m_defaultMaterial = std::make_shared<Material>(shaderProgramPtr, filteredUniforms);
}

void FireApplication::InitializeModels()
{
	std::cout << "InitializeModels" << std::endl;
	m_skyboxTexture = TextureCubemapLoader::LoadTextureShared("models/skybox/forest-skybox2.png", TextureObject::FormatRGB, TextureObject::InternalFormatSRGB8);
	m_skyboxTexture->Bind();
	float maxLod;
	m_skyboxTexture->GetParameter(TextureObject::ParameterFloat::MaxLod, maxLod);
	TextureCubemapObject::Unbind();
	m_defaultMaterial->SetUniformValue("AmbientColor", glm::vec3(0.25f));

	m_defaultMaterial->SetUniformValue("EnvironmentTexture", m_skyboxTexture);
	m_defaultMaterial->SetUniformValue("EnvironmentMaxLod", maxLod);
	m_defaultMaterial->SetUniformValue("Color", glm::vec3(1.0f));

	// Configure loader
	ModelLoader loader(m_defaultMaterial);

	// Create a new material copy for each submaterial
	loader.SetCreateMaterials(true);

	// Flip vertically textures loaded by the model loader
	loader.GetTexture2DLoader().SetFlipVertical(true);
	// Link vertex properties to attributes
	loader.SetMaterialAttribute(VertexAttribute::Semantic::Position, "VertexPosition");
	loader.SetMaterialAttribute(VertexAttribute::Semantic::Normal, "VertexNormal");
	loader.SetMaterialAttribute(VertexAttribute::Semantic::Tangent, "VertexTangent");
	loader.SetMaterialAttribute(VertexAttribute::Semantic::Bitangent, "VertexBitangent");
	loader.SetMaterialAttribute(VertexAttribute::Semantic::TexCoord0, "VertexTexCoord");

	// Link material properties to uniforms
	loader.SetMaterialProperty(ModelLoader::MaterialProperty::DiffuseColor, "Color");
	loader.SetMaterialProperty(ModelLoader::MaterialProperty::DiffuseTexture, "ColorTexture");
	loader.SetMaterialProperty(ModelLoader::MaterialProperty::NormalTexture, "NormalTexture");
	loader.SetMaterialProperty(ModelLoader::MaterialProperty::SpecularTexture, "SpecularTexture");


	std::shared_ptr<Model> campfireModel = loader.LoadShared("models/campfire2/source/campfire.obj");
	m_scene.AddSceneNode(std::make_shared<SceneModel>("campfire", campfireModel));
}

void FireApplication::InitializeRenderer()
{
	std::cout << "InitializeRenderer" << std::endl;
	m_renderer.AddRenderPass(std::make_unique<ForwardRenderPass>());
	m_renderer.AddRenderPass(std::make_unique<SkyboxRenderPass>(m_skyboxTexture));
}
void FireApplication::RenderGUI()
{
	std::cout << "renderGUI" << std::endl;
	m_imGui.BeginFrame();

	// Add debug controls for light properties
	//ImGui::DragFloat2("Particle Velocity", &m_particleVelocity[0]);
	ImGui::Separator();
	//ImGui::DragFloat3("Light position", &m_lightPosition[0], 0.1f);
	//ImGui::ColorEdit3("Light color", &m_lightColor[0]);
	//ImGui::DragFloat("Light intensity", &m_lightIntensity, 0.05f, 0.0f, 100.0f);
	//ImGui::Separator();

	m_imGui.EndFrame();
}

