#pragma once
#include "MyMathLib.h"
#include <map>
#include <string>
class Camera;

class CameraManager
{
public:
	static CameraManager* Instance();
	void AddCamera(const char* name, Camera* cam);
	Camera* GetCurrentCamera();
	void SetCurrentCamera(const char* name);
	void Update(double deltaTime);
	Matrix4 ViewProjection;
	Matrix4F ViewProjectionF;
	Matrix4F ViewF;
	Matrix4F ProjectionF;
	Vector2F screenSize;
	Vector3F cameraPos;
	Vector3F cameraUp;
	Vector3F cameraRight;
	Vector3F cameraForward;
	float far;
	float near;
private:
	
	CameraManager();
	~CameraManager();
	//copy
	CameraManager(const CameraManager&);
	//assign
	CameraManager& operator=(const CameraManager&);
	std::map<std::string, Camera*> cameras;
	Camera* currentCamera;
}; 