#pragma once
#include <string>

struct lua_State;

class Script
{
public:
	Script();
	Script(const char* filename);
	~Script();
	void LoadLuaFile(const char * filename);
	bool GetFunction(const char* functionName);
	void Call(const char* functionName = "run", int nrOfArgs = 0, int clear = 1);
	void Call(const char* functionName, void* lightUserData);
	void Reload();
	void Unload();
	std::string path;
	std::string name;
	lua_State *L;
private:
};