#include "ScriptsComponent.h"
#include "Script.h"
#include <GraphicsStorage.h>

ScriptsComponent::ScriptsComponent()
{
}

ScriptsComponent::~ScriptsComponent()
{
}

void ScriptsComponent::AddScript(const char* filename)
{
	if (strcmp(filename, "") == 0)
	{
		Script* newScript = GraphicsStorage::assetRegistry.AllocAsset<Script>();
		scripts.push_back(newScript);
		return;
	}
	for (auto script : scripts)
	{
		if (script->path.compare(filename) == 0)
		{
			return;
		}
	}
	Script* newScript = GraphicsStorage::assetRegistry.AllocAsset<Script>(filename);
	scripts.push_back(newScript);
}

void ScriptsComponent::RemoveScript(Script* script)
{
	int index = -1;
	for (int i = 0; i < scripts.size(); i++)
	{
		if (scripts[i] == script)
		{
			index = i;
			break;
		}
	}
	if (index != -1)
	{
		scripts.erase(scripts.begin() + index);
	}
}

void ScriptsComponent::Start()
{
	for (auto& script : scripts)
	{
		script->Call("init", object);
	}
}

void ScriptsComponent::Update()
{
	for (auto& script : scripts)
	{
		script->Call("run", object);
	}
}

Component* ScriptsComponent::Clone()
{
	return new ScriptsComponent(*this);
}
