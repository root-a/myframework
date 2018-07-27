#pragma once
#include <unordered_map>
#include <vector>
#include "Vector3.h"
#include "Vector3F.h"

class Object;
class DirectionalLight;
class SpotLight;
class PointLight;

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
	Object* addChild();
    Object* addChildTo(Object* parentNodeGR); //minimal function to register object for updates of scenegraph and generates unique id, best for custom objects
	void registerForPicking(Object* object); //helper function for registering objects for picking
	void unregisterForPicking(Object* object);
	std::unordered_map<unsigned int, Object*> pickingList; //picking list, only for look-ups
	std::vector<Object*> renderList; //render list
	std::vector<Object*> pointLights; //render list
	std::vector<Object*> spotLights; //render list
	std::vector<Object*> directionalLights; //render list
	std::vector<DirectionalLight*> directionalLightComponents; //render list
	std::vector<SpotLight*> spotLightComponents; //render list
	std::vector<PointLight*> pointLightComponents; //render list
    Object* SceneObject; //scenegraph root

	void addRandomObject(const mwm::Vector3& pos = mwm::Vector3()); //adds random object to the scene
	
	Object* addObject(const char* name = "cube", const mwm::Vector3& pos = mwm::Vector3());
	Object* addObjectTo(Object* parent, const char* name = "cube", const mwm::Vector3& pos = mwm::Vector3());
	Object* addPhysicObject(const char* name = "cube", const mwm::Vector3& pos = mwm::Vector3());
	void addRandomObjects(int num, int min = -20, int max = 20);
	void addRandomlyObjects(const char* name, int num, int min = -20, int max = 20);
	Object* addPointLight(bool castShadow = false, const mwm::Vector3& position = mwm::Vector3(), const mwm::Vector3F& color = mwm::Vector3F(1.f, 1.f, 1.f));
	Object* addPointLightTo(Object* parent, bool castShadow = false, const mwm::Vector3& position = mwm::Vector3(), const mwm::Vector3F& color = mwm::Vector3F(1.f, 1.f, 1.f));
	Object* addSpotLight(bool castShadow = false, const mwm::Vector3& position = mwm::Vector3(), const mwm::Vector3F& color = mwm::Vector3F(1.f, 1.f, 1.f));
	Object* addSpotLightTo(Object* parent, bool castShadow = false, const mwm::Vector3& position = mwm::Vector3(), const mwm::Vector3F& color = mwm::Vector3F(1.f, 1.f, 1.f));
	void addRandomPointLight(int min = -20, int max = 20);
	Object* addDirectionalLight(bool castShadow = false, const mwm::Vector3F& color = mwm::Vector3F(1.f, 1.f, 1.f));
	Object* addDirectionalLightTo(Object* parent, bool castShadow = false, const mwm::Vector3F& color = mwm::Vector3F(1.f, 1.f, 1.f));
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