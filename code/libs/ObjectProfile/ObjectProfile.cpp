#include "ObjectProfile.h"
#include "ShaderBlock.h"
#include "CPUBlockData.h"
#include "ShaderBlockData.h"
#include "Vbo.h"
#include "DataRegistry.h"

ObjectProfile::ObjectProfile()
{
}

ObjectProfile::~ObjectProfile()
{
}

void ObjectProfile::SetUp()
{
}

void ObjectProfile::Update()
{
}

void ObjectProfile::UpdateProfileFromDataRegistry(const DataRegistry& dataRegistry)
{
	for (auto& sbd : shaderBlockDatas)
	{
		sbd.RegisterProperties(dataRegistry);
	}
}

void ObjectProfile::SetDataRegistry(DataRegistry* dataRegistry)
{
	registryPtr = dataRegistry;
}

void ObjectProfile::AddShaderBlock(ShaderBlock * sb)
{
	for (int i = 0; i < shaderBlockDatas.size(); i++)
	{
		if (shaderBlockDatas[i].shaderBlock == sb)
		{
			return;
		}
	}
	shaderBlockDatas.emplace_back(sb);
}

void ObjectProfile::AddShaderBlocks(std::vector<ShaderBlock*>& sbs)
{
	for (auto sb : sbs)
	{
		AddShaderBlock(sb);
	}
}

void ObjectProfile::RemoveShaderBlock(ShaderBlock * sb)
{
	for (size_t i = 0; i < shaderBlockDatas.size(); i++)
	{
		if (shaderBlockDatas[i].shaderBlock == sb)
		{
			shaderBlockDatas.erase(shaderBlockDatas.begin() + i);
			return;
		}
	}
}

void ObjectProfile::RemoveShaderBlocks(std::vector<ShaderBlock*>& sbs)
{
	for (auto sb : sbs)
	{
		RemoveShaderBlock(sb);
	}
}

void ObjectProfile::AddVBO(VertexBufferDynamic* vbo)
{
	for (int i = 0; i < vbos.size(); i++)
	{
		if (vbos[i] == vbo)
		{
			return;
		}
	}
	vbos.push_back(vbo);
}

void ObjectProfile::AddVBOs(std::vector<VertexBufferDynamic*>& vbosIn)
{
	for (auto dvbo : vbosIn)
	{
		AddVBO(dvbo);
	}
}

void ObjectProfile::RemoveVBO(VertexBufferDynamic* vbo)
{
	for (size_t i = 0; i < vbos.size(); i++)
	{
		if (vbos[i] == vbo)
		{
			vbos.erase(vbos.begin() + i);
			return;
		}
	}
}

void ObjectProfile::RemoveVBOs(std::vector<VertexBufferDynamic*>& vbosIn)
{
	for (auto dvbo : vbosIn)
	{
		RemoveVBO(dvbo);
	}
}

void ObjectProfile::UpdateVBOCPUData()
{
	for (auto vbod : vbos)
	{
		//vbod->activeCount = 0;
		auto& locations = vbod->layout.GetLocations();
		for (auto& dr : registries)
		{
			if (dr->bindings.size() == 0)
			{
				continue;
			}
			bool elementSet = false;
			for (auto& loc : locations)
			{
				auto property = dr->GetProperty(loc.name.c_str());
				if (property != nullptr)
				{
					vbod->SetLocationData(property->dataAddress, loc);
					elementSet = true;
				}
			}
			if (elementSet)
			{
				vbod->IncreaseInstanceCount();
			}
		}
		vbod->Update();
	}
	registries.clear();
}

void ObjectProfile::UpdateShaderBlockCPUData()
{
	for (auto& sbd : shaderBlockDatas)
	{
		for (auto& nameAndOffset : sbd.shaderBlock->offsets)
		{
			auto property = registryPtr->GetProperty(nameAndOffset.first.c_str());
			if (property != nullptr)
			{
				sbd.shaderBlock->data.SetData(nameAndOffset.second, property->dataAddress, property->size);
			}
		}
		sbd.Submit();
	}
}

void ObjectProfile::SetAndSendData()
{
	for (auto& sbd : shaderBlockDatas)
	{
		sbd.UpdateAndSubmit();
	}
}

void ObjectProfile::SendData()
{
	for (auto& sbd : shaderBlockDatas)
	{
		sbd.Submit();
	}
}

void ObjectProfile::LoadLuaFile(const char * filename)
{
}

void ObjectProfile::Execute()
{
	UpdateShaderBlockCPUData();
	UpdateVBOCPUData();
}
