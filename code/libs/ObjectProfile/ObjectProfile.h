#pragma once
#include <vector>
#include <unordered_map>
#include "RenderElement.h"
#include "ShaderBlockData.h"
#include "DataRegistry.h"

class ShaderBlock;
class VertexBuffer;
class VertexBufferDynamic;
class DataRegistry;

class ObjectProfile : public RenderElement
{
public:
	ObjectProfile();
	~ObjectProfile();
	
	virtual void SetUp();
	virtual void Update();
	void SetAndSendData();
	void SendData();
	void UpdateProfileFromDataRegistry(const DataRegistry& dataRegistry);
	void SetDataRegistry(DataRegistry* dataRegistry);
	void AddShaderBlock(ShaderBlock* ub);
	void AddShaderBlocks(std::vector<ShaderBlock*>& ubs);
	void RemoveShaderBlock(ShaderBlock* ub);
	void RemoveShaderBlocks(std::vector<ShaderBlock*>& ubs);
	void AddVBO(VertexBufferDynamic* vbo);
	void AddVBOs(std::vector<VertexBufferDynamic*>& vbos);
	void RemoveVBO(VertexBufferDynamic* vbo);
	void RemoveVBOs(std::vector<VertexBufferDynamic*>& vbos);
	void UpdateVBOCPUData();
	void UpdateShaderBlockCPUData();
	std::vector<ShaderBlockData> shaderBlockDatas;
	std::vector<VertexBufferDynamic*> vbos;
	virtual void LoadLuaFile(const char * filename);
	virtual void Execute();
	DataRegistry* registryPtr = nullptr;
	std::vector<DataRegistry*> registries;
private:
};