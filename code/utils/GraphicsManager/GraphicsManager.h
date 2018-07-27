#include <vector>
#include <string>
class Mesh;
class Texture;
class OBJ;
class Material;

class GraphicsManager
{
	typedef unsigned int GLuint;
public:
	GraphicsManager();
	~GraphicsManager();
	static bool LoadOBJs(const char * path);
	static bool SaveToOBJ(OBJ *objMesh);
	static Mesh* LoadOBJToVBO(OBJ* object, Mesh* mesh);
	static void LoadAllOBJsToVBO();
	static bool LoadTextures(const char * path);
	static bool LoadMaterials(const char * path);
	static Texture* LoadBMP(const char *imagepath);
	static Texture* LoadDDS(const char *imagepath);
	static unsigned char * LoadImage(const char* path, int* x, int* y, int* numOfElements, int forcedNumOfEle);
	static GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path, const char* geometry_file_path = nullptr);
    static Texture* CreateTexture(int width, int height, bool isDepth, unsigned char* data);
	static void LoadAllAssets();
	static Texture* LoadTexture(char* path);
	static Texture* GenerateCubeMap(const std::vector<std::string>& textures);
	static bool LoadCubeMaps(const char* path);
private:
};