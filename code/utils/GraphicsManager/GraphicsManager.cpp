#define _CRT_SECURE_NO_DEPRECATE
#include "GraphicsManager.h"
#include "GraphicsStorage.h"
#include "OBJ.h"
#include "Vao.h"
#include "Material.h"
#include "Texture.h"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>
#include <algorithm>
#include "dirent.h"
#include "Shader.h"
#include "CPUBlockData.h"
#include "ShaderBlock.h"
#include "FrameBuffer.h"
#include "ShaderBlockData.h"
#include <GL/glew.h>
#include <mutex>
#include <thread>
#include <iosfwd>
#include "SOIL2.h"
#include "stb_image.h"

#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

static std::mutex objLoadMutex;
static std::mutex tiLoadMutex;

struct uniform_info_t
{
	union
	{
		struct { GLint type, count, offset, blockIndex, arrayStride, matrixStride, isRowMajor, atomicCounterBufferIndex, location, size; };
		GLint properties[10];
	};
	std::string name;
};

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

bool GraphicsManager::LoadOBJs(const char* path)
{
	FILE* file;
	file = fopen(path, "r");
	if (file == NULL) {
		printf("%s could not be opened.\n", path);
		return false;
	}
	std::vector<std::thread> threadPool;
	while (1) {

		char lineHeader[128];
		//meshID not in use currently
		int objID = 0;
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
		{
			break; // EOF = End Of File. Quit the loop.
		}
		// else : parse lineHeader
		threadPool.push_back(std::thread(LoadOBJ, &GraphicsStorage::objs, GraphicsStorage::paths["resources"] + lineHeader));
	}
	for (auto& th : threadPool)
	{
		th.join();
	}
	fclose(file);
	return true;
}

bool GraphicsManager::LoadOBJs(std::unordered_map<std::string, std::string>& meshes)
{
	std::vector<std::thread> threadPool;
	for (auto namePath : meshes)
	{
		threadPool.push_back(std::thread(LoadOBJ, &GraphicsStorage::objs, namePath.second));
	}
	for (auto& th : threadPool)
	{
		th.join();
	}
	return true;
}

