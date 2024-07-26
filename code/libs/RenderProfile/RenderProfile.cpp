#include "RenderProfile.h"
#include "Script.h"

RenderProfile::RenderProfile()
{
	script = new Script();
}

RenderProfile::~RenderProfile()
{
	delete script;
}

void RenderProfile::SetUp()
{
	script->Call();
}

void RenderProfile::LoadLuaFile(const char * filename)
{
	std::string directorywithfilename = "resources\\render_profiles\\";
	directorywithfilename.append(filename);
	directorywithfilename.append(".lua");

	script->LoadLuaFile(directorywithfilename.c_str());
}

void RenderProfile::Execute()
{
	SetUp();
}
