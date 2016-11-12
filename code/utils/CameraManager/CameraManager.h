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
	void Update();
	mwm::Matrix4 ViewProjection;
	mwm::Vector3 up;
	mwm::Vector3 right;
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