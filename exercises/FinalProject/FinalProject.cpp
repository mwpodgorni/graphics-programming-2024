#include "FinalProject.h"

#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/asset/ModelLoader.h>
#include <ituGL/asset/Texture2DLoader.h>
#include <ituGL/shader/Material.h>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <imgui.h>

FinalProject::FinalProject()
	: Application(1024, 1024, "Viewer demo")
	, m_cameraPosition(0, 30, 30)
	, m_cameraTranslationSpeed(20.0f)
	, m_cameraRotationSpeed(0.5f)
	, m_cameraEnabled(false)
	, m_cameraEnablePressed(false)
	, m_mousePosition(GetMainWindow().GetMousePosition(true))
	, m_ambientColor(0.0f)
	, m_lightColor(0.0f)
	, m_lightIntensity(0.0f)
	, m_lightPosition(0.0f)
	, m_specularExponentGrass(100.0f)
{
}

void FinalProject::Initialize()
{
	std::cout << "Initialize" << std::endl;
}

void FinalProject::Update()
{
	std::cout << "Update" << std::endl;

}

void FinalProject::Render()
{
	std::cout << "Render" << std::endl;

}

void FinalProject::Cleanup()
{
	std::cout << "Cleanup" << std::endl;

}

void FinalProject::InitializeModel()
{
	std::cout << "InitializeModel" << std::endl;

}

void FinalProject::InitializeCamera()
{
	std::cout << "InitializeCamera" << std::endl;

}

void FinalProject::InitializeLights()
{
	std::cout << "InitializeLights" << std::endl;

}

void FinalProject::RenderGUI()
{
	std::cout << "RenderGUI" << std::endl;

}

void FinalProject::UpdateCamera()
{
	std::cout << "UpdateCamera" << std::endl;
}
