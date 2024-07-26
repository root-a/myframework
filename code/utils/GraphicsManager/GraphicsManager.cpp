#define _CRT_SECURE_NO_DEPRECATE
#include "GraphicsManager.h"
#include "GraphicsStorage.h"
#include "OBJ.h"
#include "Vao.h"
#include "Ebo.h"
#include "Material.h"
#include "Texture.h"
#include "RenderBuffer.h"
#include "RenderPass.h"
#include "RenderProfile.h"
#include "MaterialProfile.h"
#include "TextureProfile.h"
#include "ScriptsComponent.h"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>
#include <algorithm>
#include "Shader.h"
#include "CPUBlockData.h"
#include "ShaderBlock.h"
#include "FrameBuffer.h"
#include "ShaderBlockData.h"
#include "Script.h"
#include <GL/glew.h>
#include <mutex>
#include <iosfwd>
#include "SOIL2.h"
#include "stb_image.h"
#include <filesystem>

extern "C" {
	//#include "include/lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "luajit.h"
//#include "include/luaconf.h"
}
#include "LuaTools.h"

#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

static std::mutex objLoadMutex;
static std::mutex tiLoadMutex;

std::string str_tolower(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return std::tolower(c); }
	);
	return s;
}

void splitLine(const std::string& str, std::vector<std::string>& cont, const std::string& delims = " ")
{
	std::size_t current, previous = 0;
	current = str.find_first_of(delims);
	while (current != std::string::npos) {
		cont.push_back(str.substr(previous, current - previous));
		previous = current + 1;
		current = str.find_first_of(delims, previous);
	}
	if (previous != str.size())
	{
		cont.push_back(str.substr(previous, str.size() - previous));
	}
}

void replaceAllWords(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
}

