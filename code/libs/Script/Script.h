#pragma once
#include <string>

struct lua_State;

class Script
{
public:
	Script();
	~Script();
	void LoadLuaFile(const char * filename);
	void Call(const char* functionName = "run");
	void Reload();
	void Unload();
	std::string path;
	std::string name;
private:
	lua_State *L;
};