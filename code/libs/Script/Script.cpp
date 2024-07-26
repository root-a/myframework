#include "Script.h"
//#include <luajit.hpp>
extern "C" {
	//#include "include/lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "luajit.h"
//#include "include/luaconf.h"
}
#include "LuaTools.h"

Script::Script()
{
	L = nullptr;
	//L = luaL_newstate();

	//luaL_openlibs(L);
}

Script::Script(const char* filename)
{
	L = nullptr;
	LoadLuaFile(filename);
}

Script::~Script()
{
	if (L != nullptr) lua_close(L);
}

void Script::LoadLuaFile(const char * path)
{
	Unload();
	this->path = path;
	this->name = path;
	const size_t last_slash_idx = this->name.find_last_of("\\/");
	if (std::string::npos != last_slash_idx)
	{
		this->name.erase(0, last_slash_idx + 1);
	}

	const size_t period_idx = this->name.rfind('.');
	if (std::string::npos != period_idx)
	{
		this->name.erase(period_idx);
	}

	L = luaL_newstate();

	luaL_openlibs(L);

	LuaTools::dofile(L, path);
}

bool Script::GetFunction(const char* functionName)
{
	if (L != nullptr)
	{
		lua_getglobal(L, functionName);
		if (lua_isfunction(L, -1))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}

void Script::Call(const char* functionName, int nrOfArgs, int clear)
{
	if (L != nullptr)
	{
		lua_getglobal(L, functionName);
		if (lua_isfunction(L, -1))
		{
			LuaTools::docall(L, nrOfArgs, clear);
		}
		else
		{
			lua_pop(L, 1);
		}
	}
}

void Script::Call(const char* functionName, void* lightUserData)
{
	if (L != nullptr)
	{
		lua_getglobal(L, functionName);
		if (lua_isfunction(L, -1))
		{
			lua_pushlightuserdata(L, lightUserData);
			LuaTools::docall(L, 1, 1);
		}
		else
		{
			lua_pop(L, 1);
		}
	}
}

void Script::Reload()
{
	if (L == nullptr)
	{
		return;
	}
	lua_close(L);

	L = luaL_newstate();

	luaL_openlibs(L);
	std::string scriptPath = path;
	LoadLuaFile(scriptPath.c_str());
}

void Script::Unload()
{
	if (L != nullptr)
	{
		lua_close(L);
		L = nullptr;
		name.clear();
		path.clear();
	}
}
