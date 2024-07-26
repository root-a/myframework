#pragma once
#include "gl_window.h"
#include "RenderPass.h"
#include "FBOManager.h"
#include "FrameBuffer.h"
#include "Texture.h"
#include "Shader.h"
#include "GraphicsStorage.h"
#include "SceneGraph.h"
#include "MaterialProfile.h"
#include "TextureProfile.h"
#include "RenderProfile.h"
#include "CPUBlockData.h"
#include "ShaderBlockData.h"
#include "ShaderBlock.h"
#include "SceneGraph.h"
#include "Object.h"
#include "ObjectProfile.h"
#include "Camera.h"
#include "CameraManager.h"
#include "MyMathLib.h"
#include "DirectionalLight.h"
#include "ScriptsComponent.h"
#include "ParticleSystem.h"
#include "LineSystem.h"
#include "PointSystem.h"
#include "BoundingBoxSystem.h"
#include "OBJ.h"
#include "Render.h"
#include "Material.h"
#include "Script.h"
#include "Vao.h"
#include "Ebo.h"
#include "Times.h"
#include "RenderBuffer.h"
#include "GraphicsManager.h"
#include <sstream>
#include <filesystem>

struct SimpleString
{
	char* text;
	int length;
};

struct DirectoryIterator {};

extern "C" {
	//typedef struct vec2 { float arr[2]; struct { float red, green; }; Vector2 v; } vec2;
	//typedef struct vec3 { float arr[3]; struct { float red, green, blue; }; Vector3 v; } vec3;
	//typedef struct vec4 { float arr[4]; struct { float red, green, blue, alpha; }; Vector4 v;} vec4;
	//typedef struct mat4 { float mat[4][4]; Matrix4 m; } mat4;
	//typedef struct mat3 { float mat[3][3]; Matrix3 m; } mat3;
	//typedef struct mat2 { float mat[2][2]; } mat2;
#pragma region get_opengl_extensions

	__declspec(dllexport) void* cglfwGetProcAddress(const char* procname) {
		return glfwGetProcAddress(procname);
	}

#pragma endregion
#pragma region texture

	__declspec(dllexport) Texture* Texture_new(const char* guid, GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, void * pixels, GLenum attachment) {
		return GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<Texture>(guid, target, level, internalFormat, width, height, format, type, pixels, attachment);
	}

	__declspec(dllexport) void Texture_gc(Texture* self) {
		//delete self;
	}

	__declspec(dllexport) void Texture_SetTextureParameterI(Texture* self, GLenum pname, GLint param) {
		self->SetTextureParameterI(pname, param);
	}

	__declspec(dllexport) void Texture_SetTextureParameterIV(Texture* self, GLenum pname, int* param) {
		self->SetTextureParameterIV(pname, param);
	}

	__declspec(dllexport) void Texture_SetTextureParameterF(Texture* self, GLenum pname, GLfloat param) {
		self->SetTextureParameterF(pname, param);
	}

	__declspec(dllexport) void Texture_SetTextureParameterFV(Texture* self, GLenum pname, float* param) {
		self->SetTextureParameterFV(pname, param);
	}

	__declspec(dllexport) void Texture_AddDefaultParameters(Texture* self) {
		self->SetDefaultParameters();
	}

	__declspec(dllexport) void Texture_AddClampingToEdge(Texture* self) {
		self->SetClampingToEdge();
	}

	__declspec(dllexport) void Texture_AddClampingToBorder(Texture* self, Vector4F& borderColor) {
		self->SetClampingToBorder(borderColor);
	}

	__declspec(dllexport) void Texture_SetFiltering(Texture* self, GLint MIN_FILTER, GLint MAG_FILTER) {
		self->SetFiltering(MIN_FILTER, MAG_FILTER);
	}

	__declspec(dllexport) void Texture_Generate(Texture* self) {
		self->Generate();
	}

	__declspec(dllexport) void Texture_Bind(Texture* self) {
		self->Bind();
	}

	__declspec(dllexport) void Texture_Specify(Texture* self) {
		self->Specify();
	}

	__declspec(dllexport) void Texture_GenerateBindSpecify(Texture* self) {
		self->GenerateBindSpecify();
	}

	__declspec(dllexport) void Texture_SpecifyTexture(Texture* self, GLenum target, GLsizei width, GLsizei height, void* pixels) {
		self->SpecifyTexture(target, width, height, pixels);
	}

	__declspec(dllexport) void Texture_GenerateMipMaps(Texture* self) {
		self->GenerateMipMaps();
	}

	__declspec(dllexport) void Texture_SetNearest(Texture* self) {
		self->SetNearest();
	}

	__declspec(dllexport) void Texture_SetLinear(Texture* self) {
		self->SetLinear();
	}

	__declspec(dllexport) const char* Texture_GetName(Texture* self) {
		return self->name.c_str();
	}

	__declspec(dllexport) const char* Texture_GetPath(Texture* self) {
		return self->path.c_str();
	}

	__declspec(dllexport) void Texture_SetPath(Texture* self, const char* path) {
		self->path = path;
	}

	__declspec(dllexport) const char* Texture_GetTexturePath(Texture* self) {
		return self->texturePath.c_str();
	}

	__declspec(dllexport) void Texture_SetTexturePath(Texture* self, const char* path) {
		self->texturePath = path;
	}

	__declspec(dllexport) void Texture_SetName(Texture* self, const char* name) {
		self->name = name;
	}

	__declspec(dllexport) unsigned int* Texture_GetHandleRef(Texture* self) {
		return &self->handle;
	}

	__declspec(dllexport) unsigned int Texture_GetHandle(Texture* self) {
		return self->handle;
	}

	__declspec(dllexport) unsigned int Texture_GetTarget(Texture* self) {
		return self->target;
	}

	__declspec(dllexport) int Texture_GetLevel(Texture* self) {
		return self->level;
	}

	__declspec(dllexport) int Texture_GetInternalFormat(Texture* self) {
		return self->internalFormat;
	}

	__declspec(dllexport) int Texture_GetWidth(Texture* self) {
		return self->width;
	}

	__declspec(dllexport) int Texture_GetHeight(Texture* self) {
		return self->height;
	}

	__declspec(dllexport) unsigned int Texture_GetFormat(Texture* self) {
		return self->format;
	}

	__declspec(dllexport) unsigned int Texture_GetType(Texture* self) {
		return self->type;
	}

	__declspec(dllexport) unsigned int Texture_GetAttachment(Texture* self) {
		return self->attachment;
	}

	__declspec(dllexport) bool Texture_HasCustomMipMaps(Texture* self) {
		return self->hasMipMaps;
	}

#pragma endregion

#pragma region render_buffer

	__declspec(dllexport) RenderBuffer* RenderBuffer_new(const char* guid, GLint internalFormat, GLsizei width, GLsizei height, GLenum attachment) {
		return GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<RenderBuffer>(guid, internalFormat, width, height, attachment);
	}

	__declspec(dllexport) void RenderBuffer_gc(RenderBuffer* self) {
		//delete self;
	}

	__declspec(dllexport) void RenderBuffer_Generate(RenderBuffer* self) {
		self->Generate();
	}

	__declspec(dllexport) void RenderBuffer_Bind(RenderBuffer* self) {
		self->Bind();
	}

	__declspec(dllexport) void RenderBuffer_Specify(RenderBuffer* self) {
		self->Specify();
	}

	__declspec(dllexport) void RenderBuffer_GenerateBindSpecify(RenderBuffer* self) {
		self->GenerateBindSpecify();
	}

	__declspec(dllexport) const char* RenderBuffer_GetName(RenderBuffer* self) {
		return self->name.c_str();
	}

	__declspec(dllexport) const char* RenderBuffer_GetPath(RenderBuffer* self) {
		return self->path.c_str();
	}

	__declspec(dllexport) void RenderBuffer_SetPath(RenderBuffer* self, const char* path) {
		self->path = path;
	}

	__declspec(dllexport) void RenderBuffer_SetName(RenderBuffer* self, const char* name) {
		self->name = name;
	}

	__declspec(dllexport) unsigned int* RenderBuffer_GetHandleRef(RenderBuffer* self) {
		return &self->handle;
	}

	__declspec(dllexport) unsigned int RenderBuffer_GetHandle(RenderBuffer* self) {
		return self->handle;
	}

	__declspec(dllexport) int RenderBuffer_GetInternalFormat(RenderBuffer* self) {
		return self->internalFormat;
	}

	__declspec(dllexport) int RenderBuffer_GetWidth(RenderBuffer* self) {
		return self->width;
	}

	__declspec(dllexport) int RenderBuffer_GetHeight(RenderBuffer* self) {
		return self->height;
	}

	__declspec(dllexport) unsigned int RenderBuffer_GetAttachment(RenderBuffer* self) {
		return self->attachment;
	}
#pragma endregion

#pragma region framebuffer

	__declspec(dllexport) FrameBuffer* FrameBuffer_new(const char* guid, bool isDynamic) {
		FrameBuffer* frameBuffer = GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<FrameBuffer>(guid, (unsigned int)GL_FRAMEBUFFER);
		FBOManager::Instance()->AddFrameBuffer(frameBuffer, isDynamic);
		return frameBuffer;
	}

	__declspec(dllexport) void FrameBuffer_gc(FrameBuffer* self) {
		//delete self;
	}

	__declspec(dllexport) void FrameBuffer_RegisterTexture(FrameBuffer* self, Texture* texture) {
		self->RegisterTexture(texture);
	}

	__declspec(dllexport) void FrameBuffer_SpecifyTextureAndMip(FrameBuffer* self, Texture* texture, unsigned int target, int mipLevel) {
		self->SpecifyTextureAndMip(texture, target, mipLevel);
	}

	__declspec(dllexport) void FrameBuffer_SpecifyTexture(FrameBuffer* self, Texture* texture) {
		self->SpecifyTexture(texture);
	}

	__declspec(dllexport) void FrameBuffer_SpecifyRenderBuffer(FrameBuffer* self, RenderBuffer* rbuffer) {
		self->SpecifyRenderBuffer(rbuffer);
	}

	__declspec(dllexport) void FrameBuffer_SpecifyTextures(FrameBuffer* self) {
		self->SpecifyTextures();
	}

	__declspec(dllexport) void FrameBuffer_CheckAndCleanup(FrameBuffer* self) {
		self->CheckAndCleanup();
	}

	__declspec(dllexport) RenderBuffer* FrameBuffer_RegisterRenderBuffer(FrameBuffer* self, RenderBuffer* rbuffer) {
		return self->RegisterRenderBuffer(rbuffer);
	}

	__declspec(dllexport) void FrameBuffer_Bind(FrameBuffer* self, GLuint target) {
		self->BindBuffer(target);
	}

	__declspec(dllexport) const char* FrameBuffer_GetName(FrameBuffer* self) {
		return self->name.c_str();
	}

	__declspec(dllexport) void FrameBuffer_SetName(FrameBuffer* self, const char* name) {
		self->name = name;
	}

	__declspec(dllexport) const char* FrameBuffer_GetPath(FrameBuffer* self) {
		return self->path.c_str();
	}

	__declspec(dllexport) void FrameBuffer_SetPath(FrameBuffer* self, const char* path) {
		self->path = path;
	}

	__declspec(dllexport) Texture** FrameBuffer_GetTextures(FrameBuffer* self) {
		return self->textures.data();
	}

	__declspec(dllexport) int FrameBuffer_GetTexturesSize(FrameBuffer* self) {
		return (int)self->textures.size();
	}

	__declspec(dllexport) RenderBuffer** FrameBuffer_GetRenderBuffers(FrameBuffer* self) {
		return self->renderBuffers.data();
	}

	__declspec(dllexport) int FrameBuffer_GetRenderBuffersSize(FrameBuffer* self) {
		return (int)self->renderBuffers.size();
	}

	__declspec(dllexport) int FrameBuffer_IsDynamic(FrameBuffer* self) {
		return FBOManager::Instance()->IsDynamic(self);
	}

	__declspec(dllexport) void FrameBuffer_GetScale(FrameBuffer* self, Vector2F& scaleOut) {
		scaleOut.x = self->scaleXFactor;
		scaleOut.y = self->scaleYFactor;
	}

	__declspec(dllexport) void FrameBuffer_SetScale(FrameBuffer* self, Vector2F& scale) {
		self->scaleXFactor = scale.x;
		self->scaleYFactor = scale.y;
	}

	__declspec(dllexport) void FrameBuffer_SetTextureAndMip(FrameBuffer* self, Texture* texture, int mip) {
		self->SetTextureAndMip(texture, mip);
	}

#pragma endregion
#pragma region renderpass

	__declspec(dllexport) RenderPass* RenderPass_new(const char* guid) {
		return GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<RenderPass>(guid);
	}

	__declspec(dllexport) void RenderPass_gc(RenderPass* self) {
		//delete self;
	}

	__declspec(dllexport) void RenderPass_SetFrameBuffer(RenderPass* self, FrameBuffer* fbo) {
		self->SetFrameBuffer(fbo);
	}

	__declspec(dllexport) FrameBuffer* RenderPass_GetFrameBuffer(RenderPass* self) {
		return self->fbo;
	}

	__declspec(dllexport) void RenderPass_LoadLuaFile(RenderPass* self, const char * filename) {
		self->LoadLuaFile(filename);
	}

	__declspec(dllexport) DataRegistry* RenderPass_GetDataRegistry(RenderPass* self) {
		return &self->registry;
	}

	__declspec(dllexport) void RenderPass_AddUniformBuffers(RenderPass* self, Shader* shader)
	{
		self->AddShaderBlocks(shader->globalUniformBuffers);
	}

	__declspec(dllexport) void RenderPass_AddUniformBuffer(RenderPass* self, ShaderBlock* ub)
	{
		self->AddShaderBlock(ub);
	}

	__declspec(dllexport) void RenderPass_RemoveUniformBuffer(RenderPass* self, ShaderBlock* ub)
	{
		self->RemoveShaderBlock(ub);
	}

	__declspec(dllexport) void RenderPass_UpdateProfileFromDataRegistry(RenderPass* self, const DataRegistry& dataRegistry)
	{
		self->UpdateProfileFromDataRegistry(dataRegistry);
	}

	__declspec(dllexport) const char* RenderPass_GetName(RenderPass* self)
	{
		return self->name.c_str();
	}

	__declspec(dllexport) void RenderPass_SetPath(RenderPass* self, const char* path)
	{
		self->path = path;
	}

	__declspec(dllexport) void RenderPass_SetName(RenderPass* self, const char* name)
	{
		self->name = name;
	}

	__declspec(dllexport) const char* RenderPass_GetPath(RenderPass* self)
	{
		return self->path.c_str();
	}

	__declspec(dllexport) int RenderPass_GetNumberOfUniformBufferDatas(RenderPass* self)
	{
		return (int)self->shaderBlockDatas.size();
	}

	__declspec(dllexport) Script* RenderPass_GetScript(RenderPass* self)
	{
		return self->script;
	}
#pragma endregion
#pragma region shader
	__declspec(dllexport) Shader* Shader_new(const char* guid) {
		printf("TRYING TO CREATE NEW SHADER VIA SCRIPT - UNDEFINED");
		return nullptr;
	}

	__declspec(dllexport) void Shader_SetRenderTarget(Shader* self, int slot, GLenum renderTarget) {
		self->SetRenderTarget(slot, renderTarget);
	}

	__declspec(dllexport) void Shader_gc(Shader* self) {
		//delete self;
	}

	__declspec(dllexport) void Shader_AddObjectBlock(Shader* self, BlockType type, ShaderBlock* block) {
		switch (type)
		{
		case BlockType::Uniform:
			self->objectUniformBuffers.push_back(block);
			break;
		case BlockType::Storage:
			self->objectShaderStorageBuffers.push_back(block);
			break;
		default:
			break;
		}
	}

	__declspec(dllexport) void Shader_AddMaterialBlock(Shader* self, BlockType type, ShaderBlock* block) {
		switch (type)
		{
		case BlockType::Uniform:
			self->materialUniformBuffers.push_back(block);
			break;
		case BlockType::Storage:
			self->materialShaderStorageBuffers.push_back(block);
			break;
		default:
			break;
		}
	}

	__declspec(dllexport) void Shader_AddGlobalBlock(Shader* self, BlockType type, ShaderBlock* block) {
		switch (type)
		{
		case BlockType::Uniform:
			self->globalUniformBuffers.push_back(block);
			break;
		case BlockType::Storage:
			self->globalShaderStorageBuffers.push_back(block);
			break;
		default:
			break;
		}
	}

	__declspec(dllexport) int Shader_GetObjectUniformBuffersCount(Shader* self) {
		return self->objectUniformBuffers.size();
	}

	__declspec(dllexport) ShaderBlock** Shader_GetObjectUniformBuffers(Shader* self) {
		return self->objectUniformBuffers.data();
	}

	__declspec(dllexport) int Shader_GetMaterialUniformBuffersCount(Shader* self) {
		return self->materialUniformBuffers.size();
	}

	__declspec(dllexport) ShaderBlock** Shader_GetMaterialUniformBuffers(Shader* self) {
		return self->materialUniformBuffers.data();
	}

	__declspec(dllexport) int Shader_GetGlobalUniformBuffersCount(Shader* self) {
		return self->globalUniformBuffers.size();
	}

	__declspec(dllexport) ShaderBlock** Shader_GetGlobalUniformBuffers(Shader* self) {
		return self->globalUniformBuffers.data();
	}

	__declspec(dllexport) int Shader_GetObjectShaderStorageCount(Shader* self) {
		return self->objectShaderStorageBuffers.size();
	}

	__declspec(dllexport) ShaderBlock** Shader_GetObjectShaderStorages(Shader* self) {
		return self->objectShaderStorageBuffers.data();
	}

	__declspec(dllexport) int Shader_GetMaterialShaderStorageCount(Shader* self) {
		return self->materialShaderStorageBuffers.size();
	}

	__declspec(dllexport) ShaderBlock** Shader_GetMaterialShaderStorages(Shader* self) {
		return self->materialShaderStorageBuffers.data();
	}

	__declspec(dllexport) int Shader_GetGlobalShaderStorageCount(Shader* self) {
		return self->globalShaderStorageBuffers.size();
	}

	__declspec(dllexport) ShaderBlock** Shader_GetGlobalShaderStorages(Shader* self) {
		return self->globalShaderStorageBuffers.data();
	}

	__declspec(dllexport) const char* Shader_GetName(Shader* self) {
		return self->name.c_str();
	}

	__declspec(dllexport) const char* Shader_GetPath(Shader* self) {
		return self->shaderPaths.path.c_str();
	}
#pragma endregion
#pragma region resources
	__declspec(dllexport) void GetGUID(void* asset, char outGuid[36]) {
		std::string strGUID = GraphicsStorage::assetRegistry.GetAssetIDAsString(asset);
		//int size = strlen(strGUID.c_str());
		memcpy(outGuid, strGUID.c_str(), 36);
		//int nrOfFloatsPerElement = self->layout.GetStride() / (sizeof(float));
		//Vector3F* vertexData = new Vector3F[self->maxElementCount];
		//glGetNamedBufferSubData(self->handle, 0, self->maxElementCount * nrOfFloatsPerElement, vertexData);
		//int i = 0;
	}

	__declspec(dllexport) void GenerateGUID(char outGuid[36]) {
		std::string strGUID = uuids::to_string(gen());
		//int size = strlen(strGUID.c_str());
		memcpy(outGuid, strGUID.c_str(), 36);
		//int nrOfFloatsPerElement = self->layout.GetStride() / (sizeof(float));
		//Vector3F* vertexData = new Vector3F[self->maxElementCount];
		//glGetNamedBufferSubData(self->handle, 0, self->maxElementCount * nrOfFloatsPerElement, vertexData);
		//int i = 0;
	}
	

	__declspec(dllexport) const char* GetShaderBlockFrequencyConfig(ShaderBlock* self) {
		return GraphicsStorage::shaderBlockTypes[self->name].c_str();
	}

	__declspec(dllexport) const char* ShaderBlock_GetName(ShaderBlock* self) {
		return self->name.c_str();
	}

	__declspec(dllexport) Shader* GetShader(const char * guid) {
		return (Shader*)GraphicsStorage::assetRegistry.GetAssetByStringID(guid);
	}

	__declspec(dllexport) Shader* LoadShader(const char* guid, const char* path) {
		return GraphicsManager::LoadShader(guid, path);
	}

	__declspec(dllexport) Texture* GetTexture(const char* guid) {
		return (Texture*)GraphicsStorage::assetRegistry.GetAssetByStringID(guid);
	}

	__declspec(dllexport) RenderBuffer* GetRenderBuffer(const char* guid) {
		return (RenderBuffer*)GraphicsStorage::assetRegistry.GetAssetByStringID(guid);
	}

	__declspec(dllexport) Material* GetMaterial(const char * guid) {
		return (Material*)GraphicsStorage::assetRegistry.GetAssetByStringID(guid);
	}

	__declspec(dllexport) RenderProfile* GetRenderProfile(const char * guid) {
		return (RenderProfile*)GraphicsStorage::assetRegistry.GetAssetByStringID(guid);
	}

	__declspec(dllexport) VertexArray* GetVertexArray(const char * name) {
		return (VertexArray*)GraphicsStorage::assetRegistry.GetAssetByStringID(name);
	}

	__declspec(dllexport) VertexBuffer* GetVertexBuffer(const char* guid) {
		return (VertexBuffer*)GraphicsStorage::assetRegistry.GetAssetByStringID(guid);
	}

	__declspec(dllexport) ElementBuffer* GetElementBuffer(const char* guid) {
		return (ElementBuffer*)GraphicsStorage::assetRegistry.GetAssetByStringID(guid);
	}

	__declspec(dllexport) BufferLayout* GetBufferLayout(const char* guid) {
		return (BufferLayout*)GraphicsStorage::assetRegistry.GetAssetByStringID(guid);
	}

	__declspec(dllexport) MaterialProfile* GetMaterialProfile(const char * guid) {
		return (MaterialProfile*)GraphicsStorage::assetRegistry.GetAssetByStringID(guid);
	}

	__declspec(dllexport) TextureProfile* GetTextureProfile(const char * guid) {
		return (TextureProfile*)GraphicsStorage::assetRegistry.GetAssetByStringID(guid);
	}

	__declspec(dllexport) ShaderBlock* GetUniformBuffer(const char * name)
	{
		return GraphicsStorage::GetUniformBuffer(name);
	}

	__declspec(dllexport) ShaderBlockData* GetUniformBufferData(const char * name)
	{
		return GraphicsStorage::GetUniformBufferData(name);
	}

	__declspec(dllexport) ShaderBlock* GetShaderStorageBuffer(const char * name)
	{
		return GraphicsStorage::GetShaderStorageBuffer(name);
	}

	__declspec(dllexport) ShaderBlockData* GetShaderStorageBufferData(const char * name)
	{
		return GraphicsStorage::GetShaderStorageBufferData(name);
	}

	__declspec(dllexport) RenderPass** GetRenderQueue()
	{
		return (RenderPass**)GraphicsStorage::renderingQueue.data();
	}

	__declspec(dllexport) int GetRenderQueueSize()
	{
		return GraphicsStorage::renderingQueue.size();
	}

	__declspec(dllexport) void RenderQueue_AddPass(RenderPass* pass)
	{
		GraphicsStorage::renderingQueue.push_back(pass);
	}

	__declspec(dllexport) void RenderQueue_AddPassAt(RenderPass* pass, int insertionPos)
	{
		GraphicsStorage::renderingQueue.insert(GraphicsStorage::renderingQueue.begin() + insertionPos, pass);
	}

	__declspec(dllexport) RenderPass* GetRenderPass(const char* guid)
	{
		return (RenderPass*)GraphicsStorage::assetRegistry.GetAssetByStringID(guid);
	}

	__declspec(dllexport) FrameBuffer* GetFrameBuffer(const char * guid) {
		return (FrameBuffer*)GraphicsStorage::assetRegistry.GetAssetByStringID(guid);
	}

	__declspec(dllexport) FrameBuffer* GetCurrentDrawFrameBuffer() {
		return FBOManager::Instance()->GetCurrentDrawFrameBuffer();
	}

	__declspec(dllexport) ObjectProfile* GetObjectProfile(const char * guid) {
		return (ObjectProfile*)GraphicsStorage::assetRegistry.GetAssetByStringID(guid);
	}

	__declspec(dllexport) Object* GetObject(const char * guid) {
		return (Object*)GraphicsStorage::assetRegistry.GetAssetByStringID(guid);
	}

	__declspec(dllexport) Object* GetObjectByName(const char* name) {
		for (auto& object : *GraphicsStorage::assetRegistry.GetPool<Object>())
		{
			if (object.name.compare(name) == 0)
			{
				return &object;
			}
		}
		return nullptr;
	}

	__declspec(dllexport) void Vector2F_gc(Vector2F* self)
	{
		delete self;
	}

	__declspec(dllexport) void Vector3F_gc(Vector3F* self)
	{
		delete self;
	}

	__declspec(dllexport) void Vector4F_gc(Vector4F* self)
	{
		delete self;
	}

	__declspec(dllexport) void QuaternionF_gc(QuaternionF* self)
	{
		delete self;
	}

	__declspec(dllexport) void Matrix3F_gc(Matrix3F* self)
	{
		delete self;
	}

	__declspec(dllexport) void Matrix4F_gc(Matrix4F* self)
	{
		delete self;
	}

	__declspec(dllexport) void Vector2_gc(Vector2* self)
	{
		delete self;
	}

	__declspec(dllexport) void Vector3_gc(Vector3* self)
	{
		delete self;
	}

	__declspec(dllexport) void Vector4_gc(Vector4* self)
	{
		delete self;
	}

	__declspec(dllexport) void Quaternion_gc(Quaternion* self)
	{
		delete self;
	}

	__declspec(dllexport) void Matrix3_gc(Matrix3* self)
	{
		delete self;
	}

	__declspec(dllexport) void Matrix4_gc(Matrix4* self)
	{
		delete self;
	}

#pragma endregion
#pragma region script
	__declspec(dllexport) void Script_LoadLuaFile(Script* self, const char * fileName)
	{
		self->LoadLuaFile(fileName);
	}

	__declspec(dllexport) void Script_Call(Script* self, const char * functionName)
	{
		self->Call(functionName);
	}

	__declspec(dllexport) void Script_Reload(Script* self)
	{
		self->Reload();
	}

	__declspec(dllexport) void Script_Unload(Script* self)
	{
		self->Unload();
	}

	__declspec(dllexport) const char* Script_GetName(Script* self)
	{
		return self->name.c_str();
	}

	__declspec(dllexport) const char* Script_GetPath(Script* self)
	{
		return self->path.c_str();
	}
#pragma endregion
#pragma region render_profile
	__declspec(dllexport) RenderProfile* RenderProfile_new(const char* guid)
	{
		return GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<RenderProfile>(guid);
	}

	__declspec(dllexport) void RenderProfile_gc(RenderProfile* self)
	{
		//delete self;
	}

	__declspec(dllexport) void RenderProfile_LoadLuaFile(RenderProfile* self, const char * fileName)
	{
		self->LoadLuaFile(fileName);
	}

	__declspec(dllexport) const char* RenderProfile_GetName(RenderProfile* self)
	{
		return self->name.c_str();
	}

	__declspec(dllexport) const char* RenderProfile_GetPath(RenderProfile* self)
	{
		return self->path.c_str();
	}

	__declspec(dllexport) void RenderProfile_SetPath(RenderProfile* self, const char* path)
	{
		self->path = path;
	}

	__declspec(dllexport) void RenderProfile_SetName(RenderProfile* self, const char* name)
	{
		self->name = name;
	}

	__declspec(dllexport) Script* RenderProfile_GetScript(RenderProfile* self, const char * fileName)
	{
		return self->script;
	}

#pragma endregion
#pragma region texture_profile
	__declspec(dllexport) TextureProfile* TextureProfile_new(const char* guid)
	{
		return GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<TextureProfile>(guid);
	}
	__declspec(dllexport) void TextureProfile_gc(TextureProfile* self)
	{
		//delete self;
	}

	__declspec(dllexport) void TextureProfile_AddTexture(TextureProfile* self, Texture* tex, int slot)
	{
		self->AddTexture(tex, slot);
	}

	__declspec(dllexport) void TextureProfile_RemoveTexture(TextureProfile* self, Texture* tex, int slot)
	{
		self->RemoveTextureAtSlot(tex, slot);
	}

	__declspec(dllexport) const char* TextureProfile_GetName(TextureProfile* self)
	{
		return self->name.c_str();
	}

	__declspec(dllexport) const char* TextureProfile_GetPath(TextureProfile* self)
	{
		return self->path.c_str();
	}

	__declspec(dllexport) void TextureProfile_SetPath(TextureProfile* self, const char* path)
	{
		self->path = path;
	}

	__declspec(dllexport) void TextureProfile_SetName(TextureProfile* self, const char* name)
	{
		self->name = name;
	}

	__declspec(dllexport) TextureAndSlot* TextureProfile_GetTextures(TextureProfile* self)
	{
		if (self->textures.size() > 0)
		{
			return self->textures.data();
		}
		return nullptr;
	}

	__declspec(dllexport) int TextureProfile_GetNumberOfTextures(TextureProfile* self)
	{
		return (int)self->textures.size();
	}

	__declspec(dllexport) Texture* TextureProfile_GetTexture(TextureProfile* self, int slot)
	{
		return self->GetTexture(slot);
	}

#pragma endregion
#pragma region material_profile
	__declspec(dllexport) MaterialProfile* MaterialProfile_new(const char* guid)
	{
		return GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<MaterialProfile>(guid);
	}
	__declspec(dllexport) void MaterialProfile_gc(MaterialProfile* self)
	{
		//delete self;
	}

	__declspec(dllexport) DataRegistry& MaterialProfile_GetDataRegistry(MaterialProfile* self)
	{
		return self->registry;
	}

	__declspec(dllexport) void MaterialProfile_AddUniformBuffers(MaterialProfile* self, Shader* shader)
	{
		self->AddShaderBlocks(shader->materialUniformBuffers);
	}

	__declspec(dllexport) void MaterialProfile_AddUniformBuffer(MaterialProfile* self, ShaderBlock* ub)
	{
		self->AddShaderBlock(ub);
	}

	__declspec(dllexport) void MaterialProfile_RemoveUniformBuffer(MaterialProfile* self, ShaderBlock* ub)
	{
		self->RemoveShaderBlock(ub);
	}

	__declspec(dllexport) void MaterialProfile_UpdateProfileFromDataRegistry(MaterialProfile* self, const DataRegistry& dataRegistry)
	{
		self->UpdateProfileFromDataRegistry(dataRegistry);
	}

	__declspec(dllexport) const char* MaterialProfile_GetName(MaterialProfile* self)
	{
		return self->name.c_str();
	}

	__declspec(dllexport) const char* MaterialProfile_GetPath(MaterialProfile* self)
	{
		return self->path.c_str();
	}

	__declspec(dllexport) void MaterialProfile_SetPath(MaterialProfile* self, const char* path)
	{
		self->path = path;
	}

	__declspec(dllexport) void MaterialProfile_SetName(MaterialProfile* self, const char* name)
	{
		self->name = name;
	}
#pragma endregion
#pragma region object_profile
	__declspec(dllexport) ObjectProfile* ObjectProfile_new(const char* guid)
	{
		return GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<ObjectProfile>(guid);
	}

	__declspec(dllexport) void ObjectProfile_gc(ObjectProfile* self)
	{
		//delete self;
	}

	__declspec(dllexport) void ObjectProfile_LoadLuaFile(ObjectProfile* self, const char * fileName)
	{
		self->LoadLuaFile(fileName);
	}

	__declspec(dllexport) void ObjectProfile_AddUniformBuffer(ObjectProfile* self, ShaderBlock* ub)
	{
		self->AddShaderBlock(ub);
	}

	__declspec(dllexport) void ObjectProfile_RemoveUniformBuffer(ObjectProfile* self, ShaderBlock* ub)
	{
		self->RemoveShaderBlock(ub);
	}

	__declspec(dllexport) void ObjectProfile_AddUniformBuffers(ObjectProfile* self, Shader* shader)
	{
		self->AddShaderBlocks(shader->objectUniformBuffers);
	}

	__declspec(dllexport) void ObjectProfile_AddGlobalUniformBuffers(ObjectProfile* self, Shader* shader)
	{
		self->AddShaderBlocks(shader->globalUniformBuffers);
	}

	__declspec(dllexport) void ObjectProfile_UpdateProfileFromDataRegistry(ObjectProfile* self, const DataRegistry* dataRegistry)
	{
		self->UpdateProfileFromDataRegistry(*dataRegistry);
	}

	__declspec(dllexport) const char* ObjectProfile_GetName(ObjectProfile* self)
	{
		return self->name.c_str();
	}

	__declspec(dllexport) void ObjectProfile_SetName(ObjectProfile* self, const char* name)
	{
		self->name = name;
	}
#pragma endregion
#pragma region cpu_buffer_data
	__declspec(dllexport) CPUBlockData* CPUBlockData_new()
	{
		return GraphicsStorage::assetRegistry.AllocAsset<CPUBlockData>();
	}

	__declspec(dllexport) void CPUBlockData_gc(CPUBlockData* self)
	{
		//delete self;
	}

	__declspec(dllexport) void CPUBlockData_SetData(CPUBlockData* self, int offset, void* data, int size)
	{
		self->SetData(offset, data, size);
	}
#pragma endregion
#pragma region uniform_buffer_data
	__declspec(dllexport) ShaderBlockData* UniformBufferData_new(ShaderBlock* newUniformBuffer)
	{
		ShaderBlockData* data = GraphicsStorage::assetRegistry.AllocAsset<ShaderBlockData>(newUniformBuffer);
		GraphicsStorage::uniformBuffersDatas.push_back(data);
		return data;
	}

	__declspec(dllexport) void UniformBufferData_gc(ShaderBlockData* self)
	{
		//delete self;
	}

	__declspec(dllexport) void UniformBufferData_SetData(ShaderBlockData* self, const char* uniformName, void* data, int size)
	{
		self->SetData(uniformName, data, size);
	}

	__declspec(dllexport) const char* UniformBufferData_GetName(ShaderBlockData* self)
	{
		return self->shaderBlock->name.c_str();
	}
#pragma endregion
#pragma region scene
	__declspec(dllexport) SceneGraph* SceneGraph_new()
	{
		return SceneGraph::Instance();
	}

	__declspec(dllexport) void SceneGraph_gc(SceneGraph* self)
	{
		//self->Clear();
	}

	__declspec(dllexport) Node* SceneGraph_GetRootNode(SceneGraph* self)
	{
		return &SceneGraph::Instance()->SceneRoot;
	}

	__declspec(dllexport) void SceneGraph_Clear(SceneGraph* self)
	{
		self->Clear();
	}

	__declspec(dllexport) Object* SceneGraph_AddChild(SceneGraph* self)
	{
		return self->addChild();
	}

	__declspec(dllexport) Object* SceneGraph_AddChildTo(SceneGraph* self, Node* parent)
	{
		return self->addChild();
	}

	__declspec(dllexport) void SceneGraph_Update(SceneGraph* self)
	{
		self->Update();
	}
	/*
	__declspec(dllexport) Vector3 SceneGraph_GenerateRandomIntervallVectorCubic(Scene* self, int min, int max)
	{
		return self->generateRandomIntervallVectorCubic(min, max);
	}

	__declspec(dllexport) Vector3 SceneGraph_GenerateRandomIntervallVectorFlat(Scene* self, int min, int max, int axis = 0, int axisHeight = 0)
	{
		return self->generateRandomIntervallVectorFlat(min, max);
	}

	__declspec(dllexport) Vector3 SceneGraph_GenerateRandomIntervallVectorSpherical(Scene* self, int min, int max)
	{
		return self->generateRandomIntervallVectorSpherical(min, max);
	}
	*/
	__declspec(dllexport) void SceneGraph_InitializeSceneTree(SceneGraph* self)
	{
		self->InitializeSceneTree();
	}

	__declspec(dllexport) void SceneGraph_SwitchObjectMovableMode(SceneGraph* self, Object* object, bool movable)
	{
		self->SwitchObjectMovableMode(object, movable);
	}

	__declspec(dllexport) void SceneGraph_SwitchNodeMovableMode(SceneGraph* self, Node* node, bool movable)
	{
		self->SwitchNodeMovableMode(node, movable);
	}

	__declspec(dllexport) void SceneGraph_Parent(Node* self, Node* newParent)
	{
		SceneGraph::Instance()->Parent(self, newParent);
	}

	__declspec(dllexport) void SceneGraph_ParentWithOffset(Node* self, Node* newParent, Vector3& newLocalPos, Quaternion& newLocalOri, Vector3& newLocalScale)
	{
		SceneGraph::Instance()->ParentWithOffset(self, newParent, (glm::vec3&)newLocalPos, (glm::quat&)newLocalOri, (glm::vec3&)newLocalScale);
	}

	__declspec(dllexport) void SceneGraph_ParentWithOffsetTransform(Node* self, Node* newParent, Matrix4& newLocalTransform)
	{
		SceneGraph::Instance()->ParentWithOffset(self, newParent, (glm::mat4&)newLocalTransform);
	}
	
	__declspec(dllexport) void SceneGraph_ParentInPlace(Node* self, Node* newParent)
	{
		SceneGraph::Instance()->ParentInPlace(self, newParent);
	}

	__declspec(dllexport) void SceneGraph_Unparent(Node* self)
	{
		SceneGraph::Instance()->Unparent(self);
	}

	__declspec(dllexport) void SceneGraph_UnparentTo(Node* self, Node* newParent)
	{
		SceneGraph::Instance()->Unparent(self, newParent);
	}

	__declspec(dllexport) void SceneGraph_UnparentInPlace(Node* self)
	{
		SceneGraph::Instance()->UnparentInPlace(self);
	}

	__declspec(dllexport) void SceneGraph_UnparentInPlaceTo(Node* self, Node* newParent)
	{
		SceneGraph::Instance()->UnparentInPlace(self, newParent);
	}
#pragma endregion
#pragma region object
	__declspec(dllexport) Object* Object_new(const char* guid)
	{
		Object* asset = GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<Object>(guid);
		SceneGraph::Instance()->addObject(asset);
		SceneGraph::Instance()->pickingList[asset->ID] = asset;
		//const void* addressPtr = static_cast<const void*>(asset);
		//std::stringstream ss;
		//ss << addressPtr;
		//std::string addressStr = ss.str();
		//asset->name = name + std::string("##") + addressStr;
		//auto id = GraphicsStorage::assetRegistry.GetAssetID(asset);
		//asset->name = std::string(name) + "##" + uuids::to_string(id);
		//GraphicsStorage::objects[asset->name] = asset;
		return asset;
	}

	__declspec(dllexport) void Object_gc(Object* self)
	{
		//delete self;
	}

	__declspec(dllexport) void Object_AddComponent(Object* self, Component* newComponent, const char* componentName, bool isDynamic)
	{
		if (strcmp(componentName, "DirectionalLight") == 0)
		{
			self->AddComponent((DirectionalLight*)newComponent, isDynamic);
		}
		else if (strcmp(componentName, "PointLight") == 0)
		{
			self->AddComponent((PointLight*)newComponent, isDynamic);
		}
		else if (strcmp(componentName, "SpotLight") == 0)
		{
			self->AddComponent((SpotLight*)newComponent, isDynamic);
		}
		else if (strcmp(componentName, "RigidBody") == 0)
		{
			self->AddComponent((RigidBody*)newComponent, isDynamic);
		}
		else if (strcmp(componentName, "Transform") == 0)
		{
			self->AddComponent((Node*)newComponent, isDynamic);
		}
		else if (strcmp(componentName, "Bounds") == 0)
		{
			self->AddComponent((Bounds*)newComponent, isDynamic);
		}
		else if (strcmp(componentName, "FastInstanceSystem") == 0)
		{
			self->AddComponent((FastInstanceSystem*)newComponent, isDynamic);
		}
		else if (strcmp(componentName, "InstanceSystem") == 0)
		{
			self->AddComponent((InstanceSystem*)newComponent, isDynamic);
		}
		else if (strcmp(componentName, "BoundinBoxSystem") == 0)
		{
			self->AddComponent((BoundingBoxSystem*)newComponent, isDynamic);
		}
		else if (strcmp(componentName, "LineSystem") == 0)
		{
			self->AddComponent((LineSystem*)newComponent, isDynamic);
		}
		else if (strcmp(componentName, "PointSystem") == 0)
		{
			self->AddComponent((PointSystem*)newComponent, isDynamic);
		}
		else if (strcmp(componentName, "ParticleSystem") == 0)
		{
			self->AddComponent((ParticleSystem*)newComponent, isDynamic);
		}
		else if (strcmp(componentName, "ScriptsComponent") == 0)
		{
			self->AddComponent((ScriptsComponent*)newComponent, isDynamic);
		}
	}

	__declspec(dllexport) void Object_RemoveComponent(Object* self, Component* newComponent)
	{
		self->RemoveComponent(newComponent);
	}

	__declspec(dllexport) void Object_UpdateComponentDynamicState(Object* self, Component* component, bool isDynamic)
	{
		self->SetComponentDynamicState(component, isDynamic);
	}

	__declspec(dllexport) void Object_AddMaterial(Object* self, Material* mat)
	{
		self->AddMaterial(mat);
	}

	__declspec(dllexport) void Object_AssignMaterial(Object* self, Material* mat, int sequenceIndex, int materialSlot)
	{
		self->AssignMaterial(mat, sequenceIndex, materialSlot);
	}

	__declspec(dllexport) void Object_Update(Object* self)
	{
		self->Update();
	}

	__declspec(dllexport) void Object_UpdateComponents(Object* self)
	{
		self->UpdateComponents();
	}

	__declspec(dllexport) void Object_StopDrawing(Object* self)
	{
		self->StopDrawing();
	}

	__declspec(dllexport) void Object_DrawOnce(Object* self)
	{
		self->DrawOnce();
	}

	__declspec(dllexport) void Object_DrawAlways(Object* self)
	{
		self->DrawAlways();
	}

	__declspec(dllexport) bool Object_CanDraw(Object* self)
	{
		return self->CanDraw();
	}

	__declspec(dllexport) bool Object_CanDrawAlways(Object* self)
	{
		return self->CanDrawAlways();
	}

	__declspec(dllexport) void Object_UpdateDrawState(Object* self)
	{
		self->UpdateDrawState();
	}

	__declspec(dllexport) void Object_ResetIDs(Object* self)
	{
		self->ResetIDs();
	}

	__declspec(dllexport) unsigned int Object_Count(Object* self)
	{
		return self->Count();
	}

	__declspec(dllexport) Component* Object_GetComponent(Object* self, const char * componentName)
	{
		if (strcmp(componentName, "DirectionalLight") == 0)
		{
			return self->GetComponent<DirectionalLight>();
		}
		else if (strcmp(componentName, "PointLight") == 0)
		{
			return self->GetComponent<PointLight>();
		}
		else if (strcmp(componentName, "SpotLight") == 0)
		{
			return self->GetComponent<SpotLight>();
		}
		else if (strcmp(componentName, "RigidBody") == 0)
		{
			return self->GetComponent<RigidBody>();
		}
		else if (strcmp(componentName, "Transform") == 0)
		{
			return self->GetComponent<Node>();
		}
		else if (strcmp(componentName, "Bounds") == 0)
		{
			return self->GetComponent<Bounds>();
		}
		else if (strcmp(componentName, "FastInstanceSystem") == 0)
		{
			return self->GetComponent<FastInstanceSystem>();
		}
		else if (strcmp(componentName, "InstanceSystem") == 0)
		{
			return self->GetComponent<InstanceSystem>();
		}
		else if (strcmp(componentName, "BoundinBoxSystem") == 0)
		{
			return self->GetComponent<BoundingBoxSystem>();
		}
		else if (strcmp(componentName, "LineSystem") == 0)
		{
			return self->GetComponent<LineSystem>();
		}
		else if (strcmp(componentName, "PointSystem") == 0)
		{
			return self->GetComponent<PointSystem>();
		}
		else if (strcmp(componentName, "ParticleSystem") == 0)
		{
			return self->GetComponent<ParticleSystem>();
		}
		else if (strcmp(componentName, "ScriptsComponent") == 0)
		{
			return self->GetComponent<ScriptsComponent>();
		}
		else
		{
			return nullptr;
		}
	}

	__declspec(dllexport) unsigned int Object_GetID(Object* self)
	{
		return self->ID;
	}

	__declspec(dllexport) DataRegistry* Object_GetDataRegistry(Object* self)
	{
		//ObjectProfile* op = self->GetComponent<ObjectProfile>();
		//return op != nullptr ? &op->registry : nullptr;
		return &self->registry;
	}

	__declspec(dllexport) void* Object_GetID_Data(Object* self)
	{
		return &self->ID;
	}

	__declspec(dllexport) void* Object_Transform_Data(Object* self)
	{
		Node* node = self->GetComponent<Node>();
		return node != nullptr ? &node->TopDownTransformF : nullptr;
	}

	__declspec(dllexport) Node* Object_GetNode(Object* self)
	{
		return self->node;
	}

	__declspec(dllexport) const char* Object_GetName(Object* self)
	{
		return self->GetName().c_str();
	}

	__declspec(dllexport) void Object_SetName(Object* self, const char* name)
	{
		self->name = name;
	}

	__declspec(dllexport) const char* Object_GetPath(Object* self)
	{
		return self->GetPath().c_str();
	}

	__declspec(dllexport) void Object_SetPath(Object* self, const char* path)
	{
		self->path = path;
	}

	__declspec(dllexport) Component** Object_GetComponents(Object* self)
	{
		if (self->components.size() == 0)
		{
			return nullptr;
		}
		Component** comps = new Component*[self->components.size()];
		int i = 0;
		for (auto comp : self->components)
		{
			comps[i] = comp.second;
			i++;
		}
		return comps;
	}

	__declspec(dllexport) int Object_GetComponentsSize(Object* self)
	{
		return (int)self->components.size();
	}

	__declspec(dllexport) Component** Object_GetDynamicComponents(Object* self)
	{
		if (self->dynamicComponents.size() == 0)
		{
			return nullptr;
		}
		Component** comps = new Component*[self->dynamicComponents.size()];
		int i = 0;
		for (auto comp : self->dynamicComponents)
		{
			comps[i] = comp.second;
			i++;
		}
		return comps;
	}

	__declspec(dllexport) int Object_GetDynamicComponentsSize(Object* self)
	{
		return (int)self->dynamicComponents.size();
	}

	__declspec(dllexport) void Editor_DeleteComponents(Component** ptrToArrayOfComponents)
	{
		delete[] ptrToArrayOfComponents;
	}

	__declspec(dllexport) int Object_GetNumberOfMaterialSequences(Object* self)
	{
		return (int)self->materials.size();
	}

	__declspec(dllexport) Material** Object_GetMaterialSequence(Object* self, int sequenceIndex)
	{
		if (self->materials.size() == 0)
		{
			return nullptr;
		}
		if (self->materials.size() > sequenceIndex)
		{
			return self->materials[sequenceIndex].data();
		}
		return nullptr;
	}

	__declspec(dllexport) int Object_GetSizeOfMaterialSequence(Object* self, int sequenceIndex)
	{
		if (self->materials.size() == 0)
		{
			return 0;
		}
		if (self->materials.size() > sequenceIndex)
		{
			return self->materials[sequenceIndex].size();
		}
		return 0;
	}

	_declspec(dllexport) Object* Object_GetParentObject(Object* self)
	{
		return self->GetParentObject();
	}

	_declspec(dllexport) Material* Object_GetMaterialByIndex(Object* self, int index)
	{
		return self->GetMaterial(index);
	}

	_declspec(dllexport) Material* Object_GetMaterialByName(Object* self, const char* name)
	{
		return self->GetMaterial(name);
	}
#pragma endregion
#pragma region camera
	__declspec(dllexport) Camera* Camera_new(const char* guid)
	{
		return GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<Camera>(guid);
	}

	__declspec(dllexport) void Camera_gc(Camera* self)
	{
		//delete self;
	}

	__declspec(dllexport) void* Camera_GetFarPlaneData(Camera* self)
	{
		return &self->far;
	}

	__declspec(dllexport) void* Camera_GetNearPlaneData(Camera* self)
	{
		return &self->near;
	}

	__declspec(dllexport) void* Camera_GetFovData(Camera* self)
	{
		return &self->fov;
	}

	__declspec(dllexport) void* Camera_GetSpeedData(Camera* self)
	{
		return &self->speed;
	}

	__declspec(dllexport) void* Camera_GetScreenSizeData(Camera* self)
	{
		return &self->screenSize;
	}

	__declspec(dllexport) void Camera_Update(Camera* self, double deltaTime)
	{
		self->Update(deltaTime);
	}

	__declspec(dllexport) void Camera_CalculateViewMatrix(Camera* self)
	{
		self->CalculateViewMatrix();
	}

	__declspec(dllexport) void Camera_UpdateProjection(Camera* self)
	{
		self->UpdateProjection();
	}

	__declspec(dllexport) void Camera_ComputeVectors(Camera* self)
	{
		self->ComputeVectors();
	}

	__declspec(dllexport) void Camera_UpdatePosition(Camera* self, double deltaTime)
	{
		self->UpdatePosition(deltaTime);
	}

	__declspec(dllexport) void Camera_UpdateOrientation(Camera* self, double mouseX, double mouseY)
	{
		self->UpdateOrientation(mouseX, mouseY);
	}

	__declspec(dllexport) void Camera_UpdateSize(Camera* self, int width, int height)
	{
		self->UpdateSize(width, height);
	}

	__declspec(dllexport) void Camera_ConvertMousePosToWorldDir(Camera* self, double mousePosX, double mousePosY, Vector3F& mouseInWorld)
	{
		(glm::vec3&)mouseInWorld = self->ConvertMousePosToWorldDir(mousePosX, mousePosY);
	}

	__declspec(dllexport) void Camera_GetInitPos(Camera* self, Vector3F& initPos)
	{
		(glm::vec3&)initPos = self->GetInitPos();
	}

	__declspec(dllexport) void Camera_GetPosition(Camera* self, Vector3F& position)
	{
		(glm::vec3&)position = self->GetPosition();
	}

	__declspec(dllexport) void Camera_GetPosition2(Camera* self, Vector3F& position2)
	{
		(glm::vec3&)position2 = self->GetPosition2();
	}

	__declspec(dllexport) void Camera_SetPosition(Camera* self, Vector3& vec)
	{
		self->SetPosition((glm::vec3&)vec);
	}

	__declspec(dllexport) bool Camera_IsHoldingForward(Camera* self)
	{
		return self->holdingForward;
	}

	__declspec(dllexport) bool Camera_IsHoldingBackward(Camera* self)
	{
		return self->holdingBackward;
	}

	__declspec(dllexport) bool Camera_IsHoldingRight(Camera* self)
	{
		return self->holdingRight;
	}

	__declspec(dllexport) bool Camera_IsHoldingLeft(Camera* self)
	{
		return self->holdingLeft;
	}

	__declspec(dllexport) bool Camera_IsHoldingUp(Camera* self)
	{
		return self->holdingUp;
	}

	__declspec(dllexport) bool Camera_IsHoldingDown(Camera* self)
	{
		return self->holdingDown;
	}

	__declspec(dllexport) void* Camera_GetProjectionData(Camera* self)
	{
		return &self->ProjectionMatrix;
	}

	__declspec(dllexport) void* Camera_GetViewData(Camera* self)
	{
		return &self->ViewMatrix;
	}

	__declspec(dllexport) int Camera_GetWidth(Camera* self)
	{
		return (int)self->windowWidth;
	}

	__declspec(dllexport) int Camera_GetHeight(Camera* self)
	{
		return (int)self->windowHeight;
	}

	__declspec(dllexport) float Camera_GetFov(Camera* self)
	{
		return self->fov;
	}

	__declspec(dllexport) void Camera_SetFov(Camera* self, float newFov)
	{
		self->fov = newFov;
	}

	__declspec(dllexport) float Camera_GetNearPlane(Camera* self)
	{
		return self->near;
	}

	__declspec(dllexport) void Camera_SetNearPlane(Camera* self, float newNear)
	{
		self->near = newNear;
	}

	__declspec(dllexport) float Camera_GetFarPlane(Camera* self)
	{
		return self->far;
	}

	__declspec(dllexport) void Camera_SetFarPlane(Camera* self, float newFar)
	{
		self->far = newFar;
	}

	__declspec(dllexport) Vector3* Camera_GetUp(Camera* self)
	{
		return (Vector3*)& self->up;
	}

	__declspec(dllexport) Vector3* Camera_GetRight(Camera* self)
	{
		return (Vector3*)&self->right;
	}

	__declspec(dllexport) Vector3* Camera_GetForward(Camera* self)
	{
		return (Vector3*)&self->forward;
	}

	__declspec(dllexport) float Camera_GetSpeed(Camera* self)
	{
		return self->speed;
	}

	__declspec(dllexport) void Camera_SetSpeed(Camera* self, float newSpeed)
	{
		self->speed = newSpeed;
	}

#pragma endregion
#pragma region material
	__declspec(dllexport) Material* Material_new(const char* guid)
	{
		return GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<Material>(guid);
	}

	__declspec(dllexport) void Material_gc(Material* self)
	{
		//delete self;
	}

	__declspec(dllexport) const char* Material_GetPath(Material* self)
	{
		return self->path.c_str();
	}

	__declspec(dllexport) void Material_SetPath(Material* self, const char* path)
	{
		self->path = path;
	}

	__declspec(dllexport) void Material_AssignShader(Material* self, Shader* shader)
	{
		self->AssignShader(shader);
	}

	__declspec(dllexport) void Material_AssignRenderProfile(Material* self, RenderProfile* renderProfile)
	{
		self->AssignRenderProfile(renderProfile);
	}

	__declspec(dllexport) void Material_AssignTextureProfile(Material* self, TextureProfile* textureProfile)
	{
		self->AssignTextureProfile(textureProfile);
	}

	__declspec(dllexport) void Material_AssignMaterialProfile(Material* self, MaterialProfile* materialProfile)
	{
		self->AssignMaterialProfile(materialProfile);
	}

	__declspec(dllexport) void Material_AssignMesh(Material* self, VertexArray* mesh)
	{
		self->AssignMesh(mesh);
	}

	__declspec(dllexport) void Material_AssignObjectProfile(Material* self, ObjectProfile* objectProfile)
	{
		self->AssignObjectProfile(objectProfile);
	}

	__declspec(dllexport) void Material_AssignRenderPass(Material* self, RenderPass* renderPass)
	{
		self->AssignRenderPass(renderPass);
	}

	__declspec(dllexport) const char* Material_GetName(Material* self)
	{
		return self->name.c_str();
	}

	__declspec(dllexport) void Material_SetName(Material* self, const char* name)
	{
		self->name = name;
	}

	__declspec(dllexport) RenderPass* Material_GetRenderPass(Material* self)
	{
		return self->rps;
	}

	__declspec(dllexport) MaterialProfile* Material_GetMaterialProfile(Material* self)
	{
		return self->mp;
	}

	__declspec(dllexport) RenderProfile* Material_GetRenderProfile(Material* self)
	{
		return self->rp;
	}

	__declspec(dllexport) TextureProfile* Material_GetTextureProfile(Material* self)
	{
		return self->tp;
	}

	__declspec(dllexport) ObjectProfile* Material_GetObjectProfile(Material* self)
	{
		return self->op;
	}

	__declspec(dllexport) Shader* Material_GetShader(Material* self)
	{
		return self->shader;
	}

	__declspec(dllexport) VertexArray* Material_GetVertexArray(Material* self)
	{
		return self->vao;
	}

	__declspec(dllexport) bool Material_GetUnbound(Material* self)
	{
		return self->unbound;
	}

	__declspec(dllexport) void Material_SetUnbound(Material* self, bool unbound)
	{
		self->unbound = unbound;
	}
#pragma endregion
#pragma region node
	//__declspec(dllexport) Node* Node_new(const Vector3F& position, const QuaternionF& rotation, const Vector3F& scale, bool isDynamic)
	__declspec(dllexport) Node* Node_new()
	{
		Node* newNode = GraphicsStorage::assetRegistry.AllocAsset<Node>();
		//newNode->SetPosition(position.toDouble());
		//newNode->SetOrientation(rotation.toDouble());
		//newNode->SetScale(scale.toDouble());
		//newNode->SetMovable(isDynamic);
		return newNode;
	}

	__declspec(dllexport) void Node_gc(Node* self)
	{
		//delete self;
	}

	__declspec(dllexport) Component* Node_AsComponent(Node* self)
	{
		return self;
	}

	__declspec(dllexport) Node* Component_AsNode(Component* self)
	{
		return dynamic_cast<Node*>(self);
	}

	__declspec(dllexport) void Node_SetPosition(Node* self, const Vector3F& vector)
	{
		self->SetPosition((glm::vec3&)vector);
	}

	__declspec(dllexport) void Node_SetScale(Node* self, const Vector3F& vector)
	{
		self->SetScale((glm::vec3&)vector);
	}

	__declspec(dllexport) void Node_Translate(Node* self, const Vector3F& vector)
	{
		self->Translate((glm::vec3&)vector);
	}

	__declspec(dllexport) void Node_SetOrientation(Node* self, const QuaternionF& q)
	{
		self->SetOrientation((glm::quat&)q);
	}

	__declspec(dllexport) void Node_SetRotation(Node* self, const Matrix4F& m)
	{
		self->SetRotation((glm::mat4&)m);
	}

	__declspec(dllexport) void Node_GetLocalPosition(Node* self, Vector3F& localPos)
	{
		(glm::vec3&)localPos = self->GetLocalPosition();
	}

	__declspec(dllexport) void Node_GetLocalOrientation(Node* self, QuaternionF& localOrientation)
	{
		(glm::quat&)localOrientation = self->GetLocalOrientation();
	}

	__declspec(dllexport) void Node_GetLocalScale(Node* self, Vector3F& localScale)
	{
		(glm::vec3&)localScale = self->GetLocalScale();
	}

	__declspec(dllexport) void Node_GetWorldRotation3(Node* self, Matrix3F& worldRotation)
	{
		(glm::mat3&)worldRotation = self->GetWorldRotation3();
	}

	__declspec(dllexport) void Node_GetWorldRotation(Node* self, Matrix4F& worldRotation)
	{
		(glm::mat4&)worldRotation = self->GetWorldRotation();
	}

	__declspec(dllexport) void Node_GetWorldOrientation(Node* self, QuaternionF& worldOrientation)
	{
		(glm::quat&)worldOrientation = self->GetWorldOrientation();
	}

	__declspec(dllexport) void Node_extractScale(Node* self, Vector3F& extractedWorldScale)
	{
		(glm::vec3&)extractedWorldScale = self->extractScale();
	}

	__declspec(dllexport) void Node_getScale(Node* self, Vector3F& worldScale)
	{
		(glm::vec3&)worldScale = self->getScale();
	}

	__declspec(dllexport) void Node_GetWorldPosition(Node* self, Vector3F& worldPosition)
	{
		(glm::vec3&)worldPosition = self->GetWorldPosition();
	}

	__declspec(dllexport) void Node_Parent(Node* self, Node* newParent)
	{
		self->Parent(newParent);
	}

	__declspec(dllexport) void Node_ParentWithOffset(Node* self, Node* newParent, Vector3F& newLocalPos, QuaternionF& newLocalOri, Vector3F& newLocalScale)
	{
		self->ParentWithOffset(newParent, (glm::vec3&)newLocalPos, (glm::quat&)newLocalOri, (glm::vec3&)newLocalScale);
	}

	__declspec(dllexport) void Node_ParentWithOffsetTransform(Node* self, Node* newParent, Matrix4F& newLocalTransform)
	{
		self->ParentWithOffset(newParent, (glm::mat4&)newLocalTransform);
	}

	__declspec(dllexport) void Node_ParentInPlace(Node* self, Node* newParent)
	{
		self->ParentInPlace(newParent);
	}

	__declspec(dllexport) void Node_Unparent(Node* self)
	{
		self->Unparent();
	}

	__declspec(dllexport) void Node_UnparentTo(Node* self, Node* newParent)
	{
		self->Unparent(newParent);
	}

	__declspec(dllexport) void Node_UnparentInPlace(Node* self)
	{
		self->UnparentInPlace();
	}

	__declspec(dllexport) void Node_UnparentInPlaceTo(Node* self, Node* newParent)
	{
		self->UnparentInPlace(newParent);
	}

	__declspec(dllexport) void Node_UnparentInPlaceToNearestStaticAncestor(Node* self)
	{
		self->UnparentInPlaceToNearestStaticAncestor();
	}

	__declspec(dllexport) void Node_UnparentInPlaceToNearestTotalStaticAncestor(Node* self)
	{
		self->UnparentInPlaceToNearestTotalStaticAncestor();
	}

	__declspec(dllexport) void Node_UnparentInPlaceToNearestMovableAncestor(Node* self)
	{
		self->UnparentInPlaceToNearestMovableAncestor();
	}

	__declspec(dllexport) Node* Node_FindNearestStaticAncestor(Node* self)
	{
		return self->FindNearestStaticAncestor();
	}

	__declspec(dllexport) Node* Node_FindNearestTotalStaticAncestor(Node* self)
	{
		return self->FindNearestTotalStaticAncestor();
	}

	__declspec(dllexport) Node* Node_FindNearestMovableAncestor(Node* self)
	{
		return self->FindNearestMovableAncestor();
	}

	__declspec(dllexport) int Node_IsAncestorOf(Node* self, Node* node)
	{
		return self->IsAncestorOf(node);
	}

	__declspec(dllexport) bool Node_SetMovable(Node* self, bool isMovable)
	{
		return self->SetMovable(isMovable);
	}

	__declspec(dllexport) bool Node_GetMovable(Node* self)
	{
		return self->GetMovable();
	}

	__declspec(dllexport) bool Node_GetTotalMovable(Node* self)
	{
		return self->GetTotalMovable();
	}

	__declspec(dllexport) bool Node_UpdateMovable(Node* self, Node* node)
	{
		return self->UpdateMovable(node);
	}

	__declspec(dllexport) Node** Node_GetChildren(Node* self)
	{
		return self->children.data();
	}

	__declspec(dllexport) int Node_GetChildrenCount(Node* self)
	{
		return (int)self->children.size();
	}

	__declspec(dllexport) Object* Node_GetObject(Node* self)
	{
		return self->object;
	}
#pragma endregion
#pragma region data_info
	__declspec(dllexport) DataInfo* DataInfo_new(void* address, int size, PropertyType type)
	{
		return GraphicsStorage::assetRegistry.AllocAsset<DataInfo>(address, size, type);
	}

	__declspec(dllexport) void DataInfo_gc(DataInfo* self)
	{
		//delete self;
	}

	__declspec(dllexport) void DataInfo_SetData(const DataInfo* self, void* data)
	{
		self->SetData(data);
	}
#pragma endregion
#pragma region data_registry
	__declspec(dllexport) DataRegistry* DataRegistry_new(const char* guid)
	{
		return GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<DataRegistry>(guid);
	}

	__declspec(dllexport) void DataRegistry_gc(DataRegistry* self)
	{
		//delete self;
	}

	__declspec(dllexport) void DataRegistry_RegisterProperty(DataRegistry* self, const char* name, void* memoryAddress, int size, PropertyType type)
	{
		self->RegisterProperty(name, memoryAddress, size, type);
	}

	__declspec(dllexport) void DataRegistry_RegisterPropertyWithDataInfo(DataRegistry* self, const char* name, const DataInfo* dataInfo)
	{
		self->RegisterProperty(name, dataInfo);
	}

	__declspec(dllexport) const DataInfo* DataRegistry_AddAndRegisterProperty(DataRegistry* self, const char* name, void* data, int size, PropertyType type)
	{
		return self->AddAndRegisterProperty(name, data, size, type);
	}

	__declspec(dllexport) const DataInfo* DataRegistry_GetProperty(DataRegistry* self, const char* name)
	{
		return self->GetProperty(name);
	}

	__declspec(dllexport) void DataRegistry_SetProperty(DataRegistry* self, const char* name, void* data)
	{
		self->SetProperty(name, data);
	}
#pragma endregion
#pragma region Times
	_declspec(dllexport) Times* Times_new()
	{
		return Times::Instance();
	}

	__declspec(dllexport) void Times_gc(Times* self)
	{
		//delete self;
	}

	__declspec(dllexport) void* Times_GetCurrentTimeData(Times* self)
	{
		return &self->currentTimeF;
	}

	__declspec(dllexport) void* Times_GetDeltaTimeData(Times* self)
	{
		return &self->deltaTimeF;
	}

	__declspec(dllexport) void* Times_GetDeltaTimeInverseData(Times* self)
	{
		return &self->dtInvF;
	}

	__declspec(dllexport) bool Times_GetPaused(Times* self)
	{
		return self->paused;
	}

	__declspec(dllexport) void* Times_GetPreviousTimeData(Times* self)
	{
		return &self->previousTimeF;
	}

	__declspec(dllexport) void* Times_GetTimeModifierData(Times* self)
	{
		return &self->timeModifierF;
	}

	__declspec(dllexport) void* Times_GetTimeStepData(Times* self)
	{
		return &self->timeStepF;
	}
#pragma endregion
#pragma region camera_manager
	_declspec(dllexport) CameraManager* CameraManager_new()
	{
		return CameraManager::Instance();
	}

	__declspec(dllexport) void CameraManager_gc(CameraManager* self)
	{
		//delete self;
	}
	
	__declspec(dllexport) void CameraManager_AddCamera(CameraManager* self, const char* name, Camera* camera)
	{
		self->AddCamera(name, camera);
	}

	__declspec(dllexport) Camera* CameraManager_GetCurrentCamera(CameraManager* self)
	{
		return self->GetCurrentCamera();
	}

	__declspec(dllexport) void CameraManager_SetCurrentCamera(CameraManager* self, const char* name)
	{
		return self->SetCurrentCamera(name);
	}

	__declspec(dllexport) void CameraManager_Update(CameraManager* self, double deltaTime)
	{
		return self->Update(deltaTime);
	}

	__declspec(dllexport) void* CameraManager_GetViewProjectionData(CameraManager* self)
	{
		return &self->ViewProjectionF;
	}

	__declspec(dllexport) void* CameraManager_GetViewData(CameraManager* self)
	{
		return &self->ViewF;
	}

	__declspec(dllexport) void* CameraManager_GetProjectionData(CameraManager* self)
	{
		return &self->ProjectionF;
	}

	__declspec(dllexport) void* CameraManager_GetScreenSizeData(CameraManager* self)
	{
		return (void*)&self->screenSize;
	}

	__declspec(dllexport) void* CameraManager_GetCameraPositionData(CameraManager* self)
	{
		return &self->cameraPos;
	}

	__declspec(dllexport) void* CameraManager_GetCameraUpData(CameraManager* self)
	{
		return &self->cameraUp;
	}

	__declspec(dllexport) void* CameraManager_GetCameraRightData(CameraManager* self)
	{
		return &self->cameraRight;
	}

	__declspec(dllexport) void* CameraManager_GetCameraForwardData(CameraManager* self)
	{
		return &self->cameraForward;
	}

	__declspec(dllexport) void* CameraManager_GetFarPlaneData(CameraManager* self)
	{
		return &self->far;
	}

	__declspec(dllexport) void* CameraManager_GetNearPlaneData(CameraManager* self)
	{
		return &self->near;
	}

#pragma endregion
#pragma region directional_light

	__declspec(dllexport) DirectionalLight* DirectionalLight_DirectionalLight_new()
	{
		return GraphicsStorage::assetRegistry.AllocAsset<DirectionalLight>();
	}

	__declspec(dllexport) void DirectionalLight_gc(DirectionalLight* self)
	{
		//delete self;
	}

	__declspec(dllexport) Component* DirectionalLight_AsComponent(DirectionalLight* self)
	{
		return self;
	}

	__declspec(dllexport) DirectionalLight* Component_AsDirectionalLight(Component* self)
	{
		return dynamic_cast<DirectionalLight*>(self);
	}

	__declspec(dllexport) void DirectionalLight_SetProjectionRadius(DirectionalLight* self, double radius)
	{
		self->SetProjectionRadius(radius);
	}

	__declspec(dllexport) void DirectionalLight_SetActiveBlurLevel(DirectionalLight* self, int level)
	{
		self->activeBlurLevel = level;
	}

	__declspec(dllexport) void DirectionalLight_SetBlurIntensity(DirectionalLight* self, float intensity)
	{
		self->blurIntensity = intensity;
	}

	__declspec(dllexport) void DirectionalLight_SetShadowFadeRange(DirectionalLight* self, float range)
	{
		self->shadowFadeRange = range;
	}

	__declspec(dllexport) void DirectionalLight_SetBlurMode(DirectionalLight* self, BlurMode mode)
	{
		self->blurMode = mode;
	}

	__declspec(dllexport) void DirectionalLight_SetShadowMapActive(DirectionalLight* self, bool active)
	{
		self->shadowMapActive = active;
	}

	__declspec(dllexport) void DirectionalLight_SetShadowMapBlurActive(DirectionalLight* self, bool active)
	{
		self->shadowMapBlurActive = active;
	}

	__declspec(dllexport) void* DirectionalLight_GetActiveBlurLevel_Data(DirectionalLight* self)
	{
		return &self->activeBlurLevel;
	}

	__declspec(dllexport) void* DirectionalLight_GetBlurIntensity_Data(DirectionalLight* self)
	{
		return &self->blurIntensity;
	}

	__declspec(dllexport) void* DirectionalLight_GetBlurMode_Data(DirectionalLight* self)
	{
		return &self->blurMode;
	}

	__declspec(dllexport) void* DirectionalLight_GetShadowMapActive_Data(DirectionalLight* self)
	{
		return &self->shadowMapActive;
	}

	__declspec(dllexport) void* DirectionalLight_GetShadowMapBlurActive_Data(DirectionalLight* self)
	{
		return &self->shadowMapBlurActive;
	}

	__declspec(dllexport) void* DirectionalLight_GetLightInvDir_Data(DirectionalLight* self)
	{
		return &self->LightInvDir;
	}

	__declspec(dllexport) void* DirectionalLight_GetDepthBiasVP_Data(DirectionalLight* self)
	{
		return &self->BiasedLightMatrixVP;
	}

	__declspec(dllexport) void* DirectionalLight_GetShadowFadeRange_Data(DirectionalLight* self)
	{
		return &self->shadowFadeRange;
	}

	__declspec(dllexport) void* DirectionalLight_GetLightRadius_Data(DirectionalLight* self)
	{
		return &self->radius;
	}

	__declspec(dllexport) void* DirectionalLight_GetLightVP_Data(DirectionalLight* self)
	{
		return &self->LightMatrixVP;
	}
#pragma endregion
#pragma region bounds
	__declspec(dllexport) Bounds* Bounds_new()
	{
		return GraphicsStorage::assetRegistry.AllocAsset<Bounds>();
	}

	__declspec(dllexport) void Bounds_gc(Bounds* self)
	{
		//delete self;
	}

	__declspec(dllexport) Component* Bounds_AsComponent(Bounds* self)
	{
		return self;
	}

	__declspec(dllexport) Bounds* Component_AsBounds(Component* self)
	{
		return dynamic_cast<Bounds*>(self);
	}

	__declspec(dllexport) void Bounds_SetUp(Bounds* self, const char* meshName, Vector3F& dimensions, Vector3F& center)
	{
		self->SetUp((glm::vec3&)center, (glm::vec3&)dimensions, meshName);
	}

	__declspec(dllexport) const char* Bounds_GetMeshName(Bounds* self)
	{
		return self->name.c_str();
	}

	__declspec(dllexport) void Bounds_GetDimensions(Bounds* self, Vector3F& dimensions)
	{
		dimensions = (Vector3F&)self->dimensions;
	}

	__declspec(dllexport) void Bounds_GetCenter(Bounds* self, Vector3F& center)
	{
		center = (Vector3F&)self->centerOfMesh;
	}
#pragma endregion
#pragma region particle_system
	__declspec(dllexport) ParticleSystem* ParticleSystem_new()
	{
		return GraphicsStorage::assetRegistry.AllocAsset<ParticleSystem>();
	}

	__declspec(dllexport) void ParticleSystem_gc(ParticleSystem* self)
	{
		//delete self;
	}

	__declspec(dllexport) Component* ParticleSystem_AsComponent(ParticleSystem* self)
	{
		return self;
	}

	__declspec(dllexport) ParticleSystem* Component_AsParticleSystem(Component* self)
	{
		return dynamic_cast<ParticleSystem*>(self);
	}

	__declspec(dllexport) void ParticleSystem_SetDirection(ParticleSystem* self, Vector3F& direction)
	{
		self->SetDirection((glm::vec3&)direction);
	}

	//we can return structs as * & and value
	//in lua we can then do
	//local direction = ffi.new("vec3", ffi.C.ParticleSystem_GetDirection())
	//previously I would create it first in lua
	//local direction = ffi.new("vec3")
	//then get it by passing the created value to function
	//ffi.C.ParticleSystem_GetDirection(direction)
	//this function would change the provided direction variable
	//when ok and possible return by reference
	//when not returning value should be just fine
	__declspec(dllexport) Vector3F& ParticleSystem_GetDirection(ParticleSystem* self)
	{
		return (Vector3F&)self->Direction;
	}

	__declspec(dllexport) void ParticleSystem_SetEmissionRate(ParticleSystem* self, int emissionRate)
	{
		self->SetEmissionRate(emissionRate);
	}

	__declspec(dllexport) int ParticleSystem_GetEmissionRate(ParticleSystem* self)
	{
		return self->DesiredEmissionRate;
	}

	__declspec(dllexport) void ParticleSystem_SetForce(ParticleSystem* self, Vector3F& force)
	{
		self->SetForce((glm::vec3&)force);
	}

	__declspec(dllexport) Vector3F& ParticleSystem_GetForce(ParticleSystem* self)
	{
		return (Vector3F&)self->Force;
	}

	__declspec(dllexport) void ParticleSystem_SetLifeTime(ParticleSystem* self, float lifeTime)
	{
		self->SetLifeTime(lifeTime);
	}

	__declspec(dllexport) float ParticleSystem_GetLifeTime(ParticleSystem* self)
	{
		return self->LifeTime;
	}

	__declspec(dllexport) void ParticleSystem_SetMaxParticles(ParticleSystem* self, int maxParticles)
	{
		self->SetMaxParticles(maxParticles);
	}

	__declspec(dllexport) int ParticleSystem_GetMaxParticles(ParticleSystem* self)
	{
		return self->MaxParticles;
	}

	__declspec(dllexport) void ParticleSystem_SetSize(ParticleSystem* self, float size)
	{
		self->SetSize(size);
	}

	__declspec(dllexport) float ParticleSystem_GetSize(ParticleSystem* self)
	{
		return self->Size;
	}

	__declspec(dllexport) void ParticleSystem_SetSpread(ParticleSystem* self, float spread)
	{
		self->SetSpread(spread);
	}

	__declspec(dllexport) float ParticleSystem_GetSpread(ParticleSystem* self)
	{
		return self->Spread;
	}

	__declspec(dllexport) int ParticleSystem_GetAliveParticlesCount(ParticleSystem* self)
	{
		return self->GetAliveParticlesCount();
	}

	__declspec(dllexport) int ParticleSystem_GetNewParticlesCount(ParticleSystem* self)
	{
		return self->GetNewParticlesCount();
	}
#pragma endregion
#pragma region script_component
	_declspec(dllexport) ScriptsComponent* ScriptsComponent_new()
	{
		ScriptsComponent* script = GraphicsStorage::assetRegistry.AllocAsset<ScriptsComponent>();
		return script;
	}

	__declspec(dllexport) void ScriptsComponent_gc(CameraManager* self)
	{
		//delete self;
	}

	__declspec(dllexport) Component* ScriptsComponent_AsComponent(ScriptsComponent* self)
	{
		return self;
	}

	__declspec(dllexport) ScriptsComponent* Component_AsScriptsComponent(Component* self)
	{
		return dynamic_cast<ScriptsComponent*>(self);
	}

	__declspec(dllexport) void ScriptsComponent_AddScript(ScriptsComponent* self, const char* pathToScript)
	{
		self->AddScript(pathToScript);
	}

	__declspec(dllexport) void ScriptsComponent_RemoveScript(ScriptsComponent* self, Script* script)
	{
		self->RemoveScript(script);
	}

	__declspec(dllexport) Script** ScriptsComponent_GetScripts(ScriptsComponent* self)
	{
		return self->scripts.data();
	}

	__declspec(dllexport) int ScriptsComponent_GetScriptsCount(ScriptsComponent* self)
	{
		return self->scripts.size();
	}
#pragma endregion
#pragma region render
	__declspec(dllexport) Render* Render_new()
	{
		return Render::Instance();
	}

	__declspec(dllexport) void Render_gc(Render* self)
	{
		//delete self;
	}
#pragma endregion
#pragma region Editor
	/*
	__declspec(dllexport) Editor* Editor_new(const char * name)
	{
		auto editor = GraphicsStorage::editors.find(name);
		if (editor != GraphicsStorage::editors.end())
		{
			return editor->second;
		}
		return nullptr;
	}
	*/
	//editor stuff should be in editor
	//we should rethink this, it's overly complex
	//problem with current solution is that we have:
	//object properties - properties of other objects - referenced with names and registered with new names
	//lua properties - properties registered with lua
	//properties - all other properties that are not the two above
	//and removing properties or unloading lua properties is difficult
	//if you unload lua properties it has to be all properties that have been loaded with that lua script
	__declspec(dllexport) void Editor_MarkAsLuaProperty(DataRegistry* registry, const char* luaFile, const char* property)
	{
		GraphicsStorage::luaProperties[registry][luaFile].insert(property);
	}

	__declspec(dllexport) void Editor_MarkAsObjectProperty(DataRegistry* registry, const char* objectGuid, const char* property, const char* newName)
	{
		GraphicsStorage::objectProperties[registry][objectGuid][property] = newName;
	}

	__declspec(dllexport) LuaProperties* Editor_GetLuaProperties(DataRegistry* registry)
	{
		LuaProperties* ops = new LuaProperties();
		int nrOfProperties = (int)GraphicsStorage::luaProperties[registry].size();
		const char** properties = nullptr;
		if (nrOfProperties > 0)
		{
			properties = new const char*[GraphicsStorage::luaProperties[registry].size()];
			int i = 0;
			for (auto& luaFilePropertiesPair : GraphicsStorage::luaProperties[registry])
			{
				properties[i] = luaFilePropertiesPair.first.c_str();
				i++;
			}
		}
		
		ops->properties = properties;
		ops->size = nrOfProperties;
		return ops;
	}

	__declspec(dllexport) ObjectsProperties* Editor_GetObjectsProperties(DataRegistry* registry)
	{
		ObjectsProperties * objectsProperties = new ObjectsProperties();
		int nrOfObjectProperties = (int)GraphicsStorage::objectProperties[registry].size();
		ObjectProperties* properties = nullptr;
		if (nrOfObjectProperties > 0)
		{
			properties = new ObjectProperties[nrOfObjectProperties];
			objectsProperties->properties = properties;
			objectsProperties->size = nrOfObjectProperties;
			int i = 0;
			for (auto& otherObjectPropertyPair : GraphicsStorage::objectProperties[registry])
			{
				properties[i].object = otherObjectPropertyPair.first.c_str();

				int nrOfPropertiesPerObject = (int)otherObjectPropertyPair.second.size();

				const char** propertiesOfObject = new const char*[nrOfPropertiesPerObject];
				const char** propertiesOfObjectNewNames = new const char*[nrOfPropertiesPerObject];

				int j = 0;
				for (auto& oproperty : otherObjectPropertyPair.second)
				{
					propertiesOfObject[j] = oproperty.first.c_str();
					propertiesOfObjectNewNames[j] = oproperty.second.c_str();
					j++;
				}
				properties[i].properties = propertiesOfObject;
				properties[i].newNames = propertiesOfObjectNewNames;
				properties[i].size = nrOfPropertiesPerObject;
				i++;
			}
		}

		objectsProperties->properties = properties;
		objectsProperties->size = nrOfObjectProperties;

		return objectsProperties;
	}

	__declspec(dllexport) AssetProperties* Editor_GetProperties(DataRegistry* registry)
	{
		std::vector<AssetProperty> assetProperties;
		for (auto& propertyBindingPair : registry->pb.bindings)
		{
			bool isLuaOrObjectProperty = false;
			for (auto& luaFilePropertiesPair : GraphicsStorage::luaProperties[registry])
			{
				for (auto& lproperty : luaFilePropertiesPair.second)
				{
					if (lproperty == propertyBindingPair.first)
					{
						isLuaOrObjectProperty = true;
					}
				}
			}
			for (auto& otherObjectPropertyPair : GraphicsStorage::objectProperties[registry])
			{
				for (auto& oproperty : otherObjectPropertyPair.second)
				{
					if (oproperty.first == propertyBindingPair.first)
					{
						isLuaOrObjectProperty = true;
					}
				}
			}
			if (!isLuaOrObjectProperty)
			{
				AssetProperty ap;
				ap.name = propertyBindingPair.first.c_str();
				ap.type = propertyBindingPair.second.info.PropertyTypesAsString[(int)propertyBindingPair.second.info.type];
				ap.value = propertyBindingPair.second.info.dataAddress;
				assetProperties.push_back(ap);
			}
		}
		AssetProperty* assetPropertiesArray = nullptr;
		int nrOfAssetProperties = (int)assetProperties.size();
		if (nrOfAssetProperties > 0) assetPropertiesArray = new AssetProperty[assetProperties.size()];
		for (size_t i = 0; i < nrOfAssetProperties; i++)
		{
			assetPropertiesArray[i] = assetProperties[i];
		}
		AssetProperties* aps = new AssetProperties();
		aps->properties = assetPropertiesArray;
		aps->size = nrOfAssetProperties;
		return aps;
	}

	__declspec(dllexport) void AssetProperties_gc(AssetProperties* self)
	{
		delete[] self->properties;
		delete self;
	}

	__declspec(dllexport) void LuaProperties_gc(LuaProperties* self)
	{
		delete[] self->properties;
		delete self;
	}

	__declspec(dllexport) void ObjectsProperties_gc(ObjectsProperties* self)
	{
		if (self->size > 0)
		{
			delete[] self->properties->properties;
			delete[] self->properties->newNames;
			delete self->properties;
		}
		delete self;
	}

#pragma endregion
#pragma region GraphicsManager
	//__declspec(dllexport) void LoadModel(const char* modelPath)
	//{
	//	GraphicsManager::LoadOBJ(&GraphicsStorage::objs, modelPath);
	//}
	//
	//__declspec(dllexport) void LoadAllModelsToGPU()
	//{
	//	GraphicsManager::LoadAllOBJsToVAOs(); //this should not be necessary to call from lua, it should be called every frame
	//}

	__declspec(dllexport) Texture* LoadTexture(const char* guid, const char* path)
	{
		return GraphicsManager::LoadTexture(guid, path);
	}

	__declspec(dllexport) Texture* LoadCubeMap(const char* guid, const char* path)
	{
		return GraphicsManager::LoadCubeMap(guid, path);
	}

#pragma endregion
#pragma region VertexArray
	__declspec(dllexport) VertexArray* VertexArray_new(const char* guid)
	{
		return GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<VertexArray>(guid);
	}

	__declspec(dllexport) void VertexArray_gc(VertexArray* self)
	{
		//delete self;
	}

	__declspec(dllexport) const char* VertexArray_GetName(VertexArray* self) {
		return self->name.c_str();
	}

	__declspec(dllexport) void VertexArray_SetName(VertexArray* self, const char* newName) {
		self->name = newName;
	}

	__declspec(dllexport) const char* VertexArray_GetPath(VertexArray* self) {
		return self->path.c_str();
	}

	__declspec(dllexport) void VertexArray_SetPath(VertexArray* self, const char* newPath) {
		self->path = newPath;
	}

	__declspec(dllexport) const char* VertexArray_GetConfigPath(VertexArray* self) {
		return self->configPath.c_str();
	}

	__declspec(dllexport) void VertexArray_SetConfigPath(VertexArray* self, const char* newPath) {
		self->configPath = newPath;
	}

	__declspec(dllexport) const char* VertexArray_GetMeshPath(VertexArray* self) {
		return self->meshPath.c_str();
	}

	__declspec(dllexport) void VertexArray_SetMeshPath(VertexArray* self, const char* newPath) {
		self->meshPath = newPath;
	}

	__declspec(dllexport) void VertexArray_SetPrimitiveMode(VertexArray* self, PrimitiveMode primitiveMode) {
		self->SetPrimitiveMode(primitiveMode);
	}

	__declspec(dllexport) const char* VertexArray_GetPrimitiveMode(VertexArray* self) {
		return self->GetPrimitiveModeAsStr();
	}

	__declspec(dllexport) void VertexArray_AddVertexBuffer(VertexArray* self, VertexBuffer* vbo) {
		self->AddVertexBuffer(vbo);
	}

	__declspec(dllexport) void VertexArray_AddElementBuffer(VertexArray* self, ElementBuffer* ebo) {
		self->AddElementBuffer(ebo);
	}

	__declspec(dllexport) VertexBuffer** VertexArray_GetVertexBuffers(VertexArray* self) {
		return self->vbos.data();
	}

	__declspec(dllexport) int VertexArray_GetVertexBuffersCount(VertexArray* self) {
		return self->vbos.size();
	}

	__declspec(dllexport) ElementBuffer* VertexArray_GetElementBuffer(VertexArray* self) {
		return self->ebo;
	}

	__declspec(dllexport) void VertexArray_ResizeVertexBuffer(VertexArray* self, VertexBuffer* bufferToResize, void* newData, unsigned int newElementCount) {
		self->ResizeVertexBuffer(bufferToResize, newData, newElementCount);
	}

	__declspec(dllexport) void VertexArray_ResizeElementBuffer(VertexArray* self, ElementBuffer* bufferToResize, void* newData, unsigned int newElementCount) {
		self->ResizeElementBuffer(bufferToResize, newData, newElementCount);
	}

	__declspec(dllexport) void VertexArray_SetDimensions(VertexArray* self, Vector3F& dimensions) {
		self->dimensions = (glm::vec3&)dimensions;
	}

	__declspec(dllexport) void VertexArray_SetCenter(VertexArray* self, Vector3F& center) {
		self->center = (glm::vec3&)center;
	}

	__declspec(dllexport) void VertexArray_GetDimensions(VertexArray* self, Vector3F& dimensions) {
		dimensions = (Vector3F&)self->dimensions;
	}

	__declspec(dllexport) void VertexArray_GetCenter(VertexArray* self, Vector3F& center) {
		center = (Vector3F&)self->center;
	}
#pragma endregion
#pragma region VertexBuffer
	__declspec(dllexport) VertexBuffer* VertexBuffer_new(const char* guid, const void* data, unsigned int elementCount, const BufferLayout& layout)
	{
		if (layout.isDynamic || data == nullptr)
		{
			return GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<VertexBufferDynamic>(guid, data, elementCount, layout);
		}
		else
		{
			return GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<VertexBuffer>(guid, data, elementCount, layout);
		}
	}

	__declspec(dllexport) void VertexBuffer_gc(VertexArray* self)
	{
		//delete self;
	}

	__declspec(dllexport) BufferLayout* VertexBuffer_GetBufferLayout(VertexBuffer* self) {
		return &self->layout;
	}

	__declspec(dllexport) bool VertexBuffer_IsDynamic(VertexBuffer* self) {
		return self->layout.isDynamic;
	}

	__declspec(dllexport) const char* VertexBuffer_GetName(VertexBuffer* self) {
		return self->name.c_str();
	}

	__declspec(dllexport) void VertexBuffer_SetName(VertexBuffer* self, const char* newName) {
		self->name = newName;
	}

	__declspec(dllexport) void VertexBuffer_Resize(VertexBuffer* self, void* newData, unsigned int newElementCount) {
		self->Resize(newData, newElementCount);
	}

	/*
	__declspec(dllexport) const char* VertexBuffer_GetPath(VertexBuffer* self) {
		return self->path.c_str();
	}

	__declspec(dllexport) void VertexBuffer_SetPath(VertexBuffer* self, const char* newPath) {
		self->path = newPath;
	}

	__declspec(dllexport) const char* VertexBuffer_GetDataName(VertexBuffer* self) {
		return self->dataName.c_str();
	}

	__declspec(dllexport) const char* VertexBuffer_SetDataName(VertexBuffer* self, const char* newPath) {
		return self->dataName.c_str();
	}
	*/
#pragma endregion
#pragma region BufferLayout
	__declspec(dllexport) BufferLayout* BufferLayout_new() {
		return GraphicsStorage::assetRegistry.AllocAsset<BufferLayout>();
	}

	__declspec(dllexport) void BufferLayout_gc(BufferLayout* self) {
		//delete self;
	}

	__declspec(dllexport) void BufferLayout_AddLocationLayout(BufferLayout* self, LocationLayout& locationLayout) {
		self->AddLocationLayout(locationLayout);
	}

	__declspec(dllexport) int BufferLayout_GetLocationsCount(BufferLayout* self) {
		return self->locations.size();
	}

	__declspec(dllexport) LocationLayout* BufferLayout_GetLocation(BufferLayout* self, int index) {
		return &self->locations[index];
	}

	__declspec(dllexport) bool BufferLayout_IsDynamic(BufferLayout* self) {
		return self->isDynamic;
	}
	/*
	__declspec(dllexport) const char* BufferLayout_GetPath(BufferLayout* self) {
		return self->path.c_str();
	}

	__declspec(dllexport) void BufferLayout_SetPath(BufferLayout* self, const char* newPath) {
		self->path = newPath;
	}
	*/
#pragma endregion
#pragma region BufferLayoutDefinitions
	__declspec(dllexport) void AddBufferLayoutToDefinition(const char* definitionName, BufferLayout& bufferLayout) {
		GraphicsStorage::bufferDefinitions[definitionName].push_back(bufferLayout);
	}
#pragma endregion
#pragma region LocationLayout
	__declspec(dllexport) LocationLayout* LocationLayout_new(ShaderDataType::Type type, const char* name, unsigned int instancesPerAttribute, bool normalized) {
		return GraphicsStorage::assetRegistry.AllocAsset<LocationLayout>(type, name, instancesPerAttribute, normalized);
	}

	__declspec(dllexport) void LocationLayout_gc(LocationLayout* self) {
		//delete self;
	}

	__declspec(dllexport) int LocationLayout_GetInstancesPerAttribute(LocationLayout* self) {
		return self->instancesPerAttribute;
	}

	__declspec(dllexport) bool LocationLayout_GetNormalized(LocationLayout* self) {
		return self->normalized;
	}

	__declspec(dllexport) const char* LocationLayout_GetType(LocationLayout* self) {
		return ShaderDataType::Str(self->type).data();
	}

	__declspec(dllexport) const char* LocationLayout_GetName(LocationLayout* self) {
		return self->name.c_str();
	}
#pragma endregion
#pragma region ElementBuffer
	__declspec(dllexport) ElementBuffer* ElementBuffer_new(const char* guid, const void* data, unsigned int count) {
		return GraphicsStorage::assetRegistry.AllocAssetWithStrUUID<ElementBuffer>(guid, data, count);
	}

	__declspec(dllexport) void ElementBuffer_gc(ElementBuffer* self) {
		//delete self;
	}

	__declspec(dllexport) void ElementBuffer_Resize(ElementBuffer* self, void* newData, unsigned int newElementCount) {
		self->Resize(newData, newElementCount);
	}

	__declspec(dllexport) unsigned int ElementBuffer_GetIndicesCount(ElementBuffer* self) {
		return self->indicesCount;
	}
#pragma endregion
#pragma region OBJ
	__declspec(dllexport) OBJ* OBJ_new(const char* name) {
		return nullptr;
	}

	__declspec(dllexport) void OBJ_gc(OBJ* self) {
		//delete self;
	}

	__declspec(dllexport) const char* OBJ_GetName(OBJ* self) {
		return self->name.c_str();
	}

	__declspec(dllexport) const char* OBJ_GetPath(OBJ* self) {
		return self->path.c_str();
	}

	__declspec(dllexport) int OBJ_GetPositionCount(OBJ* self) {
		return self->indexed_vertices.size();
	}

	__declspec(dllexport) int OBJ_GetUVCount(OBJ* self) {
		return self->indexed_uvs.size();
	}
	
	__declspec(dllexport) int OBJ_GetNormalCount(OBJ* self) {
		return self->indexed_normals.size();
	}

	__declspec(dllexport) int OBJ_GetTangentCount(OBJ* self) {
		return self->indexed_tangents.size();
	}

	__declspec(dllexport) int OBJ_GetBitangentCount(OBJ* self) {
		return self->indexed_bitangents.size();
	}

	__declspec(dllexport) int OBJ_GetIndicesCount(OBJ* self) {
		return self->indicesCount;
	}

	__declspec(dllexport) Vector3F* OBJ_GetPositionData(OBJ* self) {
		return (Vector3F*)self->indexed_vertices.data();
	}

	__declspec(dllexport) Vector2F* OBJ_GetUVData(OBJ* self) {
		return (Vector2F*)self->indexed_uvs.data();
	}

	__declspec(dllexport) Vector3F* OBJ_GetNormalData(OBJ* self) {
		return (Vector3F*)self->indexed_normals.data();
	}

	__declspec(dllexport) Vector3F* OBJ_GetTangentData(OBJ* self) {
		return (Vector3F*)self->indexed_tangents.data();
	}

	__declspec(dllexport) Vector3F* OBJ_GetBitangentData(OBJ* self) {
		return (Vector3F*)self->indexed_bitangents.data();
	}

	__declspec(dllexport) void* OBJ_GetIndicesData(OBJ* self) {

		if (self->indicesCount <= UCHAR_MAX)
		{
			return self->indicesUB.data();
		}
		else if (self->indicesCount <= USHRT_MAX)
		{
			return self->indicesUS.data();
		}
		return self->indices.data();
	}

	__declspec(dllexport) void OBJ_GetDimensions(OBJ* self, Vector3F& dimensions) {
		dimensions = (Vector3F&)self->dimensions;
	}

	__declspec(dllexport) void OBJ_GetCenter(OBJ* self, Vector3F& center) {
		center = (Vector3F&)self->center_of_mesh;
	}
#pragma endregion
#pragma region SimpleString
	_declspec(dllexport) void SimpleString_gc(SimpleString* self)
	{
		delete[] self->text;
		delete self;
	}
#pragma endregion

#pragma region FileSystem
	_declspec(dllexport) bool FileSystem_Exists(const char* path)
	{
		return std::filesystem::exists(path);
	}

	_declspec(dllexport) bool FileSystem_CreateDirectory(const char* path)
	{
		return std::filesystem::create_directory(path);
	}

	_declspec(dllexport) bool FileSystem_CreateDirectories(const char* path)
	{
		return std::filesystem::create_directories(path);
	}

	_declspec(dllexport) bool FileSystem_IsDirectory(const char* path)
	{
		return std::filesystem::is_directory(path);
	}

	_declspec(dllexport) bool FileSystem_IsRegularFile(const char* path)
	{
		return std::filesystem::is_regular_file(path);
	}

	_declspec(dllexport) SimpleString* FileSystem_ParentPath(const char* path)
	{
		std::filesystem::path fspath(path);
		std::string parent = fspath.parent_path().string();
		SimpleString* sPath = new SimpleString();
		char* text = new char[parent.length()];
		memcpy(text, parent.c_str(), parent.length());
		sPath->text = text;
		sPath->length = parent.length();
		return sPath;
	}


	_declspec(dllexport) SimpleString* FileSystem_GetCurrentPath()
	{
		std::string currentPath = std::filesystem::current_path().string();
		SimpleString* sPath = new SimpleString();
		char* path = new char[currentPath.length()];
		memcpy(path, currentPath.c_str(), currentPath.length());
		sPath->text = path;
		sPath->length = currentPath.length();
		return sPath;
	}

	_declspec(dllexport) void FileSystem_SetCurrentPath(const char* path)
	{
		std::filesystem::current_path(path);
	}

	_declspec(dllexport) SimpleString* FileSystem_DirectoryIteratorNext(DirectoryIterator* it)
	{
		auto current = (std::filesystem::directory_iterator*)(it);
		if ((*current) != std::filesystem::directory_iterator())
		{
			std::string currentPath = (*current)->path().string();
			SimpleString* sPath = new SimpleString();
			char* path = new char[currentPath.length()];
			memcpy(path, currentPath.c_str(), currentPath.length());
			sPath->text = path;
			sPath->length = currentPath.length();
			++(*current);
			return sPath;
		}
		else
		{
			return nullptr;
		}
	}

	_declspec(dllexport) DirectoryIterator* FileSystem_DirectoryIterator(const char* path)
	{
		return (DirectoryIterator*) (new std::filesystem::directory_iterator(path));
	}

	_declspec(dllexport) void FileSystem_DirectoryIterator_gc(DirectoryIterator* it)
	{
		delete (std::filesystem::directory_iterator*)it;
	}
#pragma endregion
}