void GraphicsManager::LoadOBJ(std::unordered_map<std::string, OBJ*>* objs, std::string path)
{
	OBJ* tempOBJ = new OBJ();
	bool res = tempOBJ->LoadAndIndexOBJ(path.c_str());
	if (res)
	{
		tempOBJ->name = path;
		size_t sep = tempOBJ->name.find_last_of("\\/");

		if (sep != std::string::npos)
			tempOBJ->name = tempOBJ->name.substr(sep + 1, tempOBJ->name.size() - sep - 1);

		size_t dot = tempOBJ->name.find_last_of(".");
		if (dot != std::string::npos)
		{
			tempOBJ->name = tempOBJ->name.substr(0, dot);
		}
		std::lock_guard<std::mutex> lock(objLoadMutex);
		(*objs)[tempOBJ->name] = tempOBJ;
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
		ss << obj->indexed_vertices.at(i).vect[0];
		ss << " ";
		ss << obj->indexed_vertices.at(i).vect[1];
		ss << " ";
		ss << obj->indexed_vertices.at(i).vect[2];
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
		ss << obj->indexed_uvs.at(i).vect[0];
		ss << " ";
		ss << obj->indexed_uvs.at(i).vect[1];
		ss << " ";
		ss << obj->indexed_uvs.at(i).vect[2];
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
		ss << obj->indexed_normals.at(i).vect[0];
		ss << " ";
		ss << obj->indexed_normals.at(i).vect[1];
		ss << " ";
		ss << obj->indexed_normals.at(i).vect[2];
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
		threadPool.push_back(std::thread(LoadTextureInfo, &GraphicsStorage::texturesToLoad, GraphicsStorage::paths["resources"] + texturePath, 0));
		//LoadTexture(texturePath);
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
	std::string fullName = path;
	size_t pos = fullName.find_last_of(".");
	std::string ext = fullName.substr(pos + 1, fullName.length() - pos);
	size_t fileNameStart = fullName.find_last_of("/\\");
	std::string fileName = fullName.substr(fileNameStart + 1, pos - fileNameStart - 1);
	std::lock_guard<std::mutex> lock(tiLoadMutex);
	texturesToLoad->insert({ fileName , ti });
	printf("Finished loading texture: %s\n", fullName.c_str());
}

void GraphicsManager::LoadTexturesIntoGPU(std::unordered_map<std::string, TextureInfo*>& texturesToLoad)
{
	for (auto& fti : texturesToLoad)
	{
		LoadTextureIntoGPU(fti.first.c_str(), fti.second);
	}
	texturesToLoad.clear();
}

Texture* GraphicsManager::LoadTextureIntoGPU(const char* fileName, TextureInfo* ti)
{
	Texture* tex = nullptr;
	if (ti != nullptr)
	{
		DDSExtraInfo* ei = NULL;
		int internalFormat;
		unsigned int format;
		switch (ti->type)
		{
		case HDR:
			tex = new Texture(GL_TEXTURE_2D, 0, GL_RGB16F, ti->width, ti->height, GL_RGB, GL_FLOAT, ti->data, GL_COLOR_ATTACHMENT0);
			tex->GenerateBindSpecify();
			tex->name = fileName;
			GraphicsStorage::textures[tex->name] = tex;
			break;
		case DDS:
			ei = (DDSExtraInfo*)ti->extraInfo;
			for (int i = 0; i < ei->nrOfCubeMapFaces; i++)
			{
				if (ei->compressed)
				{
					tex = new Texture(GL_TEXTURE_2D, 0, GL_RGB, ti->width, ti->height, GL_RGB, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
					LoadCompressedDDS(tex, ti, i);
					std::string name = fileName;
					if (i > 0) name = fileName + std::to_string(i);
					tex->name = name;
					GraphicsStorage::textures[tex->name] = tex;
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
					tex = new Texture(GL_TEXTURE_2D, 0, internalFormat, ti->width, ti->height, format, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
					tex->pixels = &((unsigned char*)ti->data)[i * ti->height * ti->width * ti->numOfElements];
					tex->GenerateBindSpecify();
					std::string name = fileName;
					if (i > 0) name = fileName + std::to_string(i);
					tex->name = name;
					GraphicsStorage::textures[tex->name] = tex;
				}
				if (!tex->hasMipMaps) tex->GenerateMipMaps();
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
			tex = new Texture(GL_TEXTURE_2D, 0, internalFormat, ti->width, ti->height, format, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
			tex->GenerateBindSpecify();
			tex->GenerateMipMaps();
			tex->name = fileName;
			GraphicsStorage::textures[tex->name] = tex;
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
		FILE* texturesFile;
		std::string texturesFilePath = GraphicsStorage::paths["resources"] + cubemapDirectory;
		texturesFile = fopen(texturesFilePath.c_str(), "r");
		if (texturesFile == NULL) {
			printf("%s could not be opened.\n", texturesFilePath.c_str());
			return false;
		}
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
		
		printf("Loading cubemap: %s\n", cubemapDirectory);
		LoadCubeMap(texturesPaths);
		fclose(texturesFile);
	}
	fclose(file);
	return true;
}

void GraphicsManager::GetFileNames(std::vector<std::string>& out, const char* directory, const char* extension)
{
	DIR* dir;
	struct dirent* ent;

	if ((dir = opendir(directory)) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			if (ent->d_type == DT_REG) {
				std::string fileFullName = ent->d_name;
				size_t pos = fileFullName.find_last_of(".");
				std::string fileNameExt = fileFullName.substr(pos + 1, fileFullName.length() - 1);
				if (strcmp("*", extension) == 0 || strcmp(fileNameExt.c_str(), extension) == 0)
				{
					std::string fileName = fileFullName.substr(0, pos);
					out.push_back(fileName);
				}
			}
		}
		closedir(dir);
	}
	else {
		/* could not open directory */
		perror("could not open directory");
	}
}

void GraphicsManager::RemoveComments(std::string& shaderCode)
{
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
}

bool GraphicsManager::LoadShaders(const char* path)
{
	//GraphicsStorage::ClearShaders();
	GraphicsStorage::shaderPaths = LoadShadersPaths(LoadShadersFiles(path));

	for (auto& program : GraphicsStorage::shaderPaths)
	{
		ReloadShaderFromPath(program.first.c_str(), GraphicsStorage::shaderPaths[program.first]);
	}

	return true;
}

std::unordered_map<std::string, std::string> GraphicsManager::LoadShadersFiles(const char* path)
{
	FILE* file;
	file = fopen(path, "r");
	if (file == NULL) {
		printf("%s could not be opened.\n", path);
		return std::unordered_map<std::string, std::string>();
	}
	char line[128];
	std::unordered_map<std::string, std::string> shaders;
	while (fgets(line, sizeof(line), file)) {
		if (line[0] == '/' || line[0] == ';' || line[0] == '#') continue; // ignore comment line
		char programName[128];
		char filePath[128];
		int matches = sscanf(line, "%s %s", programName, filePath);
		if (matches != 2)
		{
			printf("Wrong shader information!\n");
		}
		else {
			shaders[programName] = GraphicsStorage::paths["resources"] + filePath;
		}
	}
	fclose(file);
	return shaders;
}

std::unordered_map<std::string, ShaderPaths> GraphicsManager::LoadShadersPaths(std::unordered_map<std::string, std::string>& shaders)
{

	DIR* dir;
	struct dirent* ent;
	std::unordered_map<std::string, ShaderPaths> shaderPaths;
	for (auto& shader : shaders)
	{
		size_t pos = shader.second.find_last_of("/");
		std::string shaderPath = shader.second.substr(0, pos + 1);
		std::string name = shader.second.substr(pos + 1, shader.second.length() - 1);
		bool shaderFound = false;
		if ((dir = opendir(shaderPath.c_str())) != NULL) {
			/* print all the files and directories within directory */
			while ((ent = readdir(dir)) != NULL) {
				switch (ent->d_type) {
				case DT_REG:
				{
					std::string currentShaderFullName = ent->d_name;
					size_t pos = currentShaderFullName.find_last_of(".");
					std::string currentShaderName = currentShaderFullName.substr(0, pos);
					std::string currentShaderExt = currentShaderFullName.substr(pos + 1, currentShaderFullName.length() - 1);
					if (strcmp(name.c_str(), currentShaderName.c_str()) == 0)
					{
						//printf("%s\n", ent->d_name);
						if (strcmp(currentShaderExt.c_str(), "fs") == 0)
						{
							shaderPaths[shader.first].fs = shaderPath + currentShaderFullName;
							shaderFound = true;
						}
						else if (strcmp(currentShaderExt.c_str(), "vs") == 0)
						{
							shaderPaths[shader.first].vs = shaderPath + currentShaderFullName;
							shaderFound = true;
						}
						else if (strcmp(currentShaderExt.c_str(), "gs") == 0)
						{
							shaderPaths[shader.first].gs = shaderPath + currentShaderFullName;
							shaderFound = true;
						}
						else
						{
							printf("wrong shader file extension: %s\n", currentShaderExt.c_str());
						}

					}
					break;
				}
				case DT_DIR:
					//printf("%s/\n", ent->d_name);
					break;

				case DT_LNK:
					//printf("%s@\n", ent->d_name);
					break;

				default:
				{
					//printf("%s*\n", ent->d_name);
				}
				}
			}
			closedir(dir);
			if (!shaderFound)
			{
				printf("\nSHADER PROGRAM: %s not found in directory: %s !\n", name.c_str(), shaderPath.c_str());
			}
		}
		else {
			/* could not open directory */
			perror("could not open directory");
		}
	}
	return shaderPaths;
}

ShaderPaths GraphicsManager::LoadShaderPaths(std::string& path)
{
	DIR* dir;
	struct dirent* ent;
	ShaderPaths shaderPaths;
	
	size_t pos = path.find_last_of("/");
	std::string shaderPath = path.substr(0, pos + 1);
	std::string name = path.substr(pos + 1, path.length() - 1);
	bool shaderFound = false;
	if ((dir = opendir(shaderPath.c_str())) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			switch (ent->d_type) {
			case DT_REG:
			{
				std::string currentShaderFullName = ent->d_name;
				size_t pos = currentShaderFullName.find_last_of(".");
				std::string currentShaderName = currentShaderFullName.substr(0, pos);
				std::string currentShaderExt = currentShaderFullName.substr(pos + 1, currentShaderFullName.length() - 1);
				if (strcmp(name.c_str(), currentShaderName.c_str()) == 0)
				{
					//printf("%s\n", ent->d_name);
					if (strcmp(currentShaderExt.c_str(), "fs") == 0)
					{
						shaderPaths.fs = shaderPath + currentShaderFullName;
						shaderFound = true;
					}
					else if (strcmp(currentShaderExt.c_str(), "vs") == 0)
					{
						shaderPaths.vs = shaderPath + currentShaderFullName;
						shaderFound = true;
					}
					else if (strcmp(currentShaderExt.c_str(), "gs") == 0)
					{
						shaderPaths.gs = shaderPath + currentShaderFullName;
						shaderFound = true;
					}
					else
					{
						printf("wrong shader file extension: %s\n", currentShaderExt.c_str());
					}

				}
				break;
			}
			case DT_DIR:
				//printf("%s/\n", ent->d_name);
				break;

			case DT_LNK:
				//printf("%s@\n", ent->d_name);
				break;

			default:
			{
				//printf("%s*\n", ent->d_name);
			}
			}
		}
		closedir(dir);
		if (!shaderFound)
		{
			printf("\nSHADER PROGRAM: %s not found in directory: %s !\n", name.c_str(), shaderPath.c_str());
		}
	}
	else {
		/* could not open directory */
		perror("could not open directory");
	}
	return shaderPaths;
}

bool GraphicsManager::ReloadShaders()
{
	return LoadShaders("config/shaders.txt");
}

bool GraphicsManager::ReloadShader(const char* name)
{
	ShaderPaths spath = LoadShaderPaths(LoadShadersFiles("config/shaders.txt")[name]);
	if (!spath.fs.empty() || !spath.vs.empty() || !spath.gs.empty())
	{
		auto& shaderPaths = GraphicsStorage::shaderPaths[name] = spath;

		return ReloadShaderFromPath(name, shaderPaths);
	}
	return false;
}

bool GraphicsManager::ReloadShaderFromPath(const char* name, ShaderPaths& paths)
{
	printf("\033[1;36m\nLoading Shader: %s\033[0m", name);
	std::string& VertexShaderCode = ReadTextFileIntoString(paths.vs);
	std::string& FragmentShaderCode = ReadTextFileIntoString(paths.fs);
	std::string& GeometryShaderCode = ReadTextFileIntoString(paths.gs);
	RemoveComments(VertexShaderCode);
	RemoveComments(FragmentShaderCode);
	RemoveComments(GeometryShaderCode);
	unsigned int result = LoadProgram(VertexShaderCode, FragmentShaderCode, GeometryShaderCode);
	if (result > 0)
	{
		if (GraphicsStorage::shaders.find(name) == GraphicsStorage::shaders.end())
		{
			Shader* shader = new Shader(result, std::string(name), paths);
			GraphicsStorage::shaders[name] = shader;
		}
		else
		{
			glDeleteProgram(GraphicsStorage::shaderIDs[name]);
			GraphicsStorage::shaders[name]->shaderID = result;
			GraphicsStorage::shaders[name]->name = name;
			GraphicsStorage::shaders[name]->shaderPaths = paths;
		}
		
		GraphicsStorage::shaderIDs[name] = result;

		Shader& shader = *GraphicsStorage::shaders[name];
		shader.ClearBuffers();
		shader.outputs.clear();

		BlockType type = BlockType::Uniform;
		LoadBlocks(&shader, type);
		type = BlockType::Storage;
		LoadBlocks(&shader, type);
		//ParseShaderForUniformBuffers(VertexShaderCode, shader);
		//ParseShaderForUniformBuffers(FragmentShaderCode, shader);
		//ParseShaderForUniformBuffers(GeometryShaderCode, shader);
		ParseShaderForOutputs(FragmentShaderCode, shader);
		//shader.LoadUniforms();
		//shader.LoadBlocks(uniform);
		//LoadUniforms(result);
		return true;
	}
	else
	{
		printf("\033[1;31mFailed to load shader: %s\033[0m\n", name);
		return false;
	}
}

void GraphicsManager::LoadUniforms(GLuint programID)
{
	GLint i;
	GLint count;

	GLint size; // size of the variable
	GLenum type; // type of the variable (float, vec3 or mat4, etc)

	const GLsizei bufSize = 64; // maximum name length
	GLchar name[bufSize]; // variable name in GLSL
	std::string readableType; // variable name in GLSL
	GLsizei length; // name length
	glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &count);
	std::vector<std::string> samplers;
	for (i = 0; i < count; i++)
	{
		glGetActiveUniform(programID, (GLuint)i, bufSize, &length, &size, &type, name);

		
		switch (type)
		{
		case GL_SAMPLER_1D:
		{
			readableType = "sampler 1D";
			samplers.push_back(name);
			break;
		}
		case GL_SAMPLER_2D:
		{
			readableType = "sampler 2D";
			samplers.push_back(name);
			break;
		}
		case GL_SAMPLER_3D:
		{
			readableType = "sampler 3D";
			samplers.push_back(name);
			break;
		}
		case GL_SAMPLER_CUBE:
		{
			readableType = "sampler CUBE";
			samplers.push_back(name);
			break;
		}
		default:
		{
			readableType = "other";
			break;
		}
		}
		printf("Uniform #%d Type: %s Name: %s\n", i, readableType.c_str(), name); //we get type and name, number is irrelevant, it's just a count, we should get the id ourselves using the name, done :D

	}
	std::map<int, std::string> samplersOrdered;
	for (auto& samplerName : samplers)
	{
		GLuint samplerLocation = glGetUniformLocation(programID, samplerName.c_str());
		samplersOrdered[samplerLocation] = samplerName;
	}
	for (auto& sampler : samplersOrdered)
	{
		//printf("id %d Name: %s\n", sampler.first, sampler.second.c_str()); //ordered samplers with location ids
	}
}

void GraphicsManager::LoadBlocks(Shader* shader, BlockType& type)
{
	GLenum blockInterface;
	GLenum resourceInterface;
	int nrOfVariableProperties;

	switch (type)
	{
	case Uniform:
		blockInterface = GL_UNIFORM_BLOCK;
		resourceInterface = GL_UNIFORM;
		nrOfVariableProperties = 10;
		break;
	case Storage:
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
	const GLenum shaderReferences[6] = { GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_TESS_CONTROL_SHADER, GL_REFERENCED_BY_TESS_EVALUATION_SHADER, GL_REFERENCED_BY_GEOMETRY_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER, GL_REFERENCED_BY_COMPUTE_SHADER };
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
		if (type == Uniform) shaderBlock = GraphicsStorage::GetUniformBuffer(blockPropertyValues[2]);
		else shaderBlock = GraphicsStorage::GetShaderStorageBuffer(blockPropertyValues[2]);
		if (shaderBlock == nullptr)
		{
			shaderBlock = new ShaderBlock(blockPropertyValues[1], blockPropertyValues[2], type);
			shaderBlock->name = std::string(blockName.begin(), blockName.end() - 1);
			if (type == Uniform) GraphicsStorage::uniformBuffers.push_back(shaderBlock);
			else GraphicsStorage::shaderStorageBuffers.push_back(shaderBlock);
		}

		if (type == Uniform)
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

			switch (values[1])
			{
			case GL_FLOAT:				uniform_info.size = 4; break;
			case GL_FLOAT_VEC2:			uniform_info.size = 4 * 2; break;
			case GL_FLOAT_VEC3:			uniform_info.size = 4 * 3; break;
			case GL_FLOAT_VEC4:			uniform_info.size = 4 * 4; break;
			case GL_DOUBLE:				uniform_info.size = 2 * 4; break;
			case GL_DOUBLE_VEC2:		uniform_info.size = 2 * 4 * 2; break;
			case GL_DOUBLE_VEC3:		uniform_info.size = 2 * 4 * 3; break;
			case GL_DOUBLE_VEC4:		uniform_info.size = 2 * 4 * 4; break;
			case GL_INT:				uniform_info.size = 4; break;
			case GL_INT_VEC2:			uniform_info.size = 4 * 2; break;
			case GL_INT_VEC3:			uniform_info.size = 4 * 3; break;
			case GL_INT_VEC4:			uniform_info.size = 4 * 4; break;
			case GL_UNSIGNED_INT:		uniform_info.size = 4; break;
			case GL_UNSIGNED_INT_VEC2:	uniform_info.size = 4 * 2; break;
			case GL_UNSIGNED_INT_VEC3:	uniform_info.size = 4 * 3; break;
			case GL_UNSIGNED_INT_VEC4:	uniform_info.size = 4 * 4; break;
			case GL_BOOL:				uniform_info.size = 4; break;
			case GL_BOOL_VEC2:			uniform_info.size = 4 * 2; break;
			case GL_BOOL_VEC3:			uniform_info.size = 4 * 3; break;
			case GL_BOOL_VEC4:			uniform_info.size = 4 * 4; break;
			case GL_FLOAT_MAT2:			uniform_info.size = 4 * 2 * 2; break;
			case GL_FLOAT_MAT3:			uniform_info.size = 4 * 3 * 3; break;
			case GL_FLOAT_MAT4:			uniform_info.size = 4 * 4 * 4; break;
			case GL_FLOAT_MAT2x3:		uniform_info.size = 4 * 2 * 3; break;
			case GL_FLOAT_MAT2x4:		uniform_info.size = 4 * 2 * 4; break;
			case GL_FLOAT_MAT3x2:		uniform_info.size = 4 * 3 * 2; break;
			case GL_FLOAT_MAT3x4:		uniform_info.size = 4 * 3 * 4; break;
			case GL_FLOAT_MAT4x2:		uniform_info.size = 4 * 4 * 2; break;
			case GL_FLOAT_MAT4x3:		uniform_info.size = 4 * 4 * 3; break;
			case GL_DOUBLE_MAT2:		uniform_info.size = 2 * 4 * 2 * 2; break;
			case GL_DOUBLE_MAT3:		uniform_info.size = 2 * 4 * 3 * 3; break;
			case GL_DOUBLE_MAT4:		uniform_info.size = 2 * 4 * 4 * 4; break;
			case GL_DOUBLE_MAT2x3:		uniform_info.size = 2 * 4 * 2 * 3; break;
			case GL_DOUBLE_MAT2x4:		uniform_info.size = 2 * 4 * 2 * 4; break;
			case GL_DOUBLE_MAT3x2:		uniform_info.size = 2 * 4 * 3 * 2; break;
			case GL_DOUBLE_MAT3x4:		uniform_info.size = 2 * 4 * 3 * 4; break;
			case GL_DOUBLE_MAT4x2:		uniform_info.size = 2 * 4 * 4 * 2; break;
			case GL_DOUBLE_MAT4x3:		uniform_info.size = 2 * 4 * 4 * 3; break;
			default:
				break;
			}

			uniforms.emplace(std::make_pair(std::string(uniformName.begin(), uniformName.end() - 1), uniform_info));
			shaderBlock->AddVariableOffset(std::string(uniformName.begin(), uniformName.end() - 1), uniform_info.offset);
		}
	}
}

Texture* GraphicsManager::LoadDDS(const char* path) {

	unsigned char header[124];

	FILE* fp;

	/* try to open the file */
	fp = fopen(path, "rb");
	if (fp == NULL) {
		printf("%s could not be opened.\n", path);
		return 0;
	}

	/* verify the type of file */
	char filecode[4];
	fread(filecode, 1, 4, fp);
	if (strncmp(filecode, "DDS ", 4) != 0) {
		fclose(fp);
		return 0;
	}

	/* get the surface desc */
	fread(&header, 124, 1, fp);

	unsigned int height = *(unsigned int*)&(header[8]);
	unsigned int width = *(unsigned int*)&(header[12]);
	unsigned int linearSize = *(unsigned int*)&(header[16]);
	unsigned int mipMapCount = *(unsigned int*)&(header[24]);
	unsigned int fourCC = *(unsigned int*)&(header[80]);


	unsigned char* buffer;
	unsigned int bufsize;
	/* how big is it going to be including all mipmaps? */
	bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;
	buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char));
	fread(buffer, 1, bufsize, fp);
	/* close the file pointer */
	fclose(fp);

	unsigned int components = (fourCC == FOURCC_DXT1) ? 3 : 4;
	unsigned int format;
	switch (fourCC)
	{
	case FOURCC_DXT1:
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case FOURCC_DXT3:
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case FOURCC_DXT5:
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	default:
		free(buffer);
		return nullptr;
	}

	Texture* texture = new Texture(GL_TEXTURE_2D, 0, format, width, height, components == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, buffer, GL_COLOR_ATTACHMENT0);
	texture->Generate();
	texture->Bind();
	texture->hasMipMaps = mipMapCount > 1;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
	unsigned int offset = 0;

	/* load the mipmaps */
	for (unsigned int level = 0; level < mipMapCount && (width || height); ++level)
	{
		unsigned int size = ((width + 3) / 4) * ((height + 3) / 4) * blockSize;
		glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, size, buffer + offset);

		offset += size;
		width /= 2;
		height /= 2;

		// Deal with Non-Power-Of-Two textures.
		if (width < 1) width = 1;
		if (height < 1) height = 1;
	}
	
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glGenerateMipmap(GL_TEXTURE_2D);
	
	//float aniso = 16.0f;
	//glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
	
	free(buffer);

	return texture;
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