void replaceAllCharacters(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find_first_of(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, 1, to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
}

size_t findFirstOfWords(std::string& str, std::vector<std::string>& words, int start_pos)
{
	size_t result = 0;
	for (size_t i = 0; i < words.size(); i++)
	{
		if ((result = str.find(words[i], start_pos)) != std::string::npos) return result;
	}
	return std::string::npos;
}

GraphicsManager::GraphicsManager()
{
}

GraphicsManager::~GraphicsManager()
{
}

void GraphicsManager::LoadPaths(const char* path)
{
	FILE* file;
	file = fopen(path, "r");
	if (file == NULL) {
		printf("%s could not be opened.\n", path);
		return;
	}
	char line[128];
	while (fgets(line, sizeof(line), file)) {
		if (line[0] == '/' || line[0] == ';' || line[0] == '#') continue; // ignore comment line
		char folder[128];
		char folderPath[128];
		int matches = sscanf(line, "%s %s", folder, folderPath);
		if (matches != 2)
		{
			printf("Wrong folder - path information!\n");
		}
		else {
			GraphicsStorage::paths[folder] = folderPath;
		}
	}
	fclose(file);
}

bool GraphicsManager::LoadOBJs(const char* path, std::vector<OBJ*>& parsedOBJs)
{
	FILE* file;
	file = fopen(path, "r");
	if (file == NULL) {
		printf("%s could not be opened.\n", path);
		return false;
	}
	std::vector<std::string> meshPaths;
	while (1)
	{
		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
		{
			break; // EOF = End Of File. Quit the loop.
		}
		meshPaths.push_back(GraphicsStorage::paths["resources"] + lineHeader);
	}
	fclose(file);
	LoadOBJs(meshPaths, parsedOBJs);
	return true;
}

bool GraphicsManager::LoadOBJs(std::vector<std::string>& meshPaths, std::vector<OBJ*>& parsedOBJs)
{
	std::vector<std::thread> threadPool;
	for (auto& path : meshPaths)
	{
		threadPool.push_back(std::thread(LoadOBJ, &parsedOBJs, path));
	}
	for (auto& th : threadPool)
	{
		th.join();
	}
	return true;
}

void GraphicsManager::LoadOBJ(std::vector<OBJ*>* objs, std::string path)
{
	OBJ* tempOBJ = GraphicsStorage::assetRegistry.AllocAsset<OBJ>();
	bool res = tempOBJ->LoadAndIndexOBJ(path.c_str());
	if (res)
	{
		std::filesystem::path objPath(path);
		tempOBJ->name = objPath.stem().string();
		
		std::scoped_lock<std::mutex> lock(objLoadMutex);
		(*objs).push_back(tempOBJ);
	}
}

VertexArray* GraphicsManager::LoadOBJToVAO(OBJ* object, VertexArray* vao)
{
	vao->AddVertexBuffer(GraphicsStorage::assetRegistry.AllocAsset<VertexBuffer>((const void*)&object->indexed_vertices[0], (unsigned int)object->indexed_vertices.size(), BufferLayout({ {ShaderDataType::Type::Float3, "position"} })));
	vao->AddVertexBuffer(GraphicsStorage::assetRegistry.AllocAsset<VertexBuffer>((const void*)&object->indexed_uvs[0], (unsigned int)object->indexed_uvs.size(), BufferLayout({ {ShaderDataType::Type::Float2, "uv"} })));
	vao->AddVertexBuffer(GraphicsStorage::assetRegistry.AllocAsset<VertexBuffer>((const void*)&object->indexed_normals[0], (unsigned int)object->indexed_normals.size(), BufferLayout({ {ShaderDataType::Type::Float3, "normal"} })));
	if (object->indexed_tangents.size() > 0)
	{
		vao->AddVertexBuffer(GraphicsStorage::assetRegistry.AllocAsset<VertexBuffer>((const void*)&object->indexed_tangents[0], (unsigned int)object->indexed_tangents.size(), BufferLayout({ {ShaderDataType::Type::Float3, "tangent"} })));
		vao->AddVertexBuffer(GraphicsStorage::assetRegistry.AllocAsset<VertexBuffer>((const void*)&object->indexed_bitangents[0], (unsigned int)object->indexed_bitangents.size(), BufferLayout({ {ShaderDataType::Type::Float3, "bitangent"} })));
	}
	vao->AddElementBuffer(GraphicsStorage::assetRegistry.AllocAsset<ElementBuffer>(object->GetIndicesData(), object->indicesCount));
	vao->center = object->center_of_mesh;
	vao->dimensions = object->dimensions;
	return vao;
}

void GraphicsManager::LoadOBJsToVAOs(std::vector<OBJ*>& parsedOBJs)
{
	for (auto& obj : parsedOBJs)
	{
		VertexArray* newVao = GraphicsStorage::assetRegistry.AllocAsset<VertexArray>();
		newVao->name = obj->name;
		LoadOBJToVAO(obj, newVao);
	}
}

bool GraphicsManager::SaveToOBJ(OBJ* obj)
{
	FILE* file;
	file = fopen("savedFile.obj", "w");
	for (size_t i = 0; i < obj->indexed_vertices.size(); i++)
	{
		std::ostringstream ss;
		ss.precision(5);
		ss << std::fixed;
		ss << "v ";
		ss << obj->indexed_vertices.at(i)[0];
		ss << " ";
		ss << obj->indexed_vertices.at(i)[1];
		ss << " ";
		ss << obj->indexed_vertices.at(i)[2];
		ss << "\n";
		std::string s(ss.str());
		fputs(s.c_str(), file);
	}

	for (size_t i = 0; i < obj->indexed_uvs.size(); i++)
	{
		std::ostringstream ss;
		ss.precision(5);
		ss << std::fixed;
		ss << "vt ";
		ss << obj->indexed_uvs.at(i)[0];
		ss << " ";
		ss << obj->indexed_uvs.at(i)[1];
		ss << " ";
		ss << obj->indexed_uvs.at(i)[2];
		ss << "\n";
		std::string s(ss.str());
		fputs(s.c_str(), file);
	}

	for (size_t i = 0; i < obj->indexed_normals.size(); i++)
	{
		std::ostringstream ss;
		ss.precision(5);
		ss << std::fixed;
		ss << "vn ";
		ss << obj->indexed_normals.at(i)[0];
		ss << " ";
		ss << obj->indexed_normals.at(i)[1];
		ss << " ";
		ss << obj->indexed_normals.at(i)[2];
		ss << "\n";
		std::string s(ss.str());
		fputs(s.c_str(), file);
	}

	for (size_t i = 0; i < obj->indices.size(); i += 3)
	{
		std::string faceRow = "f " + std::to_string(obj->indices.at(i) + 1) + "/" + std::to_string(obj->indices.at(i) + 1) + "/" + std::to_string(obj->indices.at(i) + 1) + " ";
		faceRow = faceRow + std::to_string(obj->indices.at(i + 1) + 1) + "/" + std::to_string(obj->indices.at(i + 1) + 1) + "/" + std::to_string(obj->indices.at(i + 1) + 1) + " ";
		faceRow = faceRow + std::to_string(obj->indices.at(i + 2) + 1) + "/" + std::to_string(obj->indices.at(i + 2) + 1) + "/" + std::to_string(obj->indices.at(i + 2) + 1) + "\n";
		fputs(faceRow.c_str(), file);
	}
	
	fclose(file);

	return true;
}

bool GraphicsManager::LoadTextures(const char* path)
{
	FILE* file;
	file = fopen(path, "r");
	if (file == NULL) {
		printf("%s could not be opened.\n", path);
		return false;
	}
	//stbi_set_flip_vertically_on_load(true);
	std::vector<std::thread> threadPool;
	while (1) {

		char texturePath[128];
		//meshID not in use currently
		// read the first word of the line
		int res = fscanf(file, "%s", texturePath);
		if (res == EOF)
		{
			break; // EOF = End Of File. Quit the loop.
		}
		
		printf("Loading texture: %s\n", texturePath);
		//threadPool.push_back(std::thread(LoadTextureInfo, &GraphicsStorage::texturesToLoad, GraphicsStorage::paths["resources"] + texturePath, 0));
		LoadTextureInfo(&GraphicsStorage::texturesToLoad, GraphicsStorage::paths["resources"] + texturePath, 0);
	}
	for (auto& th : threadPool)
	{
		th.join();
	}
	fclose(file);

	LoadTexturesIntoGPU(GraphicsStorage::texturesToLoad);
	return true;
}

void GraphicsManager::LoadTextureInfo(std::unordered_map<std::string, TextureInfo*>* texturesToLoad, std::string path, int forcedNumOfEle)
{
	TextureInfo* ti = SOIL_load_image(path.c_str(), forcedNumOfEle);
	std::filesystem::path texturePath(path);
	std::scoped_lock<std::mutex> lock(tiLoadMutex);
	texturesToLoad->insert({ texturePath.string() , ti});
	printf("Finished loading texture: %s\n", path.c_str());
}

void GraphicsManager::LoadTexturesIntoGPU(std::unordered_map<std::string, TextureInfo*>& texturesToLoad)
{
	for (auto& fti : texturesToLoad)
	{
		LoadTextureIntoGPU(nullptr, fti.first.c_str(), fti.second);
	}
	texturesToLoad.clear();
}

Texture* GraphicsManager::LoadTextureIntoGPU(const char* guid, const char* filePath, TextureInfo* ti)
{
	std::filesystem::path texturePath(filePath);
	std::string fileName = texturePath.stem().string();
	Texture* tex = nullptr;
	if (ti != nullptr)
	{
		DDSExtraInfo* ei = NULL;
		int internalFormat;
		int format;
		switch (ti->type)
		{
		case HDR:
			if (guid != nullptr)
			{
				tex = GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<Texture>(guid, GL_TEXTURE_2D, 0, GL_RGB16F, ti->width, ti->height, GL_RGB, GL_FLOAT, ti->data, GL_COLOR_ATTACHMENT0);
			}
			else
			{
				tex = GraphicsStorage::assetRegistry.AllocAsset<Texture>(GL_TEXTURE_2D, 0, GL_RGB16F, ti->width, ti->height, GL_RGB, GL_FLOAT, ti->data, GL_COLOR_ATTACHMENT0);
			}
			tex->GenerateBindSpecify();
			tex->name = fileName;
			tex->texturePath = filePath;
			break;
		case DDS:
			ei = (DDSExtraInfo*)ti->extraInfo;
			for (int i = 0; i < ei->nrOfCubeMapFaces; i++)
			{
				if (ei->compressed)
				{
					if (guid != nullptr && ei->nrOfCubeMapFaces == 1)
					{
						tex = GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<Texture>(guid, GL_TEXTURE_2D, 0, GL_RGB, ti->width, ti->height, GL_RGB, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
					}
					else
					{
						tex = GraphicsStorage::assetRegistry.AllocAsset<Texture>(GL_TEXTURE_2D, 0, GL_RGB, ti->width, ti->height, GL_RGB, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
					}
					LoadCompressedDDS(tex, ti, i);
					std::string name = fileName;
					if (i > 0) name = fileName + std::to_string(i);
					tex->name = name;
					tex->texturePath = filePath;
				}
				else
				{
					switch (ti->numOfElements)
					{
					case 1:
						internalFormat = GL_R8; format = GL_RED;
						break;
					case 2:
						internalFormat = GL_RG8; format = GL_RG;
						break;
					case 3:
						internalFormat = GL_RGB; format = GL_RGB;
						break;
					case 4:
						internalFormat = GL_RGBA; format = GL_RGBA;
						break;
					default:
						internalFormat = GL_RGB; format = GL_RGB;
						break;
					}
					if (guid != nullptr && ei->nrOfCubeMapFaces == 1)
					{
						tex = GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<Texture>(guid, GL_TEXTURE_2D, 0, internalFormat, ti->width, ti->height, format, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
					}
					else
					{
						tex = GraphicsStorage::assetRegistry.AllocAsset<Texture>(GL_TEXTURE_2D, 0, internalFormat, ti->width, ti->height, format, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
					}
					tex->pixels = &((unsigned char*)ti->data)[i * ti->height * ti->width * ti->numOfElements];
					tex->GenerateBindSpecify();
					std::string name = fileName;
					if (i > 0) name = fileName + std::to_string(i);
					tex->name = name;
					tex->texturePath = filePath;
				}
				if (!tex->hasMipMaps) tex->GenerateMipMaps(); // we should have it stored in the settings so that we don't generate it here!
			}
			break;
		default:
			switch (ti->numOfElements)
			{
			case 1:
				internalFormat = GL_R8; format = GL_RED;
				break;
			case 2:
				internalFormat = GL_RG8; format = GL_RG;
				break;
			case 3:
				internalFormat = GL_RGB; format = GL_RGB;
				break;
			case 4:
				internalFormat = GL_RGBA; format = GL_RGBA;
				break;
			default:
				internalFormat = GL_RGB; format = GL_RGB;
				break;
			}
			if (guid != nullptr)
			{
				tex = GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<Texture>(guid, GL_TEXTURE_2D, 0, internalFormat, ti->width, ti->height, format, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
			}
			else
			{
				tex = GraphicsStorage::assetRegistry.AllocAsset<Texture>(GL_TEXTURE_2D, 0, internalFormat, ti->width, ti->height, format, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
			}
			tex->GenerateBindSpecify();
			tex->GenerateMipMaps(); // we should have it stored in the settings so that we don't generate it here!
			tex->name = fileName;
			tex->texturePath = filePath;
			break;
		}
		tex->SetDefaultParameters();
		SOIL_free_texture_info(ti);
	}
	return tex;
}

bool GraphicsManager::LoadCubeMaps(const char* path)
{
	FILE* file;
	file = fopen(path, "r");
	if (file == NULL) {
		printf("%s could not be opened.\n", path);
		return false;
	}
	while (1) {
		char cubemapDirectory[128];
		// read the first line
		int res = fscanf(file, "%s", cubemapDirectory);
		if (res == EOF)
		{
			break; // EOF = End Of File. Quit the loop.
		}
		
		std::string texturesFilePath = GraphicsStorage::paths["resources"] + cubemapDirectory;
		LoadCubeMap(nullptr, texturesFilePath.c_str());
	}
	fclose(file);
	return true;
}

void GraphicsManager::GetFileNames(std::vector<std::string>& out, const char* directory, const char* extension)
{
	std::filesystem::path ext(extension);
	for (const auto& it : std::filesystem::directory_iterator(directory))
	{
		if (it.is_regular_file())
		{
			if (it.path().extension() == ext)
				out.push_back(it.path().stem().string());
		}
	}
}

void GraphicsManager::RemoveComments(std::string& shaderCode)
{
	while (true)
	{
		size_t nFPos = shaderCode.find("/*");
		if (nFPos + 1)
		{
			size_t second = shaderCode.find("*/", nFPos);
			shaderCode.erase(nFPos, (second + 1) - nFPos);
		}
		else
		{
			break;
		}
	}
	while (true)
	{
		size_t nFPos = shaderCode.find("//");
		if (nFPos + 1)
		{
			size_t second = shaderCode.find("\n", nFPos);
			shaderCode.erase(nFPos, second - nFPos);
		}
		else
		{
			break;
		}
	}
}

bool GraphicsManager::LoadShaders(const char* path)
{
	//GraphicsStorage::ClearShaders();
	auto shadersPaths = LoadShadersPaths(LoadShadersFiles(path));

	for (auto& paths : shadersPaths)
	{
		ReloadShaderFromPath(nullptr, paths.second);
	}

	return true;
}

Shader* GraphicsManager::LoadShader(const char* guid, const char* path)
{
	ShaderPaths sp = LoadShaderPaths(std::move(path));
	Shader* shader = ReloadShaderFromPath(guid, sp);
	return shader;
}

std::vector<std::string> GraphicsManager::LoadShadersFiles(const char* path)
{
	FILE* file;
	file = fopen(path, "r");
	if (file == NULL) {
		printf("%s could not be opened.\n", path);
		return std::vector<std::string>();
	}
	char line[128];
	std::vector<std::string> shaders;
	while (fgets(line, sizeof(line), file)) {
		if (line[0] == '/' || line[0] == ';' || line[0] == '#') continue; // ignore comment line
		//programName, the first word, we should remove them and clean up all configs
		char programName[128];
		char filePath[128];
		int matches = sscanf(line, "%s %s", programName, filePath);
		if (matches != 2)
		{
			printf("Wrong shader information!\n");
		}
		else {
			shaders.emplace_back(GraphicsStorage::paths["resources"] + filePath);
		}
	}
	fclose(file);
	return shaders;
}

std::unordered_map<std::string, ShaderPaths> GraphicsManager::LoadShadersPaths(const std::vector<std::string>& shaders)
{
	//we should go over directory and check if shader with name exists in shader list instead
	std::unordered_map<std::string, ShaderPaths> shadersPaths;
	for (auto& shader : shaders)
	{
		std::filesystem::path shaderPath(shader);
		auto shaderDirectory = shaderPath.parent_path();
		if (std::filesystem::exists(shaderDirectory))
		{
			auto shaderDirectoryStr = shaderDirectory.string();
			auto shaderName = shaderPath.stem();
			bool shaderFound = false;
			for (auto& dirEntry : std::filesystem::directory_iterator(shaderDirectory))
			{
				if (dirEntry.is_regular_file())
				{
					auto currentShaderPath = dirEntry.path();
					auto currentShaderFullName = currentShaderPath.filename().string();
					auto currentShaderName = currentShaderPath.stem();
					if (shaderName == currentShaderName)
					{
						if (currentShaderPath.extension() == ".fs")
						{
							shadersPaths[shader].fs = std::format("{}/{}", shaderDirectoryStr, currentShaderFullName);
							shaderFound = true;
						}
						else if (currentShaderPath.extension() == ".vs")
						{
							shadersPaths[shader].vs = std::format("{}/{}", shaderDirectoryStr, currentShaderFullName);
							shaderFound = true;
						}
						else if (currentShaderPath.extension() == ".gs")
						{
							shadersPaths[shader].gs = std::format("{}/{}", shaderDirectoryStr, currentShaderFullName);
							shaderFound = true;
						}
					}
				}
			}
			if (!shaderFound)
			{
				printf("\nSHADER PROGRAM: %s not found in directory: %s !\n", shaderName.string().c_str(), shaderPath.string().c_str());
			}
			else
			{
				shadersPaths[shader].path = shader;
			}
		}
	}
	return shadersPaths;
}

ShaderPaths GraphicsManager::LoadShaderPaths(const std::string& path)
{
	ShaderPaths shaderPaths;
	
	std::filesystem::path shaderPath(path);
	auto shaderDirectory = shaderPath.parent_path();
	if (std::filesystem::exists(shaderDirectory))
	{
		auto shaderDirectoryStr = shaderDirectory.string();
		auto shaderName = shaderPath.stem();
		bool shaderFound = false;
		for (auto& dirEntry : std::filesystem::directory_iterator(shaderDirectory))
		{
			if (dirEntry.is_regular_file())
			{
				auto currentShaderPath = dirEntry.path();
				auto currentShaderFullName = currentShaderPath.filename().string();
				auto currentShaderName = currentShaderPath.stem();
				if (shaderName == currentShaderName)
				{
					if (currentShaderPath.extension() == ".fs")
					{
						shaderPaths.fs = std::format("{}/{}", shaderDirectoryStr, currentShaderFullName);
						shaderFound = true;
					}
					else if (currentShaderPath.extension() == ".vs")
					{
						shaderPaths.vs = std::format("{}/{}", shaderDirectoryStr, currentShaderFullName);
						shaderFound = true;
					}
					else if (currentShaderPath.extension() == ".gs")
					{
						shaderPaths.gs = std::format("{}/{}", shaderDirectoryStr, currentShaderFullName);
						shaderFound = true;
					}
				}
			}
		}
		if (!shaderFound)
		{
			printf("\nSHADER PROGRAM: %s not found in directory: %s !\n", shaderName.string().c_str(), shaderPath.string().c_str());
		}
		else
		{
			shaderPaths.path = path;
		}
	}
	return shaderPaths;
}

bool GraphicsManager::ReloadShaders()
{
	return LoadShaders("config/shaders.txt");
}

Shader* GraphicsManager::ReloadShader(Shader* shader)
{
	ShaderPaths spath = LoadShaderPaths(shader->shaderPaths.path);
	if (spath.fs.empty() && spath.vs.empty() && spath.gs.empty())
	{
		return nullptr;
	}
	else
	{
		return ReloadShaderFromPath(GraphicsStorage::assetRegistry.GetAssetIDAsString(shader).c_str(), spath);
	}
}

Shader* GraphicsManager::ReloadShaderFromPath(const char* guid, const ShaderPaths& paths)
{
	std::string name = std::filesystem::path(paths.path).stem().string();
	printf("\033[1;36m\nLoading Shader: %s\033[0m", name.c_str());
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	std::chrono::duration<double> elapsed_seconds;
	start = std::chrono::high_resolution_clock::now();
	std::string VertexShaderCode = std::move(ReadTextFileIntoString(paths.vs));
	std::string FragmentShaderCode = std::move(ReadTextFileIntoString(paths.fs));
	std::string GeometryShaderCode = std::move(ReadTextFileIntoString(paths.gs));
	std::unordered_set<std::string> vsIncludes;
	std::unordered_set<std::string> fsIncludes;
	std::unordered_set<std::string> gsIncludes;
	ReloadShaderCode(VertexShaderCode, vsIncludes);
	ReloadShaderCode(FragmentShaderCode, fsIncludes);
	ReloadShaderCode(GeometryShaderCode, gsIncludes);
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	printf("\nParsing Took %fs\n", elapsed_seconds.count());
	start = std::chrono::high_resolution_clock::now();
	unsigned int result = LoadProgram(VertexShaderCode, FragmentShaderCode, GeometryShaderCode);
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	printf("\nLoading Took %fs\n", elapsed_seconds.count());
	Shader* shader = nullptr;
	if (result > 0)
	{
		bool existingShader = false;
		if (guid != nullptr)
		{
			shader = (Shader*)GraphicsStorage::assetRegistry.GetAssetByStringID(guid);
			if (shader == nullptr)
			{
				shader = GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<Shader>(guid, result, name, paths);
			}
			else
			{
				existingShader = true;
			}
		}
		else
		{
			if (GraphicsStorage::shaderPathsAndGuids.find(paths.path) != GraphicsStorage::shaderPathsAndGuids.end())
			{
				shader = (Shader*)GraphicsStorage::assetRegistry.GetAssetByStringID(GraphicsStorage::shaderPathsAndGuids[paths.path]);
				existingShader = true;
			}
			else
			{
				shader = GraphicsStorage::assetRegistry.AllocAsset<Shader>(result, name, paths);
			}
		}

		if (existingShader)
		{
			glDeleteProgram(GraphicsStorage::shaderIDs[name]);
			shader->shaderID = result;
			shader->name = name;
			shader->shaderPaths = paths;
		}
		
		GraphicsStorage::shaderPathsAndGuids[paths.path] = GraphicsStorage::assetRegistry.GetAssetIDAsString(shader);
		GraphicsStorage::shaderIDs[name] = result;

		shader->Clear();
		start = std::chrono::high_resolution_clock::now();
		LoadBlocks(shader, BlockType::Uniform);
		LoadBlocks(shader, BlockType::Storage);
		LoadOutputs(shader);
		LoadSamplers(shader);
		LoadAttributes(shader);
		end = std::chrono::high_resolution_clock::now();
		elapsed_seconds = end - start;
		printf("\nLoading Blocks Outputs Samplers and Attributes Took %fs\n", elapsed_seconds.count());
		start = std::chrono::high_resolution_clock::now();
		std::sort(shader->outputs.begin(), shader->outputs.end(), [](const auto& lhs, const auto& rhs)
		{
			return lhs.index < rhs.index;
		});
		std::sort(shader->attributes.begin(), shader->attributes.end(), [](const auto& lhs, const auto& rhs)
		{
			return lhs.index < rhs.index;
		});
		for (auto& attr : shader->attributes)
		{
			printf("\033[1;32min attr %d %s %s\033[0m\n", attr.index, ShaderDataType::Str(attr.type).data(), attr.name.c_str());
		}
		for (auto& sampler : shader->samplers)
		{
			printf("\033[1;32msampler %d %s %s\033[0m\n", sampler.index, sampler.type.c_str(), sampler.name.c_str());
		}
		end = std::chrono::high_resolution_clock::now();
		elapsed_seconds = end - start;
		printf("\nSorting and printing Took %fs\n", elapsed_seconds.count());
		return shader;
	}
	else
	{
		printf("\033[1;31mFailed to load shader: %s\033[0m\n", name.c_str());
		return shader;
	}
}

bool GraphicsManager::ReloadShaderCode(std::string& shaderCode, std::unordered_set<std::string>& shaderIncludes)
{
	RemoveComments(shaderCode);
	LoadShaderIncludes(shaderCode, shaderIncludes);
	return true;
}

bool GraphicsManager::LoadShaderIncludes(std::string& shaderCode, std::unordered_set<std::string>& shaderIncludes)
{
	while(true)
	{
		size_t nFPos = shaderCode.find("#include");
		if (nFPos + 1)
		{
			size_t endInclude = shaderCode.find(";", nFPos);
			std::string includeFile = shaderCode.substr(nFPos + 9, endInclude - (nFPos + 9));
			std::string pathToIncludeFile = GraphicsStorage::paths["resources"] + "shaders/" + includeFile;
			shaderCode.erase(nFPos, (endInclude + 1) - nFPos);
			if (shaderIncludes.find(includeFile) == shaderIncludes.end())
			{
				shaderIncludes.insert(includeFile);
				std::string includeShaderCode = std::move(ReadTextFileIntoString(pathToIncludeFile));
				shaderCode.insert(nFPos, includeShaderCode);
				ReloadShaderCode(pathToIncludeFile, shaderIncludes);
			}
		}
		else
		{
			break;
		}
	}
	return false;
}

void GraphicsManager::LoadBlocks(Shader* shader, BlockType type)
{
	GLenum blockInterface;
	GLenum resourceInterface;
	int nrOfVariableProperties;

	switch (type)
	{
	case BlockType::Uniform:
		blockInterface = GL_UNIFORM_BLOCK;
		resourceInterface = GL_UNIFORM;
		nrOfVariableProperties = 10;
		break;
	case BlockType::Storage:
		blockInterface = GL_SHADER_STORAGE_BLOCK;
		resourceInterface = GL_BUFFER_VARIABLE;
		nrOfVariableProperties = 9;
		break;
	default:
		blockInterface = GL_UNIFORM_BLOCK;
		resourceInterface = GL_UNIFORM;
		nrOfVariableProperties = 10;
		break;
	}

	GLint numBlocks = 0;
	glGetProgramInterfaceiv(shader->shaderID, blockInterface, GL_ACTIVE_RESOURCES, &numBlocks);
	const GLenum blockProperties[4] = { GL_NUM_ACTIVE_VARIABLES, GL_BUFFER_DATA_SIZE, GL_BUFFER_BINDING, GL_NAME_LENGTH };
	const GLenum activeUnifProp[1] = { GL_ACTIVE_VARIABLES };
	const GLenum shaderReferences[6] = { 
		GL_REFERENCED_BY_VERTEX_SHADER,
		GL_REFERENCED_BY_TESS_CONTROL_SHADER,
		GL_REFERENCED_BY_TESS_EVALUATION_SHADER,
		GL_REFERENCED_BY_GEOMETRY_SHADER,
		GL_REFERENCED_BY_FRAGMENT_SHADER,
		GL_REFERENCED_BY_COMPUTE_SHADER
	};
	const GLenum unifProperties[10] = {
		GL_NAME_LENGTH, GL_TYPE,
		GL_ARRAY_SIZE, GL_OFFSET, GL_BLOCK_INDEX,
		GL_ARRAY_STRIDE, GL_MATRIX_STRIDE, GL_IS_ROW_MAJOR,
		GL_ATOMIC_COUNTER_BUFFER_INDEX, GL_LOCATION
	};

	for (int blockIx = 0; blockIx < numBlocks; ++blockIx)
	{
		GLint blockPropertyValues[4];
		glGetProgramResourceiv(shader->shaderID, blockInterface, blockIx, 4, blockProperties, 4, NULL, blockPropertyValues);

		if (!blockPropertyValues[0])
			continue;

		std::vector<GLint> blockUnifs(blockPropertyValues[0]);
		glGetProgramResourceiv(shader->shaderID, blockInterface, blockIx, 1, activeUnifProp, blockPropertyValues[0], NULL, &blockUnifs[0]);

		GLint shaderRefs[6];
		glGetProgramResourceiv(shader->shaderID, blockInterface, blockIx, 6, shaderReferences, 6, NULL, shaderRefs);

		std::vector<char> blockName(blockPropertyValues[3]);
		glGetProgramResourceName(shader->shaderID, blockInterface, blockIx, blockName.size(), NULL, &blockName[0]);

		ShaderBlock* shaderBlock = nullptr;
		if (type == BlockType::Uniform) shaderBlock = GraphicsStorage::GetUniformBuffer(blockPropertyValues[2]);
		else shaderBlock = GraphicsStorage::GetShaderStorageBuffer(blockPropertyValues[2]);
		if (shaderBlock == nullptr)
		{
			shaderBlock = GraphicsStorage::assetRegistry.AllocAsset<ShaderBlock>(blockPropertyValues[1], blockPropertyValues[2], type);
			shaderBlock->name = std::string(blockName.begin(), blockName.end() - 1);
			if (type == BlockType::Uniform) GraphicsStorage::uniformBuffers.push_back(shaderBlock);
			else GraphicsStorage::shaderStorageBuffers.push_back(shaderBlock);
		}
		else
		{
			//Without this when shader is reloaded after being changed the uniform buffers can mismatch, offsets can be different, size and index
			//also if we reload shader and uniform buffer then what happens to other shaders that use it?
			//it is quite unnecessary when reloading all shader or on startup
			glDeleteBuffers(1, &shaderBlock->handle); //should put in destructor
			GraphicsStorage::assetRegistry.DeallocAsset<ShaderBlock>(shaderBlock);
			shaderBlock = GraphicsStorage::assetRegistry.AllocAsset<ShaderBlock>(blockPropertyValues[1], blockPropertyValues[2], type);
			shaderBlock->name = std::string(blockName.begin(), blockName.end() - 1);
		}
		std::string shaderBlockConfigPath = "resources/shader_blocks/" + shaderBlock->name + ".json";
		std::string config = LoadShaderBlockConfig(shaderBlockConfigPath.c_str());
		if (!config.empty())
		{
			if (type == BlockType::Uniform)
			{
				if (config == "Pass")
				{
					shader->globalUniformBuffers.push_back(shaderBlock);
				}
				else if (config == "Material")
				{
					shader->materialUniformBuffers.push_back(shaderBlock);
				}
				else
				{
					shader->objectUniformBuffers.push_back(shaderBlock);
				}
			}
			else
			{
				if (config == "Pass")
				{
					shader->globalShaderStorageBuffers.push_back(shaderBlock);
				}
				else if (config == "Material")
				{
					shader->materialShaderStorageBuffers.push_back(shaderBlock);
				}
				else
				{
					shader->objectShaderStorageBuffers.push_back(shaderBlock);
				}
			}
		}
		GraphicsStorage::shaderBlockTypes[shaderBlock->name] = config;
		/*
		if (type == BlockType::Uniform)
		{
			std::string bufferType = shaderBlock->name.substr(0, 2);
			if (bufferType == "O_") { shader->objectUniformBuffers.push_back(shaderBlock); }
			else if (bufferType == "M_") { shader->materialUniformBuffers.push_back(shaderBlock); }
			else if (bufferType == "G_") { shader->globalUniformBuffers.push_back(shaderBlock); }
			else shader->globalUniformBuffers.push_back(shaderBlock);
		}
		else
		{
			std::string bufferType = shaderBlock->name.substr(0, 2);
			if (bufferType == "O_") shader->objectShaderStorageBuffers.push_back(shaderBlock);
			else if (bufferType == "M_") shader->materialShaderStorageBuffers.push_back(shaderBlock);
			else if (bufferType == "G_") shader->globalShaderStorageBuffers.push_back(shaderBlock);
			else shader->globalShaderStorageBuffers.push_back(shaderBlock);
		}
		*/

		std::unordered_map<std::string, uniform_info_t> uniforms;
		for (int unifIx = 0; unifIx < blockPropertyValues[0]; ++unifIx)
		{
			std::vector<GLint> values(nrOfVariableProperties);
			glGetProgramResourceiv(shader->shaderID, resourceInterface, blockUnifs[unifIx], nrOfVariableProperties, unifProperties, nrOfVariableProperties, NULL, &values[0]);

			// Get the name. Must use a std::vector rather than a std::string for C++03 standards issues.
			// C++11 would let you use a std::string directly.
			std::vector<char> uniformName(values[0]);
			glGetProgramResourceName(shader->shaderID, resourceInterface, blockUnifs[unifIx], uniformName.size(), NULL, &uniformName[0]);

			uniform_info_t uniform_info = {};
			for (size_t i = 0; i < nrOfVariableProperties - 2; i++) //we skip the last two properties, we set them manually below
			{
				uniform_info.properties[i] = values[i + 1]; //we skip first property, the length of the name
			}
			uniform_info.location = values[8] == -1 ? blockUnifs[unifIx] : values[8];

			unsigned int uniformType = values[1];
			uniform_info.size = ShaderDataType::Size(ShaderDataType::FromOpenGLType(uniformType));

			uniforms.try_emplace(std::string(uniformName.begin(), uniformName.end() - 1), uniform_info);
			shaderBlock->AddVariableOffset(std::string(uniformName.begin(), uniformName.end() - 1), uniform_info.offset);
		}
		GraphicsStorage::shaderBlockUniforms[shaderBlock->name] = uniforms;
	}
}

void GraphicsManager::LoadOutputs(Shader* shader)
{
	GLint numResources = 0;
	GLenum resourceInterface = GL_PROGRAM_OUTPUT;
	glGetProgramInterfaceiv(shader->shaderID, resourceInterface, GL_ACTIVE_RESOURCES, &numResources);
	const GLenum resourceProperties[5] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_LOCATION_INDEX, GL_LOCATION_COMPONENT };

	shader->outputs.resize(numResources);

	for (int resIndex = 0; resIndex < numResources; ++resIndex)
	{
		GLint resourcePropertyValues[5];
		glGetProgramResourceiv(shader->shaderID, resourceInterface, resIndex, 5, resourceProperties, 5, NULL, resourcePropertyValues);

		std::vector<char> resourceName(resourcePropertyValues[0]);
		glGetProgramResourceName(shader->shaderID, resourceInterface, resIndex, resourceName.size(), NULL, &resourceName[0]);
		std::string outputName = std::string(resourceName.begin(), resourceName.end() - 1);

		GLint resourceType = resourcePropertyValues[1];

		GLint resourceLocation = resourcePropertyValues[2];

		ShaderOutput& output = shader->outputs[resIndex];
		output.index = resourceLocation;
		output.type = ShaderDataType::FromOpenGLType(resourceType);
		output.name = outputName;
		printf("\033[1;32mout %d %s %s\033[0m\n", output.index, ShaderDataType::Str(output.type).data(), output.name.c_str());
	}
}

void GraphicsManager::LoadSamplers(Shader* shader)
{
	GLint numResources = 0;
	GLenum resourceInterface = GL_UNIFORM;
	glGetProgramInterfaceiv(shader->shaderID, resourceInterface, GL_ACTIVE_RESOURCES, &numResources);
	const GLenum resourceProperties[3] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION };

	ShaderSampler sampler;
	for (int resIndex = 0; resIndex < numResources; ++resIndex)
	{
		GLint resourcePropertyValues[3];
		glGetProgramResourceiv(shader->shaderID, resourceInterface, resIndex, 3, resourceProperties, 3, NULL, resourcePropertyValues);

		std::vector<char> resourceName(resourcePropertyValues[0]);
		glGetProgramResourceName(shader->shaderID, resourceInterface, resIndex, resourceName.size(), NULL, &resourceName[0]);

		std::string uniformName = std::string(resourceName.begin(), resourceName.end() - 1);

		GLint resourceType = resourcePropertyValues[1];
		
		GLint resourceLocation = resourcePropertyValues[2];

		switch (resourceType)
		{
		case GL_SAMPLER_1D:
		{
			sampler.index = resourceLocation;
			sampler.type = "sampler1D";
			sampler.name = uniformName;
			shader->samplers.emplace_back(std::move(sampler));
			break;
		}
		case GL_SAMPLER_2D:
		{
			sampler.index = resourceLocation;
			sampler.type = "sampler2D";
			sampler.name = uniformName;
			shader->samplers.emplace_back(std::move(sampler));
			break;
		}
		case GL_SAMPLER_3D:
		{
			sampler.index = resourceLocation;
			sampler.type = "sampler3D";
			sampler.name = uniformName;
			shader->samplers.emplace_back(std::move(sampler));
			break;
		}
		case GL_SAMPLER_CUBE:
		{
			sampler.index = resourceLocation;
			sampler.type = "samplerCUBE";
			sampler.name = uniformName;
			shader->samplers.emplace_back(std::move(sampler));
			break;
		}
		default:
		{
			break;
		}
		}
	}
}

void GraphicsManager::LoadAttributes(Shader* shader)
{
	GLint numResources = 0;
	GLenum resourceInterface = GL_PROGRAM_INPUT;
	glGetProgramInterfaceiv(shader->shaderID, resourceInterface, GL_ACTIVE_RESOURCES, &numResources);
	const GLenum resourceProperties[4] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_LOCATION_COMPONENT };

	shader->attributes.resize(numResources);

	for (int resIndex = 0; resIndex < numResources; ++resIndex)
	{
		GLint resourcePropertyValues[4];
		glGetProgramResourceiv(shader->shaderID, resourceInterface, resIndex, 4, resourceProperties, 4, NULL, resourcePropertyValues);

		std::vector<char> resourceName(resourcePropertyValues[0]);
		glGetProgramResourceName(shader->shaderID, resourceInterface, resIndex, resourceName.size(), NULL, &resourceName[0]);
		std::string outputName = std::string(resourceName.begin(), resourceName.end() - 1);

		GLint resourceType = resourcePropertyValues[1];

		ShaderDataType::Type outputType = ShaderDataType::FromOpenGLType(resourceType);

		GLint resourceLocation = resourcePropertyValues[2];

		ShaderOutput& output = shader->attributes[resIndex];
		output.index = resourceLocation;
		output.type = outputType;
		output.name = outputName;
		//std::string attributeType = output.name.substr(0, 3);
		//if (attributeType == "IA_")
		//{
		//	shader->dynamicAttributes.push_back(output);
		//}
	}
}

std::string GraphicsManager::LoadShaderBlockConfig(const char* path)
{
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	std::string scriptPath = "resources/serialization/shader_block_config_loader.lua";
	LuaTools::dofile(L, scriptPath.c_str());
	lua_getglobal(L, "LoadShaderBlockConfig");
	std::string config;
	if (lua_isfunction(L, -1))
	{
		lua_pushstring(L, path);
		int result = LuaTools::report(L, LuaTools::docall(L, 1, 0));
		config = lua_tostring(L, -1);
		lua_pop(L, 1);
	}
	lua_close(L);
	return config;
}

std::string GraphicsManager::ReadTextFileIntoString(const char* path)
{
	if (path != nullptr && strcmp(path, "") != 0)
	{
		// Read the Vertex Shader code from the file
		std::string fileContent;
		std::ifstream fileStream(path, std::ios::in);
		if (fileStream.is_open())
		{
			std::string Line = "";
			while (getline(fileStream, Line))
				fileContent += Line + "\n";
			fileStream.close();
		}
		return fileContent;
	}
	return std::string();
}

std::string GraphicsManager::ReadTextFileIntoString(const std::string& path)
{
	if (path.length() > 0)
	{
		// Read the Vertex Shader code from the file
		std::string fileContent;
		std::ifstream fileStream(path, std::ios::in);
		if (fileStream.is_open())
		{
			std::string Line = "";
			while (getline(fileStream, Line))
				fileContent += Line + "\n";
			fileStream.close();
		}
		return fileContent;
	}
	return std::string();
}

void GraphicsManager::WriteStringToTextFile(std::string& content, std::string& path)
{
	std::ofstream out(path);
	if (out.is_open())
	{
		out << content;
		out.close();
	}
}

void GraphicsManager::AppendStringToTextFile(std::string& content, std::string& path)
{
	std::ifstream in(path);
	std::ofstream out(path, std::ios::app);
	if (in.is_open())
	{
		out << content;
		in.close();
		out.close();
	}
}

void GraphicsManager::RemoveLineFromTextFileContainingText(std::string& content, std::string& path)
{
	std::ifstream in(path, std::ios::in);
	std::ofstream out(GraphicsStorage::paths["resources"] + "tempfile.txt", std::ios::out);
	if (in.is_open())
	{
		std::string line;
		if (std::getline(in, line))
		{
			int result = line.find(content, 0);
			if (result == std::string::npos)
			{
				out << line;
			}
		}
		while (std::getline(in, line))
		{
			int result = line.find(content, 0);
			if (result == std::string::npos)
			{
				out << "\n" << line;
			}
		}
		out.close();
		in.close();
		std::remove(path.c_str());
		std::rename((GraphicsStorage::paths["resources"] + "tempfile.txt").c_str(), path.c_str());
	}
}

GLuint GraphicsManager::LoadProgram(const std::string& VertexShaderCode, const std::string& FragmentShaderCode, const std::string& GeometryShaderCode) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint GeometryShaderID = -1;
	if (!GeometryShaderCode.empty())
	{
		GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("\nCompiling VS shader");
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		std::vector<char> VertexShaderErrorMessage(InfoLogLength);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);
	}

	if (!FragmentShaderCode.empty())
	{
		// Compile Fragment Shader
		printf("\nCompiling FS shader");
		char const* FragmentSourcePointer = FragmentShaderCode.c_str();
		glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
		glCompileShader(FragmentShaderID);

		// Check Fragment Shader
		glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0)
		{
			std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
			glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
			fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);
		}
	}

	if (!GeometryShaderCode.empty())
	{
		// Compile Geometry Shader
		printf("\nCompiling GS shader");
		char const* GeometrySourcePointer = GeometryShaderCode.c_str();
		glShaderSource(GeometryShaderID, 1, &GeometrySourcePointer, NULL);
		glCompileShader(GeometryShaderID);

		// Check Geometry Shader
		glGetShaderiv(GeometryShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(GeometryShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0)
		{
			std::vector<char> GeometryShaderErrorMessage(InfoLogLength);
			glGetShaderInfoLog(GeometryShaderID, InfoLogLength, NULL, &GeometryShaderErrorMessage[0]);
			fprintf(stdout, "%s\n", &GeometryShaderErrorMessage[0]);
		}
	}

	// Link the program
	fprintf(stdout, "\nLinking program");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	if (!GeometryShaderCode.empty())
	{
		glAttachShader(ProgramID, GeometryShaderID);
	}
	if (!FragmentShaderCode.empty())
	{
		glAttachShader(ProgramID, FragmentShaderID);
	}
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);

	glDeleteShader(VertexShaderID);
	if (!GeometryShaderCode.empty())
	{
		glDeleteShader(GeometryShaderID);
	}
	if (!FragmentShaderCode.empty())
	{
		glDeleteShader(FragmentShaderID);
	}
	
	fprintf(stdout, " Shader ID: %d \n", ProgramID);
	
	if (InfoLogLength > 0)
	{
		std::vector<char> ProgramErrorMessage(std::max<int>(InfoLogLength, int(1)));
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);
		ProgramID = 0;
	}
	return ProgramID;
}

