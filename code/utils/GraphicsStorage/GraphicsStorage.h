#pragma once
#include <vector>
#include <unordered_map>
#include <string>

class Mesh;
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
	static std::unordered_map<std::string,Mesh*> meshes; //get mesh by name
	static std::vector<Texture*> textures;
	static std::vector<Texture*> cubemaps;
	static std::vector<Material*> materials;
	static std::vector<OBJ*> objects;
	static std::unordered_map<std::string, GLuint> shaderIDs;
	static void ClearMaterials();
	static void ClearMeshes();
	static void ClearTextures();
	static void ClearCubemaps();
	static void ClearOBJs();
	static void ClearShaders();
private:

};

