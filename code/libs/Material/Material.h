#pragma once
#include <array>
#include <string>
#include <vector>

class RenderProfile;
class TextureProfile;
class MaterialProfile;
class VertexArray;
class Shader;
class RenderElement;
class ObjectProfile;
class RenderPass;
class DataRegistry;
class ShaderBlock;

enum class MaterialElements
{
	ERenderPass,
	EShader,
	ERenderProfile,
	ETextureProfile,
	EMaterialProfile,
	EVao,
	EObjectProfile
};

class Material
{
public:
	Material();
	~Material();
	union
	{
		std::array<RenderElement*,7> elements;
		struct
		{
			RenderPass* rps;
			Shader* shader;
			RenderProfile* rp;
			TextureProfile* tp;
			MaterialProfile* mp;
			VertexArray* vao;
			ObjectProfile* op;
		};
	};
	bool unbound = false;
	std::string name;
	std::string path;
	void AssignElement(RenderElement* ele, MaterialElements type);
	void AssignShader(Shader* ns);
	void AssignRenderProfile(RenderProfile* nrp);
	void AssignTextureProfile(TextureProfile* ntp);
	void AssignMaterialProfile(MaterialProfile* nmp);
	void AssignMesh(VertexArray* nm);
	void AssignObjectProfile(ObjectProfile* nm);
	void AssignRenderPass(RenderPass* nrps);
private:
	void UpdateProfileShaderBlocks(ObjectProfile* objectProfile, MaterialElements elementToCheck, Shader* previousShader, Shader* newShader, std::vector<ShaderBlock*>& shaderBlocks);
	void UpdateShaderRenderTargets();
};