void GraphicsManager::LoadAllAssets()
{
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	std::chrono::duration<double> elapsed_seconds;

	printf("\nALLOCATING MEMORY\n");
	start = std::chrono::high_resolution_clock::now();
	
	GraphicsStorage::assetRegistry.RegisterType<Texture>();
	GraphicsStorage::assetRegistry.RegisterType<RenderBuffer>();
	GraphicsStorage::assetRegistry.RegisterType<VertexArray>();
	GraphicsStorage::assetRegistry.RegisterType<VertexBuffer>();
	GraphicsStorage::assetRegistry.RegisterType<ElementBuffer>();
	GraphicsStorage::assetRegistry.RegisterType<BufferLayout>();
	GraphicsStorage::assetRegistry.RegisterType<LocationLayout>();
	GraphicsStorage::assetRegistry.RegisterType<VertexBufferDynamic>();
	GraphicsStorage::assetRegistry.RegisterType<Shader>();
	GraphicsStorage::assetRegistry.RegisterType<ShaderBlock>();
	GraphicsStorage::assetRegistry.RegisterType<CPUBlockData>();
	GraphicsStorage::assetRegistry.RegisterType<OBJ>();

	GraphicsStorage::assetRegistry.RegisterType<RenderPass>();
	GraphicsStorage::assetRegistry.RegisterType<RenderProfile>();
	GraphicsStorage::assetRegistry.RegisterType<TextureProfile>();
	GraphicsStorage::assetRegistry.RegisterType<MaterialProfile>();

	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	printf("\nDONE Took %fs\n", elapsed_seconds.count());


	printf("\nLOADING PATHS\n");
	start = std::chrono::high_resolution_clock::now();
	LoadPaths("config/paths.txt");
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	printf("\nDONE Took %fs\n", elapsed_seconds.count());

	printf("\nLOADING GPU PROGRAMS\n");
	start = std::chrono::high_resolution_clock::now();
	LoadShaders("config/shaders.txt");
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	printf("\nDONE Took %fs\n", elapsed_seconds.count());

	printf("\nLOADING OBJs\n");
	start = std::chrono::high_resolution_clock::now();
	std::vector<OBJ*> parsedOBJs;
	LoadOBJs("config/models.txt", parsedOBJs);
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	printf("\nDONE Took %fs\n", elapsed_seconds.count());
	
	printf("\nLOADING OBJs TO VAOs\n");
	start = std::chrono::high_resolution_clock::now();
	LoadOBJsToVAOs(parsedOBJs);
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	printf("\nDONE Took %fs\n", elapsed_seconds.count());

	printf("\nLOADING TEXTURES\n");
	start = std::chrono::high_resolution_clock::now();
	LoadTextures("config/textures.txt");
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	printf("\nDONE Took %fs\n", elapsed_seconds.count());

	printf("\nLOADING CUBE MAPS\n");
	start = std::chrono::high_resolution_clock::now();
	LoadCubeMaps("config/cubemaps.txt");
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	printf("\nDONE Took %fs\n", elapsed_seconds.count());
}

