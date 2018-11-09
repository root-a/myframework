#define _CRT_SECURE_NO_DEPRECATE
#include "GraphicsManager.h"
#include "GraphicsStorage.h"
#include "OBJ.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "dirent.h"
#include <GL/glew.h>
#include <iosfwd>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII
using namespace mwm;
GraphicsManager::GraphicsManager()
{
}

GraphicsManager::~GraphicsManager()
{
}

bool GraphicsManager::LoadOBJs(const char * path)
{
	FILE * file;
	file = fopen(path, "r");
	if (file == NULL){
		printf("Impossible to open the file ! Are you in the right path ?\n");
		getchar();
		return false;
	}
	while (1){

		char lineHeader[128];
		//meshID not in use currently
		int objID = 0;
		// read the first word of the line
		int res = fscanf(file, "%s %d", lineHeader, &objID);
		if (res == EOF)
		{
			break; // EOF = End Of File. Quit the loop.
		}
		// else : parse lineHeader

		OBJ* tempOBJ = new OBJ();
		tempOBJ->LoadAndIndexOBJ(lineHeader);
		tempOBJ->ID = objID;
		tempOBJ->name = lineHeader;
		size_t sep = tempOBJ->name.find_last_of("\\/");

		if (sep != std::string::npos)
			tempOBJ->name = tempOBJ->name.substr(sep + 1, tempOBJ->name.size() - sep - 1);

		size_t dot = tempOBJ->name.find_last_of(".");
		if (dot != std::string::npos)
		{
			tempOBJ->name = tempOBJ->name.substr(0, dot);
		}

		GraphicsStorage::objects.push_back(tempOBJ);
	}
	fclose(file);
	return true;
}

