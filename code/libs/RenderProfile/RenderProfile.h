#pragma once
#include "RenderElement.h"

class Script;

class RenderProfile : public RenderElement
{
public:
	RenderProfile();
	~RenderProfile();
	
	void SetUp();

	void LoadLuaFile(const char * filename);
	void Execute();
	Script* script;
	std::string path;
private:
};