TextureInfo* GraphicsManager::LoadImage(const char* path, int forcedNumOfEle)
{
	return SOIL_load_image(path, forcedNumOfEle);
}

Texture* GraphicsManager::LoadTexture(const char* guid, const char* path)
{
	std::string fullName = path;
	std::filesystem::path fullPath(fullName);
	std::string fileName = fullPath.stem().string();
	TextureInfo* ti = nullptr;
	Texture* tex = nullptr;
	
	// we have to pass guid to these functions
	ti = GraphicsManager::LoadImage(path, 0);
	return LoadTextureIntoGPU(guid, path, ti);

	// next steps:
	// we might want to create and generate texture return that to lua
	// that will give valid texture handle non threaded
	// load multi threaded the image
	// and lastly load it's settings non threaded, we will call that from c++ when ready
	// then again we also want to be able to load this stuff entirely non threaded 
}

Texture* GraphicsManager::LoadCompressedDDS(Texture* tex, TextureInfo* ti, int index)
{
	DDSExtraInfo* ei = (DDSExtraInfo*)ti->extraInfo;
	unsigned char* buffer = (unsigned char*)ti->data;
	unsigned int internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	switch (ei->fourCC)
	{
	case FOURCC_DXT1:
		internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case FOURCC_DXT3:
		internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case FOURCC_DXT5:
		internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	default:
		return nullptr;
	}

	tex->Generate();
	tex->Bind();
	tex->hasMipMaps = ei->nrOfMips > 1;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	unsigned int blockSize = (internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
	unsigned int offset = 0;
	int width = ti->width;
	int height = ti->height;

	int tWidth = width;
	int tHeight = height;
	if (index > 0)
	{
		int mipOffset = 0;
		for (size_t i = 0; i < ei->nrOfMips && (tWidth || tHeight); i++)
		{
			unsigned int size = ((tWidth + 3) / 4) * ((tHeight + 3) / 4) * blockSize;
			mipOffset += size;
			tWidth /= 2;
			tHeight /= 2;

			// Deal with Non-Power-Of-Two textures.
			if (tWidth < 1) tWidth = 1;
			if (tHeight < 1) tHeight = 1;
		}
		offset += (mipOffset * index);
	}

	for (unsigned int level = 0; level < ei->nrOfMips && (width || height); ++level)
	{
		unsigned int size = ((width + 3) / 4) * ((height + 3) / 4) * blockSize;
		glCompressedTexImage2D(GL_TEXTURE_2D, level, internalFormat, width, height, 0, size, buffer + offset);

		offset += size;
		width /= 2;
		height /= 2;

		// Deal with Non-Power-Of-Two textures.
		if (width < 1) width = 1;
		if (height < 1) height = 1;
	}
	tex->internalFormat = internalFormat;
	return tex;
}

Texture* GraphicsManager::LoadCompressedDDSCubeMap(Texture* tex, TextureInfo* ti)
{
	DDSExtraInfo* ei = (DDSExtraInfo*)ti->extraInfo;
	unsigned char* buffer = (unsigned char*)ti->data;
	unsigned int internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	switch (ei->fourCC)
	{
	case FOURCC_DXT1:
		internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case FOURCC_DXT3:
		internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case FOURCC_DXT5:
		internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	default:
		return nullptr;
	}

	tex->Generate();
	tex->Bind();
	tex->hasMipMaps = ei->nrOfMips > 1;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	unsigned int blockSize = (internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
	unsigned int offset = 0;
	unsigned int size = ((ti->width + 3) / 4) * ((ti->height + 3) / 4) * blockSize;
	for (unsigned int i = 0; i < 6; ++i)
	{
		glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, ti->width, ti->height, 0, size, buffer + offset);
		offset += size;
		for (int i = 1; i < ei->nrOfMips; ++i)
		{
			int mx = ti->width >> (i + 2);
			int my = ti->height >> (i + 2);
			if (mx < 1)
			{
				mx = 1;
			}
			if (my < 1)
			{
				my = 1;
			}
			int mipOffset = mx * my * blockSize;
			offset += mipOffset;
		}
	}
	tex->internalFormat = internalFormat;
	return tex;
}

Texture* GraphicsManager::LoadCompressedDDSCubeMapFace(Texture* tex, TextureInfo* ti, int index)
{
	DDSExtraInfo* ei = (DDSExtraInfo*)ti->extraInfo;
	unsigned char* buffer = (unsigned char*)ti->data;
	unsigned int internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	switch (ei->fourCC)
	{
	case FOURCC_DXT1:
		internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case FOURCC_DXT3:
		internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case FOURCC_DXT5:
		internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	default:
		return nullptr;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	unsigned int blockSize = (internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
	unsigned int offset = 0;
	int width = ti->width;
	int height = ti->height;
	unsigned int size = ((width + 3) / 4) * ((height + 3) / 4) * blockSize;

	glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, 0, internalFormat, width, height, 0, size, buffer);

	tex->internalFormat = internalFormat;
	tex->width = ti->width;
	tex->height = ti->height;
	tex->hasMipMaps = ei->nrOfMips > 1;
	return tex;
}

Texture* GraphicsManager::LoadCubeMap(const char* guid, const char* path)
{
	FILE* texturesFile = NULL;
	texturesFile = fopen(path, "r");
	if (texturesFile == NULL) {
		printf("%s could not be opened.\n", path);
		return nullptr;
	}
	printf("Loading cubemap: %s\n", path);
	std::vector<std::string> texturesPaths;
	while (1)
	{
		char cubeMapTexturePath[128];
		int res = fscanf(texturesFile, "%s", cubeMapTexturePath);
		if (res == EOF)
		{
			break; // EOF = End Of File. Quit the loop.
		}
		texturesPaths.push_back(GraphicsStorage::paths["resources"] + cubeMapTexturePath);
	}
	fclose(texturesFile);

	Texture* tex = nullptr;
	
	if (texturesPaths.size() == 1)
	{
		TextureInfo* ti = GraphicsManager::LoadImage(texturesPaths.at(0).c_str(), 0);
		if (ti != nullptr)
		{
			DDSExtraInfo* ei = NULL;
			int internalFormat;
			int format;
			switch (ti->type)
			{
			case DDS:
				ei = (DDSExtraInfo*)ti->extraInfo;
				switch (ti->numOfElements)
				{
				case 1:
					internalFormat = GL_R8; format = GL_RED;
					break;
				case 2:
					internalFormat = GL_RG8; format = GL_RG;
					break;
				case 3:
					internalFormat = GL_RGB; format = GL_RGB;
					break;
				case 4:
					internalFormat = GL_RGBA; format = GL_RGBA;
					break;
				default:
					internalFormat = GL_RGB; format = GL_RGB;
					break;
				}
				if (ei->nrOfCubeMapFaces == 6)
				{
					tex = GraphicsStorage::assetRegistry.AllocAsset<Texture>(GL_TEXTURE_CUBE_MAP, 0, internalFormat, ti->width, ti->height, format, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
					if (ei->compressed)
					{
						LoadCompressedDDSCubeMap(tex, ti);
					}
					else
					{
						tex->Generate();
						tex->Bind();
						
						for (int i = 0; i < ei->nrOfCubeMapFaces; i++)
						{
							tex->SpecifyTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, ti->width, ti->height, &((unsigned char*)ti->data)[i * ti->width * ti->height * ti->numOfElements]);
						}
					}
				}
				break;
			default:
				switch (ti->numOfElements)
				{
				case 1:
					internalFormat = GL_R8; format = GL_RED;
					break;
				case 2:
					internalFormat = GL_RG8; format = GL_RG;
					break;
				case 3:
					internalFormat = GL_RGB; format = GL_RGB;
					break;
				case 4:
					internalFormat = GL_RGBA; format = GL_RGBA;
					break;
				default:
					internalFormat = GL_RGB; format = GL_RGB;
					break;
				}

				tex = GraphicsStorage::assetRegistry.AllocAsset<Texture>(GL_TEXTURE_CUBE_MAP, 0, internalFormat, ti->width, ti->height, format, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
				if (ei->nrOfCubeMapFaces == 6)
				{
					tex->Generate();
					tex->Bind();

					for (int i = 0; i < ei->nrOfCubeMapFaces; i++)
					{
						tex->SpecifyTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, ti->width, ti->height, &((unsigned char*)ti->data)[i * ti->width * ti->height * ti->numOfElements]);
					}
				}
				break;
			}
			if (tex != nullptr)
			{
				std::string fullName = texturesPaths.at(0);
				std::filesystem::path fullPath(fullName);
				tex->name = fullPath.parent_path().stem().string();
				tex->texturePath = path;
				tex->SetDefaultParameters();
				SOIL_free_texture_info(ti);
			}
		}
		else
		{
			printf("%s could not be opened.\n", texturesPaths.at(0).c_str());
		}
	}
	else
	{
		if (texturesPaths.size() == 6)
		{
			TextureInfo* ti = GraphicsManager::LoadImage(texturesPaths.at(0).c_str(), 0);
			if (ti != nullptr)
			{
				DDSExtraInfo* ei = nullptr;
				int internalFormat;
				int format;
				switch (ti->type)
				{
				case DDS:
					ei = (DDSExtraInfo*)ti->extraInfo;
					switch (ti->numOfElements)
					{
					case 1:
						internalFormat = GL_R8; format = GL_RED;
						break;
					case 2:
						internalFormat = GL_RG8; format = GL_RG;
						break;
					case 3:
						internalFormat = GL_RGB; format = GL_RGB;
						break;
					case 4:
						internalFormat = GL_RGBA; format = GL_RGBA;
						break;
					default:
						internalFormat = GL_RGB; format = GL_RGB;
						break;
					}
					if (guid != nullptr)
					{
						tex = GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<Texture>(guid, GL_TEXTURE_CUBE_MAP, 0, internalFormat, ti->width, ti->height, format, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
					}
					else
					{
						tex = GraphicsStorage::assetRegistry.AllocAsset<Texture>(GL_TEXTURE_CUBE_MAP, 0, internalFormat, ti->width, ti->height, format, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
					}
					tex->Generate();
					tex->Bind();
					if (ei->compressed)
					{
						LoadCompressedDDSCubeMapFace(tex, ti, 0); //face0
						SOIL_free_texture_info(ti);
						for (size_t i = 1; i < 6; i++) //face 1-5
						{
							ti = GraphicsManager::LoadImage(texturesPaths.at(i).c_str(), 0);
							if (ti != nullptr)
							{
								LoadCompressedDDSCubeMapFace(tex, ti, (int)i);
								SOIL_free_texture_info(ti);
							}
							else
							{
								printf("%s could not be opened.\n", texturesPaths.at(i).c_str());
							}
						}
					}
					else
					{
						tex->SpecifyTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0, ti->width, ti->height, ti->data); //face0
						SOIL_free_texture_info(ti);
						for (int i = 1; i < 6; i++) //face 1-5
						{
							ti = GraphicsManager::LoadImage(texturesPaths.at(i).c_str(), 0);
							if (ti != nullptr)
							{
								tex->SpecifyTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, ti->width, ti->height, ti->data);
								SOIL_free_texture_info(ti);
							}
							else
							{
								printf("%s could not be opened.\n", texturesPaths.at(i).c_str());
							}
						}
					}
					break;
				default:
					switch (ti->numOfElements)
					{
					case 1:
						internalFormat = GL_R8; format = GL_RED;
						break;
					case 2:
						internalFormat = GL_RG8; format = GL_RG;
						break;
					case 3:
						internalFormat = GL_RGB; format = GL_RGB;
						break;
					case 4:
						internalFormat = GL_RGBA; format = GL_RGBA;
						break;
					default:
						internalFormat = GL_RGB; format = GL_RGB;
						break;
					}
					if (guid != nullptr)
					{
						tex = GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<Texture>(guid, GL_TEXTURE_CUBE_MAP, 0, internalFormat, ti->width, ti->height, format, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
					}
					else
					{
						tex = GraphicsStorage::assetRegistry.AllocAsset<Texture>(GL_TEXTURE_CUBE_MAP, 0, internalFormat, ti->width, ti->height, format, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
					}
					tex->Generate();
					tex->Bind();

					tex->SpecifyTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0, ti->width, ti->height, ti->data);
					SOIL_free_texture_info(ti);
					for (int i = 1; i < 6; i++)
					{
						ti = GraphicsManager::LoadImage(texturesPaths.at(i).c_str(), 0);
						tex->SpecifyTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, ti->width, ti->height, ti->data);
						SOIL_free_texture_info(ti);
					}
					break;
				}
			}
			else
			{
				printf("%s could not be opened.\n", texturesPaths.at(0).c_str());
			}
		}
		if (tex != nullptr)
		{
			std::string fullName = texturesPaths.at(0);
			std::filesystem::path fullPath(fullName);
			tex->name = fullPath.parent_path().stem().string();
			tex->texturePath = path;
			tex->SetDefaultParameters();
		}
	}
	
	return tex;
}

bool GraphicsManager::DumpTexture(FrameBuffer* fbo, Texture* texture)
{
	int x = texture->width;
	int y = texture->height;
	int nrOfChannels = 3;
	if (texture->format == GL_RGBA) nrOfChannels = 4;
	long imageSize = x * y * nrOfChannels;
	unsigned char* data = new unsigned char[imageSize];
	fbo->ReadPixelData(0, 0, x, y, GL_UNSIGNED_BYTE, data, texture);
	int xa = x % 256;
	int xb = (x - xa) / 256; int ya = y % 256;
	int yb = (y - ya) / 256;
	unsigned char header[18] = { 0,0,2,0,0,0,0,0,0,0,0,0,(char)xa,(char)xb,(char)ya,(char)yb,24,0 };

	std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::cout << "finished computation at " << std::ctime(&currentTime) << "\n";
	std::tm* now = std::localtime(&currentTime);
	std::ostringstream fileName;
	fileName << texture->name << 1900 + now->tm_year << "-" << 1 + now->tm_mon << "-" << now->tm_mday << "_" << now->tm_hour << "-" << now->tm_min << "-" << now->tm_sec << ".tga";

	std::fstream File(fileName.str(), std::ios::out | std::ios::binary);
	File.write(reinterpret_cast<char*>(header), sizeof(char) * 18);
	File.write(reinterpret_cast<char*>(data), sizeof(char) * imageSize);
	File.close();
	delete[] data;
	return false;
}

bool GraphicsManager::DumpTextureSTB(FrameBuffer* fbo, Texture* texture)
{
	int x = texture->width;
	int y = texture->height;
	int nrOfChannels = 3;
	if (texture->format == GL_RGBA) nrOfChannels = 4;
	long imageSize = x * y * nrOfChannels;
	unsigned char* data = new unsigned char[imageSize];
	fbo->ReadPixelData(0, 0, x, y, GL_UNSIGNED_BYTE, data, texture);

	std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::cout << "finished computation at " << std::ctime(&currentTime) << "\n";
	std::tm* now = std::localtime(&currentTime);
	std::ostringstream fileName;
	fileName << texture->name << 1900 + now->tm_year << "-" << 1 + now->tm_mon << "-" << now->tm_mday << "_" << now->tm_hour << "-" << now->tm_min << "-" << now->tm_sec << ".png";

	SOIL_save_image(fileName.str().c_str(), SOIL_SAVE_TYPE_PNG, x, y, nrOfChannels, data);
	//stbi_write_tga(fileName.str().c_str(), x, y, nrOfChannels, data);
	//stbi_write_png(fileName.str().c_str(), windowWidth, windowHeight, nrOfChannels, data, x*nrOfChannels);
	//stbi_write_bmp(fileName.str().c_str(), windowWidth, windowHeight, 3, data);
	//stbi_write_jpg(fileName.str().c_str(), windowWidth, windowHeight, 3, data, int quality);
	//stbi_write_hdr(fileName.str().c_str(), windowWidth, windowHeight, 3, data);
	delete[] data;
	return false;
}
