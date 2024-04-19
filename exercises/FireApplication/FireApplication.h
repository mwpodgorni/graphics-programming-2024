#pragma once

#include <ituGL/utils/DearImGui.h>
#include <ituGL/camera/CameraController.h>
#include <ituGL/renderer/Renderer.h>
#include <ituGL/scene/Scene.h>
#include <ituGL/application/Application.h>

class TextureCubemapObject;
class Material;
class FireApplication : public Application
{
public:
	FireApplication();

protected:
	void Initialize() override;
	void Update() override;
	void Render() override;
	void Cleanup() override;
private:
	void InitializeCamera();
	void InitializeLights();
	void InitializeMaterials();
	void InitializeModels();
	void InitializeRenderer();
	void RenderGUI();
private:
	// Helper object for debug GUI
	DearImGui m_imGui;

	// Camera controller
	CameraController m_cameraController;
	// Global scene
	Scene m_scene;
	// Renderer
	Renderer m_renderer;
	// Skybox texture
	std::shared_ptr<TextureCubemapObject> m_skyboxTexture;
	// Default material
	std::shared_ptr<Material> m_defaultMaterial;
};
