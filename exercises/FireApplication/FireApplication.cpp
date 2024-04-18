#include "FireApplication.h"

#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/asset/ModelLoader.h>
#include <ituGL/asset/Texture2DLoader.h>
#include <ituGL/shader/Material.h>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <imgui.h>
#include <ituGL/asset/TextureCubemapLoader.h>
#include <iostream>
#include <fstream>
#include <sstream>


FireApplication::FireApplication()
	: Application(1024, 1024, "Fire demo")
{
	std::cout << "constructor" << std::endl;
}

void FireApplication::Initialize()
{
	std::cout << "initialize" << std::endl;
	Application::Initialize();
}

void FireApplication::Update()
{
	Application::Update();
}

void FireApplication::Render()
{
	Application::Render();
}
void FireApplication::Cleanup()
{
	// Cleanup DearImGUI
	//m_imGui.Cleanup();

	Application::Cleanup();
}