std::string GraphicsManager::ReadTextFileIntoString(std::string& path)
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

void GraphicsManager::ParseShaderForUniformBuffers(std::string& shaderCode, Shader& shader)
{
	std::vector<size_t> startPositions;
	std::vector<size_t> endPositions;
	std::vector<size_t> indexes;
	std::vector<size_t> sizes;
	std::vector<std::string> layouts;
	std::vector<std::string> names;
	size_t uniformBufferPos = 0;
	size_t uniformEndPos = 0;
	size_t uniformStartPos = 0;

	while (uniformBufferPos != std::string::npos)
	{
		size_t tempPos = shaderCode.find("std140", uniformBufferPos);
		if (tempPos == std::string::npos)
		{
			tempPos = shaderCode.find("std430", uniformBufferPos);
			if (tempPos == std::string::npos) break;
			else {
				layouts.push_back("std430");
				uniformBufferPos = tempPos;
			}
		}
		else
		{
			layouts.push_back("std140");
			uniformBufferPos = tempPos;
		}
		
		uniformStartPos = shaderCode.find("{", uniformBufferPos);
		startPositions.push_back(uniformStartPos);
		uniformEndPos = shaderCode.find("}", uniformStartPos);
		endPositions.push_back(uniformEndPos);

		if ((uniformBufferPos = shaderCode.find("binding", uniformBufferPos)) != std::string::npos)
		{
			uniformBufferPos = shaderCode.find("=", uniformBufferPos);
			std::string uniformBufferCode = shaderCode.substr(uniformBufferPos + 1, uniformStartPos - (uniformBufferPos + 1));
			std::stringstream ss(uniformBufferCode);
			ss.seekg(0, ss.beg);
			/* Running loop till the end of the stream */
			std::string temp;
			int found = -1;
			while (!ss.eof()) {

				/* extracting word by word from stream */
				ss >> temp;

				/* Checking the given word is integer or not */
				if (std::stringstream(temp) >> found)
				{
					indexes.push_back(found);
					break;
				}
			}
			int foundUniform = false;
			while (ss.cur != uniformStartPos) {

				/* extracting word by word from stream */
				ss >> temp;
				if (temp == "uniform")
				{
					//store uniform name
					ss >> temp;
					names.push_back(temp);
					break;
				}
			}
		}
	}

	if (indexes.size() == 0) return;

	for (int i = 0; i < indexes.size(); i++)
	{
		printf("uniform buffer index: %d\n", (int)indexes[i]);
	}

	if (startPositions.size() > 0)
	{
		std::stringstream ss(shaderCode);
		std::string token;
		for (size_t i = 0; i < startPositions.size(); i++)
		{
			if (!layouts[i].compare("std140")) {
				if (shader.HasUniformBuffer(indexes[i])) continue;
			}
			else if (!layouts[i].compare("std430")) {
				if (shader.HasShaderStorageBuffer(indexes[i])) continue;
			}

			std::vector<std::string> uniforms;
			ss.seekg(startPositions[i], ss.beg);
			std::string uniformBufferCode = shaderCode.substr(startPositions[i], endPositions[i] - startPositions[i]);
			replaceAllCharacters(uniformBufferCode, "\n\r\t{}", "");
			splitLine(uniformBufferCode, uniforms, std::string(";"));

			std::regex ws_re("\\s+");
			int bufferSize = 0; //will be used also as location
			std::unordered_map<std::string, unsigned int> locations;
			for (auto& uniform : uniforms)
			{
				std::vector<std::string> uniformPair{
					std::sregex_token_iterator(uniform.begin(), uniform.end(), ws_re, -1), {}
				};
				int typeIndex = 0;

				if (uniformPair.size() > 2)
				{
					typeIndex = 1;
				}
				int numberOfArrayElements = 0;
				std::string cleanName = uniformPair[typeIndex + 1];
				size_t arrayStart = uniformPair[typeIndex + 1].find("[");
				if (arrayStart != std::string::npos)
				{
					size_t arrayEnd = uniformPair[typeIndex + 1].find("]");
					std::string num = uniformPair[typeIndex + 1].substr(arrayStart + 1, arrayEnd - (arrayStart + 1));
					replaceAllCharacters(num, " \n\r\t[]", "");
					std::stringstream(num) >> numberOfArrayElements;
					cleanName = uniformPair[typeIndex + 1].substr(0, arrayStart);
					replaceAllCharacters(cleanName, " \n\r\t[]", "");
				}
				
				if (!uniformPair[typeIndex].compare("mat4"))
				{
					int offset = (16 - (bufferSize % 16)) % 16;
					bufferSize += offset;
					locations[uniformPair[typeIndex + 1]] = bufferSize;
					if (numberOfArrayElements == 0) bufferSize += 64;
					else bufferSize += numberOfArrayElements * 64;
				}
				else if (!uniformPair[typeIndex].find("mat3"))
				{
					int offset = (16 - (bufferSize % 16)) % 16;
					bufferSize += offset;
					locations[uniformPair[typeIndex + 1]] = bufferSize;
					if (numberOfArrayElements == 0) bufferSize += 64;
					else bufferSize += numberOfArrayElements * 64;
				}
				else if (!uniformPair[typeIndex].find("mat2"))
				{
					int offset = (16 - (bufferSize % 16)) % 16;
					bufferSize += offset;
					locations[uniformPair[typeIndex + 1]] = bufferSize;
					if (numberOfArrayElements == 0) bufferSize += 64;
					else bufferSize += numberOfArrayElements * 64;
				}
				else if (!uniformPair[typeIndex].find("vec4"))
				{
					int offset = (16 - (bufferSize % 16)) % 16;
					bufferSize += offset;
					locations[uniformPair[typeIndex + 1]] = bufferSize;
					if (numberOfArrayElements == 0) bufferSize += 16;
					else bufferSize += numberOfArrayElements * 16;
				}
				else if (!uniformPair[typeIndex].find("vec3"))
				{
					int offset = (16 - (bufferSize % 16)) % 16;
					bufferSize += offset;
					locations[uniformPair[typeIndex + 1]] = bufferSize;
					if (numberOfArrayElements == 0) bufferSize += 12;
					else bufferSize += (numberOfArrayElements * 16);
				}
				else if (!uniformPair[typeIndex].find("vec2"))
				{
					int offset = (8 - (bufferSize % 8)) % 8;
					bufferSize += offset;
					locations[uniformPair[typeIndex + 1]] = bufferSize;
					if (numberOfArrayElements == 0) bufferSize += 8;
					else bufferSize += (numberOfArrayElements * 16);
				}
				else if (!uniformPair[typeIndex].find("float"))
				{
					int offset = (4 - (bufferSize % 4)) % 4;
					bufferSize += offset;
					locations[uniformPair[typeIndex + 1]] = bufferSize;
					if (numberOfArrayElements == 0) bufferSize += 4;
					else {
						if (!layouts[i].compare("std140")) {
							bufferSize += (numberOfArrayElements * 16);
						}
						else if (!layouts[i].compare("std430")) {
							bufferSize += (numberOfArrayElements * 4);
						}
					}
				}
				else if (!uniformPair[typeIndex].find("int"))
				{
					int offset = (4 - (bufferSize % 4)) % 4;
					bufferSize += offset;
					locations[uniformPair[typeIndex + 1]] = bufferSize;
					if (numberOfArrayElements == 0) bufferSize += 4;
					else {
						if (!layouts[i].compare("std140")) {
							bufferSize += (numberOfArrayElements * 16);
						}
						else if (!layouts[i].compare("std430")) {
							bufferSize += (numberOfArrayElements * 4);
						}
					}
				}
				else if (!uniformPair[typeIndex].find("uint"))
				{
					int offset = (4 - (bufferSize % 4)) % 4;
					bufferSize += offset;
					locations[uniformPair[typeIndex + 1]] = bufferSize;
					if (numberOfArrayElements == 0) bufferSize += 4;
					else {
						if (!layouts[i].compare("std140")) {
							bufferSize += (numberOfArrayElements * 16);
						}
						else if (!layouts[i].compare("std430")) {
							bufferSize += (numberOfArrayElements * 4);
						}
					}
				}
				else if (!uniformPair[typeIndex].find("bool"))
				{
					int offset = (4 - (bufferSize % 4)) % 4;
					bufferSize += offset;
					locations[uniformPair[typeIndex + 1]] = bufferSize;
					if (numberOfArrayElements == 0) bufferSize += 4;
					else {
						if (!layouts[i].compare("std140")) {
							bufferSize += (numberOfArrayElements * 16);
						}
						else if (!layouts[i].compare("std430")) {
							bufferSize += (numberOfArrayElements * 4);
						}
					}
				}
			}
			std::string uniformBufferType = names[i].substr(0, 2);
			if (!layouts[i].compare("std140")) {
				ShaderBlock* uniformBuffer = GraphicsStorage::GetUniformBuffer(indexes[i]);
				if (uniformBuffer == nullptr)
				{
					uniformBuffer = new ShaderBlock(bufferSize, indexes[i], BlockType::Uniform);
					uniformBuffer->name = names[i];
					for (auto& uniformLocation : locations)
					{
						uniformBuffer->AddVariableOffset(uniformLocation.first, uniformLocation.second);
					}
					GraphicsStorage::uniformBuffers.push_back(uniformBuffer);
				}
				
				if (uniformBufferType == "O_") { shader.objectUniformBuffers.push_back(uniformBuffer); }
				else if (uniformBufferType == "M_") { shader.materialUniformBuffers.push_back(uniformBuffer); }
				else if (uniformBufferType == "G_") { shader.globalUniformBuffers.push_back(uniformBuffer); }
				else printf("\nwrong uniform buffer name: %s\nuniform buffer name must start with O_ M_ or G_", names[i].c_str());
			}
			else if (!layouts[i].compare("std430")) {
				ShaderBlock* shaderStorageBuffer = GraphicsStorage::GetShaderStorageBuffer(indexes[i]);
				if (shaderStorageBuffer == nullptr)
				{
					shaderStorageBuffer = new ShaderBlock(bufferSize, indexes[i], BlockType::Storage);
					shaderStorageBuffer->name = names[i];
					for (auto& uniformLocation : locations)
					{
						shaderStorageBuffer->AddVariableOffset(uniformLocation.first, uniformLocation.second);
					}
					GraphicsStorage::shaderStorageBuffers.push_back(shaderStorageBuffer);
				}
				if (uniformBufferType == "O_") shader.objectShaderStorageBuffers.push_back(shaderStorageBuffer);
				else if (uniformBufferType == "M_") shader.materialShaderStorageBuffers.push_back(shaderStorageBuffer);
				else if (uniformBufferType == "G_") shader.globalShaderStorageBuffers.push_back(shaderStorageBuffer);
				else printf("\nwrong shader storage name: %s\nshader storage name must start with O_ M_ or G_", names[i].c_str());
			}
		}
	}
}

