#include "CameraManager.h"
#include "Camera.h"

using namespace mwm;

CameraManager::CameraManager()
{
	currentCamera = nullptr;
}

CameraManager::~CameraManager()
{
}

CameraManager* CameraManager::Instance()
{
	static CameraManager instance;

	return &instance;
}

Camera* CameraManager::GetCurrentCamera()
{
	return currentCamera;
}

void CameraManager::AddCamera(const char* name, Camera* cam)
{
	cameras[name] = cam;
}

void CameraManager::SetCurrentCamera(const char* name)
{
	if (cameras.find(name) != cameras.end()) currentCamera = cameras[name];
}

void CameraManager::Update(double deltaTime)
{
	currentCamera->Update(deltaTime);
	ViewProjection = currentCamera->ViewMatrix*currentCamera->ProjectionMatrix;
}
