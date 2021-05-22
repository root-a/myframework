#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

class Texture;
class OBJ;
class Material;
class Vao;
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
	static bool LoadOBJs(const char* path);
	static bool LoadOBJs(std::unordered_map<std::string, std::string>& meshes);
	static void LoadOBJ(std::unordered_map<std::string, OBJ*>* objs, std::string path);
	static bool SaveToOBJ(OBJ* objMesh);
	static Vao* LoadOBJToVAO(OBJ* object, Vao* vao);
	static void LoadAllOBJsToVAOs();
	static bool LoadTextures(const char* path);
	static void LoadTextureInfo(std::unordered_map<std::string, TextureInfo*>* texturesToLoad, std::string path, int forcedNumOfEle);
	static void LoadTexturesIntoGPU(std::unordered_map<std::string, TextureInfo*>& texturesToLoad);
	static Texture* LoadTextureIntoGPU(const char* fileName, TextureInfo* ti);
	static bool LoadShaders(const char* path);
	static std::unordered_map<std::string, std::string> LoadShadersFiles(const char* path);
	static std::unordered_map<std::string, ShaderPaths> LoadShadersPaths(std::unordered_map<std::string, std::string>& shaders);
	static ShaderPaths LoadShaderPaths(std::string& path);
	static bool ReloadShaders();
	static bool ReloadShader(const char* name);
	static bool ReloadShaderFromPath(const char* name, ShaderPaths& paths);
	static bool ReloadShaderCode(std::string& shaderCode, std::unordered_set<std::string>& shaderIncludes);
	static bool LoadShaderIncludes(std::string&shaderCode, std::unordered_set<std::string>& shaderIncludes);
	static void LoadBlocks(Shader* shader, BlockType& type);
	static std::string ReadTextFileIntoString(const char* path);
	static std::string ReadTextFileIntoString(std::string& path);
	static void WriteStringToTextFile(std::string& content, std::string& path);
	static void AppendStringToTextFile(std::string& content, std::string& path);
	static void RemoveLineFromTextFileContainingText(std::string& content, std::string& path);
	static void ParseShaderForUniformBuffers(std::string& shaderCode, Shader& shader);
	static void ParseShaderForOutputs(std::string& shaderCode, Shader& shader);
	static void RemoveComments(std::string& shaderCode);
	static TextureInfo* LoadImage(const char* path, int forcedNumOfEle);
	static GLuint LoadProgram(std::string& vertex, std::string& fragment = std::string(), std::string& geometry = std::string());
	static Texture* LoadTexture(char* path);
	static Texture* LoadCompressedDDS(Texture* tex, TextureInfo* ti, int index = 0);
	static Texture* LoadCompressedDDSCubeMap(Texture* tex, TextureInfo* ti);
	static Texture* LoadCompressedDDSCubeMapFace(Texture* tex, TextureInfo* ti, int index = 0);
	static Texture* LoadCubeMap(const std::vector<std::string>& textures);
	static bool DumpTexture(FrameBuffer* fbo, Texture* texture);
	static bool DumpTextureSTB(FrameBuffer* fbo, Texture* texture);
	static bool LoadCubeMaps(const char* path);
	static void GetFileNames(std::vector<std::string>& out, const char* directory, const char* extension = "*");
private:
};