bool GraphicsManager::SaveToOBJ(OBJ *obj)
{
	FILE * file;
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

bool GraphicsManager::LoadTextures(const char * path)
{

	FILE * file;
	file = fopen(path, "r");
	if (file == NULL){
		printf("Impossible to open the file ! Are you in the right path ?\n");
		getchar();
		return false;
	}
	while (1){

		char texturePath[128];
		//meshID not in use currently
		// read the first word of the line
		int res = fscanf(file, "%s", texturePath);
		if (res == EOF)
		{
			break; // EOF = End Of File. Quit the loop.
		}
		
		Texture* texture = LoadTexture(texturePath);
		GraphicsStorage::textures.push_back(texture);
	}
	fclose(file);
	return true;
}



bool GraphicsManager::LoadCubeMaps(const char* path)
{
	FILE * file;
	file = fopen(path, "r");
	if (file == NULL){
		printf("Impossible to open the file ! Are you in the right path ?\n");
		return false;
	}
	while (1){

		char cubemapDirectory[128];
		// read the first line
		int res = fscanf(file, "%s", cubemapDirectory);
		if (res == EOF)
		{
			break; // EOF = End Of File. Quit the loop.
		}
		FILE* texturesFile;
		texturesFile = fopen(cubemapDirectory, "r");
		if (texturesFile == NULL){
			printf("Impossible to open the file ! Are you in the right path ?\n");
			getchar();
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
			texturesPaths.push_back(std::string(cubeMapTexturePath));
		}
		if (texturesPaths.size() == 6)
		{
			printf("Loading cubemap: %s\n", cubemapDirectory);
			Texture* cubeMap = GenerateCubeMap(texturesPaths);
			GraphicsStorage::cubemaps.push_back(cubeMap);
		}
		fclose(texturesFile);
	}
	fclose(file);
	return true;
}

bool GraphicsManager::LoadMaterials(const char * path) {
	
	FILE * file;
	file = fopen(path, "r");
	if (file == NULL){
		printf("Impossible to open the file ! Are you in the right path ?\n");
		getchar();
		return false;
	}
	char line[128];
	while (fgets(line, sizeof(line), file)) {
		if (line[0] == '/' || line[0] == ';' || line[0] == '#') continue; /* ignore comment line */
		int diffuseID, normalID, specID, emiID, heighID;
		char materialName[128];
		int matches = sscanf(line, "%s %d %d %d %d %d", materialName, &diffuseID, &normalID, &specID, &emiID, &heighID);
		if (matches != 6)
		{
			printf("Wrong material information!\n");
		} else {
			Material* material = new Material();
			material->AssignTexture(GraphicsStorage::textures[diffuseID]);
			GraphicsStorage::materials.push_back(material);
		}
		
	}
	fclose(file);
	return true;
}

bool GraphicsManager::LoadShaders(const char * path)
{
	FILE * file;
	file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ?\n");
		getchar();
		return false;
	}
	char line[128];
	std::map<std::string, std::string> shaders;
	while (fgets(line, sizeof(line), file)) {
		if (line[0] == '/' || line[0] == ';' || line[0] == '#') continue; /* ignore comment line */
		int diffuseID, normalID, specID, emiID, heighID;
		char programName[128];
		char filePath[128];
		int matches = sscanf(line, "%s %s", programName, filePath);
		if (matches != 2)
		{
			printf("Wrong shader information!\n");
		}
		else {
			shaders[programName] = filePath;
			//scan for all files within filepath for the same name
			//maybe we should make another file, shader directories
			//directories for all, we scan everything and then from second file we pick only the files we want to use
			//ShaderManager::Instance()->AddShader(programName, LoadProgram(VS + filePath, FS + filePath, GS + filePath);
		}
	}
	fclose(file);
	std::unordered_map<std::string, ShaderPaths> shadersMap;

	DIR *dir;
	struct dirent *ent;
	for (auto& shader : shaders)
	{
		size_t pos = shader.second.find_last_of("/");
		std::string shaderPath = shader.second.substr(0, pos+1);
		std::string shaderName = shader.second.substr(pos+1, shader.second.length()-1);
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
					std::string currentShaderExt = currentShaderFullName.substr(pos+1, currentShaderFullName.length()-1);
					if (strcmp(shaderName.c_str(), currentShaderName.c_str()) == 0)
					{
						printf("%s\n", ent->d_name);
						if (strcmp(currentShaderExt.c_str(), "fs") == 0)
						{
							shadersMap[shader.first].fs = shaderPath + currentShaderFullName;
							shaderFound = true;
						}
						else if (strcmp(currentShaderExt.c_str(), "vs") == 0)
						{
							shadersMap[shader.first].vs = shaderPath + currentShaderFullName;
							shaderFound = true;
						}
						else if (strcmp(currentShaderExt.c_str(), "gs") == 0)
						{
							shadersMap[shader.first].gs = shaderPath + currentShaderFullName;
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
					printf("%s/\n", ent->d_name);
					break;

				case DT_LNK:
					printf("%s@\n", ent->d_name);
					break;

				default:
					printf("%s*\n", ent->d_name);
				}
			}
			closedir(dir);
			if (!shaderFound)
			{
				printf("\nSHADER PROGRAM: %s not found in directory: %s !\n", shaderName.c_str(), shaderPath.c_str());
			}
		}
		else {
			/* could not open directory */
			perror("could not open directory");
		}
	}
	for (auto& program : shadersMap)
	{
		GLuint programID = LoadProgram(program.second.vs.c_str(), program.second.fs.c_str(), program.second.gs.c_str());
		GraphicsStorage::shaderIDs[program.first] = programID;
	}

	return true;
}

Texture* GraphicsManager::LoadBMP(const char *imagepath){

	printf("Reading image %s\n", imagepath);

	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	// Actual RGB data
	unsigned char * data;

	// Open the file
	FILE * file;
	file = fopen(imagepath, "rb");
	if (!file)							    
	{ printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); 
	  getchar(); 
	  return 0;
	}

	// Read the header, i.e. the 54 first bytes

	// If less than 54 bytes are read, problem
	if (fread(header, 1, 54, file) != 54){
		printf("Not a correct BMP file\n");
		return 0;
	}
	// A BMP files always begins with "BM"
	if (header[0] != 'B' || header[1] != 'M'){
		printf("Not a correct BMP file\n");
		return 0;
	}
	// Make sure this is a 24bpp file
	if (*(int*)&(header[0x1E]) != 0)         { printf("Not a correct BMP file\n");    return 0; }
	if (*(int*)&(header[0x1C]) != 24)         { printf("Not a correct BMP file\n");    return 0; }

	// Read the information about the image
	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize == 0)    imageSize = width*height * 3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos == 0)      dataPos = 54; // The BMP header is done that way

	// Create a buffer
	data = new unsigned char[imageSize];

	// Read the actual data from the file into the buffer
	fread(data, 1, imageSize, file);

	// Everything is in memory now, the file wan be closed
	fclose(file);

	// Create one OpenGL texture
	Texture* texture = CreateTexture(width,height,false,data);

	// OpenGL has now copied the data. Free our own version
	delete[] data;

	return texture;
}

