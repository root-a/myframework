#pragma once
#include "Component.h"
#include <vector>

class Script;

class ScriptsComponent : public Component
{
public:
	ScriptsComponent();
	~ScriptsComponent();
	void AddScript(const char * filename);
	void RemoveScript(Script* filename);
	void Start();
	void Update();
	Component* Clone();
	std::vector<Script*> scripts;
private:
};