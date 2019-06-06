#pragma once
#include <vector>
#include <string>
class Texture;
class OBJ;
class Material;
class Vao;

class GraphicsManager
{
	typedef unsigned int GLuint;
public:
	GraphicsManager();
	~GraphicsManager();
	static bool LoadOBJs(const char * path);
	static bool SaveToOBJ(OBJ *objMesh);
	static Vao* LoadOBJToVAO(OBJ* object, Vao* vao);
	static void LoadAllOBJsToVAOs();
	static bool LoadTextures(const char * path);
	static bool LoadMaterials(const char * path);
	static bool LoadShaders(const char * path);
	static bool ReloadShaders();
	static bool ReloadShader(const char *name);
	static void LoadUniforms(GLuint programID);
	static Texture* LoadBMP(const char *imagepath);
	static Texture* LoadDDS(const char *imagepath);
	static unsigned char * LoadImage(const char* path, int* x, int* y, int* numOfElements, int forcedNumOfEle);
	static float * LoadHDRImage(const char* path, int* x, int* y, int* numOfElements, int forcedNumOfEle);
	static GLuint LoadProgram(const char * vertex_file_path, const char * fragment_file_path, const char* geometry_file_path = nullptr);
    static Texture* CreateTexture(int width, int height, bool isDepth, unsigned char* data);
	static void LoadAllAssets();
	static Texture* LoadTexture(char* path);
	static Texture* GenerateCubeMap(const std::vector<std::string>& textures);
	static Texture* LoadHDRMap(char* path);
	static bool LoadCubeMaps(const char* path);
private:
};