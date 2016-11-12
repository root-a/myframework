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
	currentCamera->Update((float)Time::timeStep);
	Matrix4 View = currentCamera->getViewMatrix();
	ViewProjection = View*currentCamera->ProjectionMatrix;
	up = Vector3(View[0][1], View[1][1], View[2][1]);
	right = Vector3(View[0][0], View[1][0], View[2][0]);
}