Texture* GraphicsManager::LoadDDS(const char *imagepath){

	unsigned char header[124];

	FILE *fp;

	/* try to open the file */
	fp = fopen(imagepath, "rb");
	if (fp == NULL){
		printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar();
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


	unsigned char * buffer;
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
		return 0;
	}

	Texture* texture = new Texture(GL_TEXTURE_2D, 0, format, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer, GL_COLOR_ATTACHMENT0);

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
	unsigned int offset = 0;

	/* load the mipmaps */
	for (unsigned int level = 0; level < mipMapCount && (width || height); ++level)
	{
		unsigned int size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
		glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,
			0, size, buffer + offset);

		offset += size;
		width /= 2;
		height /= 2;

		// Deal with Non-Power-Of-Two textures.
		if (width < 1) width = 1;
		if (height < 1) height = 1;

	}

	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	float aniso = 16.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
	
	free(buffer);

	texture->handle = textureID;
	return texture;


}

GLuint GraphicsManager::LoadProgram(const char * vertex_file_path, const char * fragment_file_path, const char* geometry_file_path){

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint GeometryShaderID = -1;
	if (geometry_file_path != nullptr && strcmp(geometry_file_path, "") != 0)
	{
		GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
	}

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open())
	{
		std::string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()){
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	// Read the Geometry Shader code from the file
	std::string GeometryShaderCode;
	if (geometry_file_path != nullptr)
	{
		std::ifstream GeometryShaderStream(geometry_file_path, std::ios::in);
		if (GeometryShaderStream.is_open())
		{
			std::string Line = "";
			while (getline(GeometryShaderStream, Line))
				GeometryShaderCode += "\n" + Line;
			GeometryShaderStream.close();
		}
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
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


	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
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

	if (geometry_file_path != nullptr && strcmp(geometry_file_path, "") != 0)
	{
		// Compile Geometry Shader
		printf("Compiling shader : %s\n", geometry_file_path);
		char const * GeometrySourcePointer = GeometryShaderCode.c_str();
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
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	if (geometry_file_path != nullptr && strcmp(geometry_file_path, "") != 0)
	{
		glAttachShader(ProgramID, GeometryShaderID);
	}
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		std::vector<char> ProgramErrorMessage(std::max<int>(InfoLogLength, int(1)));
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(VertexShaderID);
	if (geometry_file_path != nullptr && strcmp(geometry_file_path, "") != 0)
	{
		glDeleteShader(GeometryShaderID);
	}
	glDeleteShader(FragmentShaderID);
	fprintf(stdout, "Shader ID: %d \n\n", ProgramID);
	return ProgramID;
}

Mesh* GraphicsManager::LoadOBJToVBO(OBJ* object, Mesh* mesh)
{
	//Create VAO
	glGenVertexArrays(1, &mesh->vaoHandle);
	//Bind VAO
	glBindVertexArray(mesh->vaoHandle);

	// 1rst attribute buffer : vertices
	glGenBuffers(1, &mesh->vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, object->indexed_vertices.size() * sizeof(Vector3F), &object->indexed_vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute, size, type, normalized?, stride, array buffer offset
	glEnableVertexAttribArray(0);

	// 2nd attribute buffer : UVs
	glGenBuffers(1, &mesh->uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, object->indexed_uvs.size() * sizeof(Vector2F), &object->indexed_uvs[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute, size, type, normalized?, stride, array buffer offset
	glEnableVertexAttribArray(1);

	// 3rd attribute buffer : normals
	glGenBuffers(1, &mesh->normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, object->indexed_normals.size() * sizeof(Vector3F), &object->indexed_normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute, size, type, normalized?, stride, array buffer offset
	glEnableVertexAttribArray(2);

	// 4th element buffer Generate a buffer for the indices as well
	glGenBuffers(1, &mesh->elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, object->indices.size() * sizeof(unsigned int), &object->indices[0], GL_STATIC_DRAW);
	mesh->indicesSize = (unsigned int)object->indices.size();

	//Unbind the VAO now that the VBOs have been set up
	glBindVertexArray(0);
	
	mesh->obj = object;
	return mesh;
}

void GraphicsManager::LoadAllOBJsToVBO()
{
	for (size_t i = 0; i < GraphicsStorage::objects.size(); i++)
	{	
		Mesh* newMesh = new Mesh();
		LoadOBJToVBO(GraphicsStorage::objects.at(i), newMesh);
		GraphicsStorage::meshes[newMesh->obj->name] = newMesh;
	}
}

Texture* GraphicsManager::CreateTexture(int width, int height, bool isDepth, unsigned char* data) {

	Texture* texture = new Texture(GL_TEXTURE_2D, 0, isDepth ? GL_DEPTH_COMPONENT : GL_RGBA8, width, height, isDepth ? GL_DEPTH_COMPONENT : GL_RGBA8, isDepth ? GL_FLOAT : GL_UNSIGNED_BYTE, data ? data : NULL, GL_COLOR_ATTACHMENT0);
	
    // Poor filtering, or ...
	//texture->AddTextureParameterI(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//texture->AddTextureParameterI(GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // ... nice trilinear filtering.
	texture->AddTextureParameterI(GL_TEXTURE_WRAP_S, GL_REPEAT);
	texture->AddTextureParameterI(GL_TEXTURE_WRAP_T, GL_REPEAT);

	texture->AddTextureParameterI(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	texture->AddTextureParameterI(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	texture->Generate();
	glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
}

void GraphicsManager::LoadAllAssets()
{
	printf("\nLOADING GPU PROGRAMS\n");
	LoadShaders("Resources/programs.txt");
	printf("\nDONE\n");

	printf("\nLOADING OBJs\n");
	LoadOBJs("Resources/models.txt");
	printf("\nDONE\n");

	//load all objs to vbo and create meshes with buffers
	printf("\nLOADING OBJs TO VBO and creating Meshes with buffers\n");
	LoadAllOBJsToVBO();
	printf("\nDONE\n");

	printf("\nLOADING TEXTURES\n");
	LoadTextures("Resources/textures.txt");
	printf("\nDONE\n");

	printf("\nLOADING CUBE MAPS\n");
	LoadCubeMaps("Resources/cubemaps.txt");
	printf("\nDONE\n");

	printf("\nLOADING MATERIALS\n");
	LoadMaterials("Resources/materials.txt");
	printf("\nDONE\n");
}

unsigned char * GraphicsManager::LoadImage(const char* path, int* x, int* y, int* numOfElements, int forcedNumOfEle)
{
	//printf("Reading image %s\n", path);
	return stbi_load(path, x, y, numOfElements, forcedNumOfEle);
}

Texture* GraphicsManager::LoadTexture(char* path)
{
	return LoadDDS(path);
}

Texture* GraphicsManager::GenerateCubeMap(const std::vector<std::string>& textures)
{
	// Load the texture
	
	GLuint cubeMapTextureID;
	glGenTextures(1, &cubeMapTextureID);
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);

	int x, y, numOfElements;
	for (int i = 0; i < 6; i++)
	{
		unsigned char* data = GraphicsManager::LoadImage(textures.at(i).c_str(), &x, &y, &numOfElements, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	Texture* tex = new Texture(GL_TEXTURE_CUBE_MAP, 0, GL_RGB, x, y, GL_RGB, GL_UNSIGNED_BYTE, NULL, GL_COLOR_ATTACHMENT0);
	tex->handle = cubeMapTextureID;
	return tex;
}