#pragma once
#include <map>
#include <vector>
#include "Vector3.h"
#include "Vector3F.h"
class Object;
class Scene
{
public:
	enum axis
	{
		x,
		y,
		z
	};

    static Scene* Instance();
    unsigned int idCounter;
    Object* addChildTo(Object* parentNodeGR);
	std::map<unsigned int, Object*> pickingList; //render/picking list
	std::vector<Object*> pointLights; //render list
	std::vector<Object*> directionalLights; //render list
    Object* SceneObject; //scenegraph root

	void addRandomObject(const mwm::Vector3& pos = mwm::Vector3());
	Object* addObjectToScene(const char* name = "cube", const mwm::Vector3& pos = mwm::Vector3());
	Object* addObjectTo(Object* parent, const char* name = "cube", const mwm::Vector3& pos = mwm::Vector3());
	Object* addPhysicObject(const char* name = "cube", const mwm::Vector3& pos = mwm::Vector3());
	void addRandomObjects(int num, int min = -20, int max = 20);
	void addRandomlyObjects(const char* name, int num, int min = -20, int max = 20);
	Object* addPointLight(const mwm::Vector3& position, const mwm::Vector3F& color = mwm::Vector3F(1.f, 1.f, 1.f));
	void addRandomPointLight(int min = -20, int max = 20);
	Object* addDirectionalLight(const mwm::Vector3F& direction, const mwm::Vector3F& color = mwm::Vector3F(1.f, 1.f, 1.f));
	void addRandomlyPointLights(int num, int min = -20, int max = 20);
	void addRandomlyPhysicObjects(const char* name, int num, int min = -20, int max = 20);
	mwm::Vector3 generateRandomIntervallVectorCubic(int min, int max);
	mwm::Vector3 generateRandomIntervallVectorFlat(int min, int max, axis axis = x, int axisHeight = 0);
	mwm::Vector3 generateRandomIntervallVectorSpherical(int min, int max);
	void Update();
	void Clear();
private:
    Scene();
    ~Scene();
    //copy
    Scene(const Scene&);
    //assign
    Scene& operator=(const Scene&);
};