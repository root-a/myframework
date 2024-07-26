#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <thread>
class Texture;
class OBJ;
class Material;
class VertexArray;
class Shader;
struct ShaderPaths;
class FrameBuffer;
struct TextureInfo;
enum class BlockType;

class GraphicsManager
{
	typedef unsigned int GLuint;
public:
	GraphicsManager();
	~GraphicsManager();
	static void LoadPaths(const char* path);
	static void LoadAllAssets();
	static bool LoadOBJs(const char* path, std::vector<OBJ*>& parsedOBJs);
	static bool LoadOBJs(std::vector<std::string>& meshPaths, std::vector<OBJ*>& parsedOBJs);
	static void LoadOBJ(std::vector<OBJ*>* objs, std::string path);
	static void LoadOBJsToVAOs(std::vector<OBJ*>& parsedOBJs);
	static bool SaveToOBJ(OBJ* objMesh);
	static VertexArray* LoadOBJToVAO(OBJ* object, VertexArray* vao);
	//static void LoadAllOBJsToVAOs();
	static bool LoadTextures(const char* path);
	static void LoadTextureInfo(std::unordered_map<std::string, TextureInfo*>* texturesToLoad, std::string path, int forcedNumOfEle);
	static void LoadTexturesIntoGPU(std::unordered_map<std::string, TextureInfo*>& texturesToLoad);
	static Texture* LoadTextureIntoGPU(const char* guid, const char* fileName, TextureInfo* ti);
	static bool LoadShaders(const char* path);
	static Shader* LoadShader(const char* guid, const char* path);
	static std::vector<std::string> LoadShadersFiles(const char* path);
	static std::unordered_map<std::string, ShaderPaths> LoadShadersPaths(const std::vector<std::string>& shaders);
	static ShaderPaths LoadShaderPaths(const std::string& path);
	static bool ReloadShaders();
	static Shader* ReloadShader(Shader* shader);
	static Shader* ReloadShaderFromPath(const char* name, const ShaderPaths& paths);
	static bool ReloadShaderCode(std::string& shaderCode, std::unordered_set<std::string>& shaderIncludes);
	static bool LoadShaderIncludes(std::string&shaderCode, std::unordered_set<std::string>& shaderIncludes);
	static void LoadBlocks(Shader* shader, BlockType type);
	static void LoadOutputs(Shader* shader);
	static void LoadSamplers(Shader* shader);
	static void LoadAttributes(Shader* shader);
	static std::string LoadShaderBlockConfig(const char* path);
	static std::string ReadTextFileIntoString(const char* path);
	static std::string ReadTextFileIntoString(const std::string& path);
	static void WriteStringToTextFile(std::string& content, std::string& path);
	static void AppendStringToTextFile(std::string& content, std::string& path);
	static void RemoveLineFromTextFileContainingText(std::string& content, std::string& path);
	static void RemoveComments(std::string& shaderCode);
	static TextureInfo* LoadImage(const char* path, int forcedNumOfEle);
	static GLuint LoadProgram(const std::string& vertex, const std::string& fragment = std::string(), const std::string& geometry = std::string());
	static Texture* LoadTexture(const char* guid, const char* path);
	static Texture* LoadCompressedDDS(Texture* tex, TextureInfo* ti, int index = 0);
	static Texture* LoadCompressedDDSCubeMap(Texture* tex, TextureInfo* ti);
	static Texture* LoadCompressedDDSCubeMapFace(Texture* tex, TextureInfo* ti, int index = 0);
	static Texture* LoadCubeMap(const char* guid, const char* path);
	static bool DumpTexture(FrameBuffer* fbo, Texture* texture);
	static bool DumpTextureSTB(FrameBuffer* fbo, Texture* texture);
	static bool LoadCubeMaps(const char* path);
	static void GetFileNames(std::vector<std::string>& out, const char* directory, const char* extension = ".*");
private:
};