void GraphicsManager::ParseShaderForOutputs(std::string& shaderCode, Shader& shader)
{
	size_t outPos = 0;
	std::vector<size_t> indexes;
	std::vector<std::string> types;
	std::vector<std::string> names;
	int defaultIndex = 0;
	while (outPos != std::string::npos)
	{
		size_t tempPos = shaderCode.find("out ", outPos);
		if (tempPos == std::string::npos) break;
		else
		{
			if (tempPos != 0)
			{
				if (shaderCode[tempPos - 1] == '\n' || shaderCode[tempPos - 1] == '\t' || shaderCode[tempPos - 1] == '\r' || shaderCode[tempPos - 1] == ' ')
				{
					outPos = tempPos;
				}
				else
				{
					break;
				}
			}
			else
			{
				outPos = tempPos;
			}
		}
		size_t previousLineEnd = shaderCode.rfind(";", outPos);
		if (previousLineEnd == std::string::npos) previousLineEnd = 0;
		size_t outLineEnd = shaderCode.find(";", outPos);
		std::string outCode = shaderCode.substr(previousLineEnd, outLineEnd - previousLineEnd);
		std::vector<std::string> codeWords;
		size_t layoutIndex = outCode.find("layout", 0);
		if (layoutIndex != std::string::npos)
		{
			size_t locationIndex = layoutIndex;
			if ((locationIndex = outCode.find("location", locationIndex)) != std::string::npos)
			{
				locationIndex = outCode.find("=", locationIndex);
				std::stringstream ss(outCode);
				ss.seekg(locationIndex + 1, ss.beg);
				/* Running loop till the end of the stream */
				std::string temp;
				int found = -1;
				while (!ss.eof()) {

					/* extracting word by word from stream */
					ss >> temp;

					/* Checking the given word is integer or not */
					if (std::stringstream(temp) >> found)
					{
						indexes.push_back(found);
						break;
					}
				}
			}
		}
		else
		{
			indexes.push_back(defaultIndex);
			defaultIndex++;
		}

		size_t spaceAfterOut = shaderCode.find(" ", outPos);
		std::string typeAndNameCode = shaderCode.substr(spaceAfterOut + 1, outLineEnd - (spaceAfterOut + 1));
		replaceAllCharacters(typeAndNameCode, "\n\r\t{};", "");
		std::vector<std::string> typeAndName;
		splitLine(typeAndNameCode, typeAndName);
		types.push_back(typeAndName[0]);
		names.push_back(typeAndName[1]);
		outPos = outLineEnd;
	}
	shader.outputs.resize(indexes.size());
	for (size_t i = 0; i < indexes.size(); i++)
	{
		ShaderOutput& output = shader.outputs[i];
		output.index = (int)indexes[i];
		output.type = types[i];
		output.name = names[i];
		printf("\033[1;32mout %d %s %s\033[0m\n", (int)indexes[i], types[i].c_str(), names[i].c_str());
	}
}

