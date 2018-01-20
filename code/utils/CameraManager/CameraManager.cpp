#include "CameraManager.h"
#include "Camera.h"
#include "Time.h"

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

void CameraManager::Update()
{
	currentCamera->Update(Time::deltaTime);
	ViewProjection = currentCamera->ViewMatrix*currentCamera->ProjectionMatrix;
	up = Vector3(currentCamera->ViewMatrix[0][1], currentCamera->ViewMatrix[1][1], currentCamera->ViewMatrix[2][1]);
	right = Vector3(currentCamera->ViewMatrix[0][0], currentCamera->ViewMatrix[1][0], currentCamera->ViewMatrix[2][0]);
}
