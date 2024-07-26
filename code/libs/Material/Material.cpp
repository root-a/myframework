#include "Material.h"
#include "MaterialProfile.h"
#include "Shader.h"
#include "GraphicsStorage.h"
#include "ShaderBlock.h"
#include "ShaderBlockData.h"
#include "ObjectProfile.h"
#include "RenderPass.h"
#include "FrameBuffer.h"
#include "Texture.h"

Material::Material()
{
	rps = nullptr;
	shader = nullptr;
	rp = nullptr;
	tp = nullptr;
	mp = nullptr;
	vao = nullptr;
	op = nullptr;
}

Material::~Material()
{
}

void Material::AssignElement(RenderElement* ele, MaterialElements type)
{
	switch (type)
	{
	case MaterialElements::ERenderPass:
		AssignRenderPass((RenderPass*)ele);
		break;
	case MaterialElements::EShader:
		AssignShader((Shader*)ele);
		break;
	case MaterialElements::ERenderProfile:
		AssignRenderProfile((RenderProfile*)ele);
		break;
	case MaterialElements::ETextureProfile:
		AssignTextureProfile((TextureProfile*)ele);
		break;
	case MaterialElements::EMaterialProfile:
		AssignMaterialProfile((MaterialProfile*)ele);
		break;
	case MaterialElements::EVao:
		AssignMesh((VertexArray*)ele);
		break;
	case MaterialElements::EObjectProfile:
		AssignObjectProfile((ObjectProfile*)ele);
		break;
	default:
		break;
	}
}

void Material::AssignShader(Shader * ns)
{
	Shader* previousShader = shader;
	shader = ns;

	if (shader != nullptr)
	{
		if (mp != nullptr)
		{
			UpdateProfileShaderBlocks(mp, MaterialElements::EMaterialProfile, previousShader, shader, shader->materialUniformBuffers);
			mp->UpdateProfileFromDataRegistry(mp->registry);
		}
		if (op != nullptr)
		{			
			UpdateProfileShaderBlocks(op, MaterialElements::EObjectProfile, previousShader, shader, shader->objectUniformBuffers);
		}
		if (rps != nullptr)
		{
			UpdateProfileShaderBlocks(rps, MaterialElements::ERenderPass, previousShader, shader, shader->globalUniformBuffers);
			rps->UpdateProfileFromDataRegistry(rps->registry);
			UpdateShaderRenderTargets();
		}
	}
}

void Material::AssignRenderProfile(RenderProfile * nrp)
{
	rp = nrp;
}

void Material::AssignTextureProfile(TextureProfile * ntp)
{
	tp = ntp;
}

void Material::AssignMaterialProfile(MaterialProfile * nmp)
{
	mp = nmp;
	if (shader != nullptr)
	{
		if (mp != nullptr)
		{
			mp->AddShaderBlocks(shader->materialUniformBuffers);
			mp->UpdateProfileFromDataRegistry(mp->registry);
		}
	}
}

void Material::AssignMesh(VertexArray* nm)
{
	vao = nm;
	if (vao != nullptr)
	{
		if (op != nullptr)
		{
			//might be worth clearing before adding
			op->AddVBOs(vao->dynamicVBOs);
		}
	}
}

void Material::AssignObjectProfile(ObjectProfile * nop)
{
	op = nop;
	if (op != nullptr)
	{
		if (shader != nullptr)
		{
			op->AddShaderBlocks(shader->objectUniformBuffers);
		}
		if (vao != nullptr)
		{
			//might be worth clearing before adding
			op->AddVBOs(vao->dynamicVBOs);
			//op->UpdateProfileFromDataRegistry(op->registry);
		}
	}
}

void Material::AssignRenderPass(RenderPass * nrps)
{
	rps = nrps;
	if (shader != nullptr)
	{
		if (rps != nullptr)
		{
			UpdateShaderRenderTargets();
			rps->AddShaderBlocks(shader->globalUniformBuffers);
			rps->UpdateProfileFromDataRegistry(rps->registry);
		}
	}
}

void Material::UpdateProfileShaderBlocks(ObjectProfile* objectProfile, MaterialElements elementToCheck, Shader* previousShader, Shader* newShader, std::vector<ShaderBlock*>& shaderBlocks)
{
	//when changing shader we want to remove shader blocks that are not used by new shader
	//shader blocks are not deleted
	//only pointers and those are pointers to shaderblockdatas
	//same material profile can be used by another object
	//that object can use different shader
	//that object shader can only use few of same uniform buffers
	//and we want to remove all uniform buffers from the profile that are not used by either objects
	std::vector<ShaderBlock*> remainingNotFoundBuffers;
	for (auto& ubd : objectProfile->shaderBlockDatas)
	{
		remainingNotFoundBuffers.push_back(ubd.shaderBlock);
	}

	int found = false;
	for (auto& mat : *GraphicsStorage::assetRegistry.GetPool<Material>()) //we go over all materials
	{
		if (&mat != this) //we look at all other materials except for this one
		{
			if (mat.elements[(int)elementToCheck] == objectProfile) //we look at object profile or material profile or renderpass
			{
				if (mat.shader == previousShader) //we stop all search if material with same objectProfile/materialProfile/renderPass has same shader
				{
					found = true; //we found another material with same shader and profile, this might make sense for material and pass but not really for objectprofile, all profiles are unique
					break;
				}
				else if (mat.shader != nullptr) //as long shader is not null
				{
					for (auto& ubd : objectProfile->shaderBlockDatas) //we look att profile shader blocks
					{
						bool result = mat.shader->HasUniformBuffer(ubd.shaderBlock->index); //we check if shader of searched material has same uniform buffer as object profile
						if (result)
						{
							int remainingUBIndex = -1;
							for (size_t i = 0; i < remainingNotFoundBuffers.size(); i++)
							{
								if (remainingNotFoundBuffers[i]->index == ubd.shaderBlock->index) //we look for index in remainingNotFoundBuffers
								{
									remainingUBIndex = i;
									break;
								}
							}
							if (remainingUBIndex != -1)
							{
								//we remove buffers that we have found used in other materials
								//this is because same material profile can be used in another material
								//we do this for different shaders that use same uniform buffers but not always all are same
								//it might use only one or two same uniform buffers and those we want keep others that are not shared by other objects we can remove
								//it might be a good idea to rethink this, it's ok to use same uniform buffer no problemo
								//the idea is to submit data only once for assets with same values
								//if they share same profile the profile will be executed only once
								//maybe in the future we can think of something
								remainingNotFoundBuffers.erase(remainingNotFoundBuffers.begin() + remainingUBIndex);
							}
							if (remainingNotFoundBuffers.size() == 0)
							{
								found = true;
								break;
							}
						}
					}
				}
			}
		}
	}
	if (!found)
	{
		objectProfile->RemoveShaderBlocks(remainingNotFoundBuffers);
	}

	objectProfile->AddShaderBlocks(shaderBlocks);
	//objectProfile->UpdateProfileFromDataRegistry(objectProfile->registry);
}

void Material::UpdateShaderRenderTargets()
{
	if (shader != nullptr && rps != nullptr && rps->fbo != nullptr)
	{
		shader->renderTargets.clear();
		shader->renderTargets.resize(shader->outputs.size());
		for (size_t i = 0; i < rps->fbo->textures.size() && i < shader->outputs.size(); i++)
		{
			shader->renderTargets[i] = rps->fbo->textures[i]->attachment;
		}
	}
}
