#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "Shader.h"
class Vao;
class Texture2D;
class Material;
class OBJ;
class Texture;

class GraphicsStorage
{
	typedef unsigned int GLuint;
public:
	GraphicsStorage();
	~GraphicsStorage();
	static std::unordered_map<std::string,Vao*> vaos;
	static std::vector<Texture*> textures;
	static std::vector<Texture*> cubemaps;
	static std::vector<Material*> materials;
	static std::unordered_map<std::string,OBJ*> objs;
	static std::unordered_map<std::string, GLuint> shaderIDs;
	static std::unordered_map<std::string, ShaderPaths> shaderPaths;
	static std::unordered_map<std::string, Shader*> shaders;
	static void ClearMaterials();
	static void ClearVaos();
	static void ClearTextures();
	static void ClearCubemaps();
	static void ClearOBJs();
	static void ClearShaders();
	static void Clear();
private:

};