GLuint GraphicsManager::LoadProgram(std::string& VertexShaderCode, std::string& FragmentShaderCode, std::string& GeometryShaderCode) {

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

Vao* GraphicsManager::LoadOBJToVAO(OBJ* object, Vao* vao)
{
	//vao->Bind();
	//attribute index - attribute index location in the shader
	//binding index - index of the vbo, is it one and the same or multiple, decide where inside the vao you want to place it
	//outcommented is the old way of using state machine in opengl with glGen
	//current code is uses DSA - Direct State Access where we are not required to bind buffers in order to change them
	vao->vertexBuffers.reserve(4);
	///GLuint vertexBuffer;
	///GLuint bindingIndex = 0;
	///GLuint attributeIndex = 0;
	// 1rst attribute buffer : vertices
	///BufferLayout layout = { 
	///	LayoutLocation(ShaderDataType::Float3, "position"),
	///	{ ShaderDataType::Float2, "uv" }
	///};
	vao->AddVertexBuffer(&object->indexed_vertices[0], object->indexed_vertices.size() * sizeof(Vector3F), { {ShaderDataType::Float3, "position"} });
	///glCreateBuffers(1, &vertexBuffer);
	///glNamedBufferStorage(vertexBuffer, object->indexed_vertices.size() * sizeof(Vector3F), &object->indexed_vertices[0], GL_DYNAMIC_STORAGE_BIT);
	///glEnableVertexArrayAttrib(vao->handle, attributeIndex); //vao handle, attribute index, which attrib index to enable on this vao
	///glVertexArrayVertexBuffer(vao->handle, bindingIndex, vertexBuffer, 0, sizeof(Vector3F)); //vao handle, binding index, vbo handle, offset to first element, stride (distance between elements)
	///glVertexArrayAttribFormat(vao->handle, attributeIndex, 3, GL_FLOAT, GL_FALSE, 0); //vao handle, attribute index, values per element, type of data, normalized, relativeoffset - The distance between elements within the buffer.
	///glVertexArrayAttribBinding(vao->handle, attributeIndex, bindingIndex); //vao handle, attribute index, binding index
	///glVertexArrayBindingDivisor(vao->handle, bindingIndex, 0);
	//glGenBuffers(1, &vertexBuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	//glBufferData(GL_ARRAY_BUFFER, object->indexed_vertices.size() * sizeof(Vector3F), &object->indexed_vertices[0], GL_STATIC_DRAW);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute, size, type, normalized?, stride, array buffer offset
	//glEnableVertexAttribArray(0);
	///vao->vertexBuffers.push_back(vertexBuffer);

	///GLuint uvbuffer;
	///bindingIndex += 1;
	///attributeIndex += 1;
	// 2nd attribute buffer : UVs
	vao->AddVertexBuffer(&object->indexed_uvs[0], object->indexed_uvs.size() * sizeof(Vector2F), { {ShaderDataType::Float2, "uv"} });
	///glCreateBuffers(1, &uvbuffer);
	///glNamedBufferStorage(uvbuffer, object->indexed_uvs.size() * sizeof(Vector2F), &object->indexed_uvs[0], GL_DYNAMIC_STORAGE_BIT);
	///glEnableVertexArrayAttrib(vao->handle, attributeIndex); //vao handle, attribute index, which attrib index to enable on this vao
	///glVertexArrayVertexBuffer(vao->handle, bindingIndex, uvbuffer, 0, sizeof(Vector2F)); //vao handle, binding index, vbo handle, offset to first element, stride (distance between elements)
	///glVertexArrayAttribFormat(vao->handle, attributeIndex, 2, GL_FLOAT, GL_FALSE, 0); //vao handle, attribute index, values per element, type of data, normalized, relativeoffset - The distance between elements within the buffer.
	///glVertexArrayAttribBinding(vao->handle, attributeIndex, bindingIndex); //vao handle, attribute index, binding index
	///glVertexArrayBindingDivisor(vao->handle, bindingIndex, 0);
	//glGenBuffers(1, &uvbuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	//glBufferData(GL_ARRAY_BUFFER, object->indexed_uvs.size() * sizeof(Vector2F), &object->indexed_uvs[0], GL_STATIC_DRAW);
	//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute, size, type, normalized?, stride, array buffer offset
	//glEnableVertexAttribArray(1);
	///vao->vertexBuffers.push_back(uvbuffer);

	///GLuint normalbuffer;
	///bindingIndex += 1;
	///attributeIndex += 1;
	// 3rd attribute buffer : normals
	vao->AddVertexBuffer(&object->indexed_normals[0], object->indexed_normals.size() * sizeof(Vector3F), { {ShaderDataType::Float3, "normal"} });
	///glCreateBuffers(1, &normalbuffer);
	///glNamedBufferStorage(normalbuffer, object->indexed_normals.size() * sizeof(Vector3F), &object->indexed_normals[0], GL_DYNAMIC_STORAGE_BIT);
	///glEnableVertexArrayAttrib(vao->handle, attributeIndex); //vao handle, attribute index, which attrib index to enable on this vao
	///glVertexArrayVertexBuffer(vao->handle, bindingIndex, normalbuffer, 0, sizeof(Vector3F)); //vao handle, binding index, vbo handle, offset to first element, stride (distance between elements)
	///glVertexArrayAttribFormat(vao->handle, attributeIndex, 3, GL_FLOAT, GL_FALSE, 0); //vao handle, attribute index, values per element, type of data, normalized, relativeoffset - The distance between elements within the buffer.
	///glVertexArrayAttribBinding(vao->handle, attributeIndex, bindingIndex); //vao handle, attribute index, binding index
	///glVertexArrayBindingDivisor(vao->handle, bindingIndex, 0);
	//glGenBuffers(1, &normalbuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	//glBufferData(GL_ARRAY_BUFFER, object->indexed_normals.size() * sizeof(Vector3F), &object->indexed_normals[0], GL_STATIC_DRAW);
	//glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute, size, type, normalized?, stride, array buffer offset
	//glEnableVertexAttribArray(2);
	///vao->vertexBuffers.push_back(normalbuffer);

	if (object->indexed_tangents.size() > 0)
	{
		///GLuint tangentbuffer;
		///bindingIndex += 1;
		///attributeIndex += 1;
		vao->AddVertexBuffer(&object->indexed_tangents[0], object->indexed_tangents.size() * sizeof(Vector3F), { {ShaderDataType::Float3, "tangent"} });
		///glCreateBuffers(1, &tangentbuffer);
		///glNamedBufferStorage(tangentbuffer, object->indexed_tangents.size() * sizeof(Vector3F), &object->indexed_tangents[0], GL_DYNAMIC_STORAGE_BIT);
		///glEnableVertexArrayAttrib(vao->handle, attributeIndex); //vao handle, attribute index, which attrib index to enable on this vao
		///glVertexArrayVertexBuffer(vao->handle, bindingIndex, tangentbuffer, 0, sizeof(Vector3F)); //vao handle, binding index, vbo handle, offset to first element, stride (distance between elements)
		///glVertexArrayAttribFormat(vao->handle, attributeIndex, 3, GL_FLOAT, GL_FALSE, 0); //vao handle, attribute index, values per element, type of data, normalized, relativeoffset - The distance between elements within the buffer.
		///glVertexArrayAttribBinding(vao->handle, attributeIndex, bindingIndex); //vao handle, attribute index, binding index
		///glVertexArrayBindingDivisor(vao->handle, bindingIndex, 0);
		//glGenBuffers(1, &tangentbuffer);
		//glBindBuffer(GL_ARRAY_BUFFER, tangentbuffer);
		//glBufferData(GL_ARRAY_BUFFER, object->indexed_tangents.size() * sizeof(Vector3F), &object->indexed_tangents[0], GL_STATIC_DRAW);
		//glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute, size, type, normalized?, stride, array buffer offset
		//glEnableVertexAttribArray(3);
		///vao->vertexBuffers.push_back(tangentbuffer);

		///GLuint bitangentbuffer;
		///bindingIndex += 1;
		///attributeIndex += 1;
		vao->AddVertexBuffer(&object->indexed_bitangents[0], object->indexed_bitangents.size() * sizeof(Vector3F), { {ShaderDataType::Float3, "bitangent"} });
		///glCreateBuffers(1, &bitangentbuffer);
		///glNamedBufferStorage(bitangentbuffer, object->indexed_bitangents.size() * sizeof(Vector3F), &object->indexed_bitangents[0], GL_DYNAMIC_STORAGE_BIT);
		///glEnableVertexArrayAttrib(vao->handle, attributeIndex); //vao handle, attribute index, which attrib index to enable on this vao
		///glVertexArrayVertexBuffer(vao->handle, bindingIndex, bitangentbuffer, 0, sizeof(Vector3F)); //vao handle, binding index, vbo handle, offset to first element, stride (distance between elements)
		///glVertexArrayAttribFormat(vao->handle, attributeIndex, 3, GL_FLOAT, GL_FALSE, 0); //vao handle, attribute index, values per element, type of data, normalized, relativeoffset - The distance between elements within the buffer.
		///glVertexArrayAttribBinding(vao->handle, attributeIndex, bindingIndex); //vao handle, attribute index, binding index
		///glVertexArrayBindingDivisor(vao->handle, bindingIndex, 0);
		//glGenBuffers(1, &bitangentbuffer);
		//glBindBuffer(GL_ARRAY_BUFFER, bitangentbuffer);
		//glBufferData(GL_ARRAY_BUFFER, object->indexed_bitangents.size() * sizeof(Vector3F), &object->indexed_bitangents[0], GL_STATIC_DRAW);
		//glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute, size, type, normalized?, stride, array buffer offset
		//glEnableVertexAttribArray(4);
		///vao->vertexBuffers.push_back(bitangentbuffer);
	}

	///GLuint elementbuffer;
	// 4th element buffer Generate a buffer for the indices as well
	vao->AddIndexBuffer(&object->indices[0], object->indices.size(), IndicesType::UNSIGNED_INT);
	///glCreateBuffers(1, &elementbuffer);
	///glNamedBufferStorage(elementbuffer, object->indices.size() * sizeof(unsigned int), &object->indices[0], GL_DYNAMIC_STORAGE_BIT);
	///glVertexArrayElementBuffer(vao->handle, elementbuffer);
	//glGenBuffers(1, &elementbuffer);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, object->indices.size() * sizeof(unsigned int), &object->indices[0], GL_STATIC_DRAW);
	///vao->indicesCount = (unsigned int)object->indices.size();
	///vao->vertexBuffers.push_back(elementbuffer);
	///vao->indexBuffer = elementbuffer;

	//Unbind the VAO now that the VBOs have been set up
	//vao->Unbind();
	
	vao->center = object->center_of_mesh;
	vao->dimensions = object->dimensions;

	return vao;
}

void GraphicsManager::LoadAllOBJsToVAOs()
{
	for (auto& obj : GraphicsStorage::objs)
	{
		Vao* newVao = new Vao();
		newVao->name = obj.second->name;
		LoadOBJToVAO(obj.second, newVao);
		GraphicsStorage::vaos[obj.second->name] = newVao;
		//delete obj.second;
	}
	//GraphicsStorage::objs.clear();
}

Texture* GraphicsManager::CreateTexture(int width, int height, bool isDepth, int numOfElements, unsigned char* data) {

	Texture* texture = new Texture(GL_TEXTURE_2D, 0, isDepth ? GL_DEPTH_COMPONENT : (numOfElements == 3 ? GL_RGB : GL_RGBA), width, height, isDepth ? GL_DEPTH_COMPONENT : (numOfElements == 3 ? GL_RGB : GL_RGBA), isDepth ? GL_FLOAT : GL_UNSIGNED_BYTE, data, GL_COLOR_ATTACHMENT0);
	texture->Generate();
	texture->Bind();
	if (data)
	{
		texture->Specify();
		texture->GenerateMipMaps();
	}
	
	//if (data)
	//{
		//glTexImage2D(GL_TEXTURE_2D, 0, isDepth ? GL_DEPTH_COMPONENT : (numOfElements == 3 ? GL_RGB : GL_RGBA), width, height, isDepth ? GL_DEPTH_COMPONENT : (numOfElements == 3 ? GL_RGB : GL_RGBA), isDepth ? GL_FLOAT : GL_UNSIGNED_BYTE, data);
		//glGenerateMipmap(GL_TEXTURE_2D);
	//}

    // Poor filtering, or ...
	///glTexParameteri(texture->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	///glTexParameteri(texture->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // ... nice trilinear filtering.
	//glTexParameteri(texture->target, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(texture->target, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//glTexParameteri(texture->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(texture->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);


    return texture;
}

void GraphicsManager::LoadAllAssets()
{
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	std::chrono::duration<double> elapsed_seconds;

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
	LoadOBJs("config/models.txt");
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	printf("\nDONE Took %fs\n", elapsed_seconds.count());

	printf("\nLOADING OBJs TO VAOs\n");
	start = std::chrono::high_resolution_clock::now();
	LoadAllOBJsToVAOs();
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

Texture* GraphicsManager::LoadTexture(char* path)
{
	std::string fullName = path;
	size_t pos = fullName.find_last_of(".");
	std::string ext = fullName.substr(pos + 1, fullName.length() - pos);
	size_t fileNameStart = fullName.find_last_of("/\\");
	std::string fileName = fullName.substr(fileNameStart + 1, pos - fileNameStart - 1);
	bool stored = false;
	int x = 0, y = 0, numOfElements = 0;
	TextureInfo* ti = nullptr;
	Texture* tex = nullptr;
	
	ti = GraphicsManager::LoadImage(path, 0);
	if (ti != nullptr)
	{
		DDSExtraInfo* ei = NULL;
		int internalFormat;
		unsigned int format;
		switch (ti->type)
		{
		case HDR:
			tex = new Texture(GL_TEXTURE_2D, 0, GL_RGB16F, ti->width, ti->height, GL_RGB, GL_FLOAT, ti->data, GL_COLOR_ATTACHMENT0);
			tex->GenerateBindSpecify();
			tex->name = fileName;
			GraphicsStorage::textures[tex->name] = tex;
			break;
		case DDS:
			ei = (DDSExtraInfo*)ti->extraInfo;
			for (int i = 0; i < ei->nrOfCubeMapFaces; i++)
			{
				if (ei->compressed)
				{
					tex = new Texture(GL_TEXTURE_2D, 0, GL_RGB, ti->width, ti->height, GL_RGB, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
					LoadCompressedDDS(tex, ti, i);
					std::string name = fileName;
					if (i > 0) name = fileName + std::to_string(i);
					tex->name = name;
					GraphicsStorage::textures[tex->name] = tex;
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
					tex = new Texture(GL_TEXTURE_2D, 0, internalFormat, ti->width, ti->height, format, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
					tex->pixels = &((unsigned char*)ti->data)[i * ti->height * ti->width * ti->numOfElements];
					tex->GenerateBindSpecify();
					std::string name = fileName;
					if (i > 0) name = fileName + std::to_string(i);
					tex->name = name;
					GraphicsStorage::textures[tex->name] = tex;
				}
				if (!tex->hasMipMaps) tex->GenerateMipMaps();
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
			tex = new Texture(GL_TEXTURE_2D, 0, internalFormat, ti->width, ti->height, format, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
			tex->GenerateBindSpecify();
			tex->GenerateMipMaps();
			tex->name = fileName;
			GraphicsStorage::textures[tex->name] = tex;
			break;
		}
		SOIL_free_texture_info(ti);
		tex->SetDefaultParameters();
	}
	else
	{
		printf("%s could not be opened.\n", path);
	}
	return tex;
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

Texture* GraphicsManager::LoadCubeMap(const std::vector<std::string>& textures)
{
	int x = 0, y = 0, numOfElements = 0;
	Texture* tex = nullptr;
	
	if (textures.size() == 1)
	{
		TextureInfo* ti = GraphicsManager::LoadImage(textures.at(0).c_str(), 0);
		if (ti != nullptr)
		{
			DDSExtraInfo* ei = NULL;
			int internalFormat;
			unsigned int format;
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
					tex = new Texture(GL_TEXTURE_CUBE_MAP, 0, internalFormat, ti->width, ti->height, format, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
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

				tex = new Texture(GL_TEXTURE_CUBE_MAP, 0, internalFormat, ti->width, ti->height, format, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
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
				std::string fullName = textures.at(0);
				size_t pos = fullName.find_last_of("/\\");
				std::string fileName = fullName.substr(0, pos);
				pos = fileName.find_last_of("/\\");
				fileName = fileName.substr(pos + 1);
				tex->name = fileName;
				tex->SetDefaultParameters();
				GraphicsStorage::cubemaps[tex->name] = tex;
				SOIL_free_texture_info(ti);
			}
		}
		else
		{
			printf("%s could not be opened.\n", textures.at(0).c_str());
		}
	}
	else
	{
		if (textures.size() == 6)
		{
			TextureInfo* ti = GraphicsManager::LoadImage(textures.at(0).c_str(), 0);
			if (ti != nullptr)
			{
				DDSExtraInfo* ei = nullptr;
				int internalFormat;
				unsigned int format;
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

					tex = new Texture(GL_TEXTURE_CUBE_MAP, 0, internalFormat, ti->width, ti->height, format, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
					tex->Generate();
					tex->Bind();
					if (ei->compressed)
					{
						LoadCompressedDDSCubeMapFace(tex, ti, 0); //face0
						SOIL_free_texture_info(ti);
						for (size_t i = 1; i < 6; i++) //face 1-5
						{
							ti = GraphicsManager::LoadImage(textures.at(i).c_str(), 0);
							if (ti != nullptr)
							{
								LoadCompressedDDSCubeMapFace(tex, ti, (int)i);
								SOIL_free_texture_info(ti);
							}
							else
							{
								printf("%s could not be opened.\n", textures.at(i).c_str());
							}
						}
					}
					else
					{
						tex->SpecifyTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0, ti->width, ti->height, ti->data); //face0
						SOIL_free_texture_info(ti);
						for (int i = 1; i < 6; i++) //face 1-5
						{
							ti = GraphicsManager::LoadImage(textures.at(i).c_str(), 0);
							if (ti != nullptr)
							{
								tex->SpecifyTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, ti->width, ti->height, ti->data);
								SOIL_free_texture_info(ti);
							}
							else
							{
								printf("%s could not be opened.\n", textures.at(i).c_str());
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
					tex = new Texture(GL_TEXTURE_CUBE_MAP, 0, internalFormat, ti->width, ti->height, format, GL_UNSIGNED_BYTE, ti->data, GL_COLOR_ATTACHMENT0);
					tex->Generate();
					tex->Bind();

					tex->SpecifyTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0, ti->width, ti->height, ti->data);
					SOIL_free_texture_info(ti);
					for (int i = 1; i < 6; i++)
					{
						ti = GraphicsManager::LoadImage(textures.at(i).c_str(), 0);
						tex->SpecifyTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, ti->width, ti->height, ti->data);
						SOIL_free_texture_info(ti);
					}
					break;
				}
			}
			else
			{
				printf("%s could not be opened.\n", textures.at(0).c_str());
			}
		}
		if (tex != nullptr)
		{
			std::string fullName = textures.at(0);
			size_t pos = fullName.find_last_of("/\\");
			std::string fileName = fullName.substr(0, pos);
			pos = fileName.find_last_of("/\\");
			fileName = fileName.substr(pos + 1);
			tex->name = fileName;
			tex->SetDefaultParameters();
			GraphicsStorage::cubemaps[tex->name] = tex;
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
