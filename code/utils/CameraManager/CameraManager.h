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
	glm::mat4 ViewProjection;
	glm::mat4 ViewProjectionF;
	glm::mat4 ViewF;
	glm::mat4 ProjectionF;
	glm::vec2 screenSize;
	glm::vec3 cameraPos;
	glm::vec3 cameraUp;
	glm::vec3 cameraRight;
	glm::vec3 cameraForward;
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