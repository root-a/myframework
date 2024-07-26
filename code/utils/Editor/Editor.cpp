#include "Editor.h"
#include "DataRegistry.h"
#include "Object.h"
#include "Texture.h"
#include "RenderBuffer.h"
#include "ImGuiWrapper.h"
#include <imgui.h>
#include "Material.h"
#include "Script.h"
#include "RenderPass.h"
#include "ShaderBlock.h"
#include "ShaderBlockData.h"
#include "SceneGraph.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "RigidBody.h"
#include "Bounds.h"
#include "GraphicsStorage.h"
#include "GraphicsManager.h"
#include "Vao.h"
#include "Ebo.h"
#include "MaterialProfile.h"
#include "TextureProfile.h"
#include "RenderProfile.h"
#include "RenderPass.h"
#include "FrameBuffer.h"
#include "PointSystem.h"
#include "LineSystem.h"
#include "BoundingBoxSystem.h"
#include "Times.h"
#include "Render.h"
#include "PhysicsManager.h"
#include "DebugDraw.h"
#include "CameraManager.h"
#include "Camera.h"
#include <GLFW/glfw3.h>
#include "FBOManager.h"
#include <algorithm>
#include "misc/cpp/imgui_stdlib.h"
#include "OBJ.h"
#include "ParticleSystem.h"
#include "ScriptsComponent.h"
#include <thread>
#include <filesystem>
#include <set>
#include <mutex>
#include <format>
#include <string_view>
#include "ShaderDataType.h"
#include "ImGuizmo.h"
#include <glm/glm.hpp>
#include "imgui.h"

static std::mutex meshLoadMutex;
static std::mutex createVaoMutex;

extern "C" {
//#include "include/lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "luajit.h"
//#include "include/luaconf.h"
}
#include "LuaTools.h"

Editor::Editor()
{
	InitializeGLC();
	for (auto& shader : *GraphicsStorage::assetRegistry.GetPool<Shader>())
	{
		if (shader.name.compare("SoftParticle") == 0)
		{
			particlesShader = &shader;
			break;
		}
	}
	for (auto& tex : *GraphicsStorage::assetRegistry.GetPool<Texture>())
	{
		if (tex.texturePath.empty())
		{
			renderTargets.emplace(tex.name, &tex);
		}
		else
		{
			if (tex.target == GL_TEXTURE_CUBE_MAP)
			{
				cubemaps.emplace(tex.name, &tex);
			}
			else
			{
				textures.emplace(tex.name, &tex);
			}
		}
	}
	for (auto& rb : *GraphicsStorage::assetRegistry.GetPool<RenderBuffer>())
	{
		renderBuffers.emplace(rb.name, &rb);
	}

	keyToggles.emplace(ImGuiKey_LeftAlt, KeyToggle(ImGuiKey_LeftAlt));

}

Editor::~Editor()
{
}

static void HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

template<typename UIPropertiesFunction, typename UIMenuItemsFunction>
inline void Editor::DrawComponentBasic(Component* component, const std::string& name, bool dynamic, Object* object, const UIPropertiesFunction& uiFunction, const UIMenuItemsFunction& menuItems)
{
	if (component != nullptr)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		ImVec2 contentRegionAvaliable = ImGui::GetContentRegionAvail();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
		float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
		ImGui::Separator();
		bool open = ImGui::TreeNodeEx(name.c_str(), treeNodeFlags, name.c_str());
		ImGui::PopStyleVar();
		ImGui::SameLine(contentRegionAvaliable.x - lineHeight * 1.0f);
		if (ImGui::Button("...", ImVec2{ lineHeight, lineHeight }))
		{
			ImGui::OpenPopup("ComponentSettings");
		}

		bool removeComponent = false;
		if (ImGui::BeginPopup("ComponentSettings"))
		{
			if (ImGui::MenuItem("Remove Component"))
			{
				removeComponent = true;
			}
			if (dynamic)
			{
				if (ImGui::MenuItem("Make Static"))
				{
					object->SetComponentDynamicState(component, false);
				}
			}
			else
			{
				if (ImGui::MenuItem("Make Dynamic"))
				{
					object->SetComponentDynamicState(component, true);
				}
			}
			menuItems(component);
			ImGui::EndPopup();
		}

		if (open)
		{
			uiFunction(component);
			ImGui::TreePop();
		}

		if (removeComponent)
		{
			object->RemoveComponent(component);
		}
	}
}

bool Editor::CreateNewAssetUI(char * nameBuffer, bool copy)
{
	ImGui::PushItemWidth(-1);
	float calculatedWidth = ImGui::CalcItemWidth();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Name:"); ImGui::SameLine();
	ImGui::PushItemWidth(ImGui::GetFontSize() * -7.f);
	ImGui::InputText("##edit", nameBuffer, 128);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	bool createNew = false;
	if (ImGui::Button(copy ? "Create Copy" : "Create New", ImVec2(-1, 0.f)))
	{
		if (strcmp(nameBuffer, "") != 0)
		{
			createNew = true;
		}
	}
	ImGui::Separator();
	return createNew;
}

Object* Editor::CreateNewObject(char* nameBuffer)
{
	Object* asset = GraphicsStorage::assetRegistry.AllocAsset<Object>();
	asset->name = nameBuffer;
	SceneGraph::Instance()->addObject(asset);
	SceneGraph::Instance()->pickingList[asset->ID] = asset;
	return asset;
}

//template should be one of existing json templates
//atm it's hardcored pass, shader, etc
Object* Editor::CreateDefaultObject(char * name)
{
	Object* newObject = GraphicsStorage::assetRegistry.AllocAsset<Object>();
	SceneGraph::Instance()->addObject(newObject);
	newObject->name = name;
	SceneGraph::Instance()->pickingList[newObject->ID] = newObject;

	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	std::string scriptPath = "resources/objects/properties/default_mesh.lua";
	LuaTools::dofile(L, scriptPath.c_str());
	lua_getglobal(L, "Register");
	if (lua_isfunction(L, -1))
	{
		lua_pushlightuserdata(L, newObject);
		lua_pushstring(L, scriptPath.c_str());
		int result = LuaTools::report(L, LuaTools::docall(L, 2, 1));
		lua_pop(L, 1);
	}
	lua_close(L);

	Material* newMaterial = GraphicsStorage::assetRegistry.AllocAsset<Material>();
	newMaterial->name = name;

	ObjectProfile* op = GraphicsStorage::assetRegistry.AllocAsset<ObjectProfile>();
	op->name = name;
	newMaterial->AssignObjectProfile(op);

	//we need something like default assets
	for (auto& asset : *GraphicsStorage::assetRegistry.GetPool<RenderPass>())
	{
		if (asset.name.compare(name) == 0)
		{
			newMaterial->AssignRenderPass(&asset);
			break;
		}
	}
	for (auto& asset : *GraphicsStorage::assetRegistry.GetPool<Shader>())
	{
		if (asset.name.compare("geometry") == 0)
		{
			newMaterial->AssignShader(&asset);
			break;
		}
	}
	for (auto& asset : *GraphicsStorage::assetRegistry.GetPool<TextureProfile>())
	{
		if (asset.name.compare("default") == 0)
		{
			newMaterial->AssignTextureProfile(&asset);
			break;
		}
	}
	for (auto& asset : *GraphicsStorage::assetRegistry.GetPool<VertexArray>())
	{
		if (asset.name.compare(name) == 0)
		{
			newMaterial->AssignMesh(&asset);
			break;
		}
	}
	
	newObject->AssignMaterial(newMaterial);
	
	lastPickedObject = newObject;

	return newObject;
}

//TODO would be nice to copy entire hierarchy
//currently duplicated loose the link to template (no path)
//maybe we should add button to duplicate with path
//we have clone functions in the components
//we could use that to loop over the components and just clone
//but the sad part is that the the components then might need dependency injection to the memory pool allocator
//it might be good if I can just do this with dependency injection and simple interface
//i might want to turn all systems into one, maybe fast instance system is all we need
//probably it would be best to create ui for these systems especially fast instance system
//i have to add ui for bounds
//logic for toggling passes, maybe funcitonality of render pass?
//ohh but we do have render queue we can add and remove passes from it
Object* Editor::DuplicateObject(Object* object)
{
	Object* newObject = GraphicsStorage::assetRegistry.AllocAsset<Object>();
	SceneGraph::Instance()->addObject(newObject);
	newObject->name = object->name;
	//copy components
	for (auto comp : object->components)
	{
		bool found = false;
		for (auto dynComp : object->dynamicComponents)
		{
			if (comp.second == dynComp.second)
			{
				found = true;
				break;
			}
		}
		
		if (std::type_index(typeid(DirectionalLight*)) == comp.first)
		{
			DirectionalLight* newComp = GraphicsStorage::assetRegistry.AllocAsset<DirectionalLight>();
			newObject->AddComponent(newComp, found);
		}
		else if (std::type_index(typeid(PointLight*)) == comp.first)
		{
			PointLight* newComp = GraphicsStorage::assetRegistry.AllocAsset<PointLight>();
			newObject->AddComponent(newComp, found);
		}
		else if (std::type_index(typeid(SpotLight*)) == comp.first)
		{
			SpotLight* newComp = GraphicsStorage::assetRegistry.AllocAsset<SpotLight>();
			newObject->AddComponent(newComp, found);
		}
		else if (std::type_index(typeid(RigidBody*)) == comp.first)
		{
			RigidBody* newComp = GraphicsStorage::assetRegistry.AllocAsset<RigidBody>();
			newObject->AddComponent(newComp, found);
		}
		else if (std::type_index(typeid(Node*)) == comp.first)
		{
			Node* prevNode = object->GetComponent<Node>();
			Node* newComp = GraphicsStorage::assetRegistry.AllocAsset<Node>();
			*newComp = *prevNode;
			if (newComp->parent != nullptr)
			{
				newComp->parent->children.push_back(newComp);
			}
			newComp->children.clear();
			newObject->AddComponent(newComp, found);
			bool movable = prevNode->GetMovable();
			bool totalMovable = prevNode->GetTotalMovable();
			if (movable && prevNode->FindNearestMovableAncestor() == nullptr)
			{
				SceneGraph::Instance()->dirtyDynamicNodes.push_back(newComp);
			}
		}
		else if (std::type_index(typeid(Bounds*)) == comp.first)
		{
			Bounds* compCasted = (Bounds*)comp.second;
			Bounds* newComp = GraphicsStorage::assetRegistry.AllocAsset<Bounds>();
			*newComp = *compCasted;
			newObject->AddComponent(newComp, found);
		}
		else if (std::type_index(typeid(LineSystem*)) == comp.first)
		{
			LineSystem* compCasted = (LineSystem*)comp.second;
			LineSystem* newComp = GraphicsStorage::assetRegistry.AllocAsset<LineSystem>((int)compCasted->MaxCount);
			newObject->AddComponent(newComp, found);
		}
		else if (std::type_index(typeid(PointSystem*)) == comp.first)
		{
			PointSystem* compCasted = (PointSystem*)comp.second;
			PointSystem* newComp = GraphicsStorage::assetRegistry.AllocAsset<PointSystem>((int)compCasted->MaxCount);
			newObject->AddComponent(newComp, found);
		}
		else if (std::type_index(typeid(BoundingBoxSystem*)) == comp.first)
		{
			BoundingBoxSystem* compCasted = (BoundingBoxSystem*)comp.second;
			BoundingBoxSystem* newComp = GraphicsStorage::assetRegistry.AllocAsset<BoundingBoxSystem>((int)compCasted->MaxCount);
			newObject->AddComponent(newComp, found);
		}
		else if (std::type_index(typeid(ParticleSystem*)) == comp.first)
		{
			ParticleSystem* compCasted = (ParticleSystem*)comp.second;
			ParticleSystem* newComp = GraphicsStorage::assetRegistry.AllocAsset<ParticleSystem>((int)compCasted->MaxParticles, compCasted->EmissionRate);
			newObject->AddComponent(newComp, found);
		}
		else if (std::type_index(typeid(ScriptsComponent*)) == comp.first)
		{
			ScriptsComponent* compCasted = (ScriptsComponent*)comp.second;
			ScriptsComponent* newComp = GraphicsStorage::assetRegistry.AllocAsset<ScriptsComponent>();
			for (auto script : compCasted->scripts)
			{
				newComp->AddScript(script->path.c_str());
			}
			newObject->AddComponent(newComp, found);
		}
	}

	//copy properties
	//ObjectProfile* prevOp = object->GetComponent<ObjectProfile>();
	//DataRegistry* prevRegistry = prevOp != nullptr ? &prevOp->registry : nullptr;
	
	DataRegistry* prevRegistry = &object->registry;
	if (prevRegistry != nullptr)
	{
		//generate same properties via lua, consists of added properties (we could safely copy) and properties linked from the object itself, we can't copy that because then new object would have properties like model matrix pointing to the original object not the new one.
		if (GraphicsStorage::luaProperties.find(prevRegistry) != GraphicsStorage::luaProperties.end())
		{
			for (auto& scriptNameProperties : GraphicsStorage::luaProperties[prevRegistry])
			{
				lua_State* L = luaL_newstate();
				luaL_openlibs(L);
				LuaTools::dofile(L, scriptNameProperties.first.c_str());
				lua_getglobal(L, "Register");
				if (lua_isfunction(L, -1))
				{
					lua_pushlightuserdata(L, newObject);
					lua_pushstring(L, scriptNameProperties.first.c_str());
					int result = LuaTools::report(L, LuaTools::docall(L, 2, 1));
					lua_pop(L, 1);
				}
				lua_close(L);
			}
		}
		//copy object properties, are properties of other objects so it consists of pointers to other objects, safe to copy
		if (GraphicsStorage::objectProperties.find(prevRegistry) != GraphicsStorage::objectProperties.end())
		{
			GraphicsStorage::objectProperties[&newObject->registry] = GraphicsStorage::objectProperties[prevRegistry];
		}
	}

	//copy materials
	if (object->materials.size() > 0)
	{
		if (object->materials[0][0]->op->vbos.size() > 0)
		{
			newObject->materials = object->materials;
		}
		else
		{
			for (int i = 0; i < object->materials.size(); i++)
			{
				for (int j = 0; j < object->materials[i].size(); j++)
				{
					Material* newMat = GraphicsStorage::assetRegistry.AllocAsset<Material>();
					*newMat = *object->materials[i][j];
					newMat->path.clear();
					if (newMat->op != nullptr)
					{
						newObject->path.clear();
						ObjectProfile* op = GraphicsStorage::assetRegistry.AllocAsset<ObjectProfile>();
						op->name = newMat->op->name;
						newMat->AssignObjectProfile(op);
					}
					
					
					newObject->AssignMaterial(newMat, i, j);
				}
			}
		}
	}
	
	return newObject;
}

//TODO read json file that specifies the paths to serialization scripts
//because currently we hardcode paths to serialization scripts :(
Object* Editor::CreateObjectFromTemplate(char* path)
{
	static Script sceneLoaderScript("resources/serialization/scene_loader.lua");	
	if (sceneLoaderScript.GetFunction("LoadChild"))
	{
		lua_pushnil(sceneLoaderScript.L);
		lua_newtable(sceneLoaderScript.L);
		lua_pushstring(sceneLoaderScript.L, "path");
		lua_pushstring(sceneLoaderScript.L, path);
		lua_settable(sceneLoaderScript.L, -3);
		int result = LuaTools::report(sceneLoaderScript.L, LuaTools::docall(sceneLoaderScript.L, 2, 1));
		return nullptr;
	}
	return nullptr;
}

Material* Editor::CreateNewMaterial(char* nameBuffer)
{
	Material* m = GraphicsStorage::assetRegistry.AllocAsset<Material>();
	m->name = nameBuffer;
	return m;
}

void Editor::ImportNewMesh(char* nameBuffer)
{
	
}

void Editor::ImportNewTexture(char* nameBuffer)
{
	
}

bool Editor::CreateNewRenderTargetUI(char* nameBuffer)
{
	GLCCombo(targets, "Target", currentTarget);
	ImGui::PushItemWidth(ImGui::GetFontSize() * -20.f);
	ImGui::DragInt("Mip Level", &currentLevel);
	ImGui::PopItemWidth();
	GLCCombo(internalFormats, "Internal Format", currentInternalFormat);

	ImGui::PushItemWidth(ImGui::GetFontSize() * -20.f);
	ImGui::DragInt2("Width, Height", currentWidthHeight);
	ImGui::PopItemWidth();

	GLCCombo(formats, "Format", currentFormat);
	GLCCombo(types, "Type", currentType);
	GLCCombo(attachments, "Attachment", currentAttachment);

	ImGui::PushItemWidth(ImGui::GetFontSize() * -20.f);
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Name:");
	ImGui::SameLine();
	ImGui::InputText("##edit", nameBuffer, 128);

	ImGuiStyle& style = ImGui::GetStyle();
	float spacing = style.ItemInnerSpacing.x;
	ImGui::SameLine(0, spacing);
	bool createNew = false;
	if (ImGui::Button("Create New", ImVec2(-1, 0.f)))
	{
		if (strcmp(nameBuffer, "") != 0)
		{
			createNew = true;
		}
	}
	ImGui::Separator();
	ImGui::PopItemWidth();
	return createNew;
}

void Editor::CreateNewRenderTarget(char* nameBuffer)
{
	if (renderTargets.find(nameBuffer) == renderTargets.end())
	{
		Texture* texture = GraphicsStorage::assetRegistry.AllocAsset<Texture>(currentTarget, currentLevel, (int)currentInternalFormat, currentWidthHeight[0], currentWidthHeight[1], currentFormat, currentType, nullptr, currentAttachment);
		texture->GenerateBindSpecify();
		texture->name = nameBuffer;
		renderTargets[texture->name] = texture;
	}
}

bool Editor::CreateNewRenderBufferUI(char * nameBuffer)
{
	ImGui::PushItemWidth(ImGui::GetFontSize() * -20.f);
	GLCCombo(internalFormats, "Internal Format", currentInternalFormat);

	ImGui::PushItemWidth(ImGui::GetFontSize() * -20.f);
	ImGui::DragInt2("Width, Height", currentWidthHeight);
	ImGui::PopItemWidth();

	GLCCombo(attachments, "Attachment", currentAttachment);

	ImGui::PushItemWidth(ImGui::GetFontSize() * -20.f);
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Name:");
	ImGui::SameLine();
	ImGui::InputText("##edit", nameBuffer, 128);

	ImGuiStyle& style = ImGui::GetStyle();
	float spacing = style.ItemInnerSpacing.x;
	ImGui::SameLine(0, spacing);
	bool createNew = false;
	if (ImGui::Button("Create New", ImVec2(-1, 0.f)))
	{
		if (strcmp(nameBuffer, "") != 0)
		{
			createNew = true;
		}
	}
	ImGui::Separator();
	ImGui::PopItemWidth();
	return createNew;
}

RenderBuffer* Editor::CreateNewRenderBuffer(char * nameBuffer)
{
	RenderBuffer* rbuffer = GraphicsStorage::assetRegistry.AllocAsset<RenderBuffer>((int)currentInternalFormat, currentWidthHeight[0], currentWidthHeight[1], currentAttachment);
	rbuffer->GenerateBindSpecify();
	rbuffer->name = nameBuffer;
	return rbuffer;
}

FrameBuffer* Editor::CreateNewFrameBuffer(char* nameBuffer)
{
	FrameBuffer* frameBuffer = GraphicsStorage::assetRegistry.AllocAsset<FrameBuffer>((unsigned int)GL_FRAMEBUFFER);
	FBOManager::Instance()->AddFrameBuffer(frameBuffer, false);
	frameBuffer->name = nameBuffer;
	return frameBuffer;
}

RenderPass* Editor::CreateNewRenderPass(char* nameBuffer)
{
	RenderPass* pass = GraphicsStorage::assetRegistry.AllocAsset<RenderPass>();
	pass->name = nameBuffer;
	return pass;
}

MaterialProfile* Editor::CreateNewMaterialProfile(char* nameBuffer)
{
	MaterialProfile* profile = GraphicsStorage::assetRegistry.AllocAsset<MaterialProfile>();
	profile->name = nameBuffer;
	return profile;
}

TextureProfile* Editor::CreateNewTextureProfile(char* nameBuffer)
{
	TextureProfile* profile = GraphicsStorage::assetRegistry.AllocAsset<TextureProfile>();
	profile->name = nameBuffer;
	return profile;
}

RenderProfile* Editor::CreateNewRenderProfile(char* nameBuffer)
{
	RenderProfile* profile = GraphicsStorage::assetRegistry.AllocAsset<RenderProfile>();
	profile->name = nameBuffer;
	return profile;
}

ObjectProfile* Editor::CreateNewObjectProfile(char* nameBuffer)
{
	ObjectProfile* profile = GraphicsStorage::assetRegistry.AllocAsset<ObjectProfile>();
	profile->name = nameBuffer;
	return profile;
}

Shader* Editor::CreateNewShader(char * nameBuffer)
{
	ShaderPaths sp;
	std::string shaderPath = std::string("resources/shaders/") + std::string(nameBuffer);
	sp.vs = shaderPath + std::string(".vs");
	std::string vshader =
		"#version 420 core\n\n"
		"layout(location = 0) in vec3 vertexPos;\n\n"
		"void main()\n"
		"{\n"
		"	gl_Position = vec4(vertexPos,1);\n"
		"};\n";
	GraphicsManager::WriteStringToTextFile(vshader, sp.vs);
	//GraphicsManager::AddPairToJson(std::string(nameBuffer), shaderPath, std::string("resources/shaders.json"));
	return GraphicsManager::ReloadShaderFromPath(nameBuffer, sp);
}

void Editor::CreateNewScript(char * nameBuffer)
{
}

void Editor::ObjectEditor(Object* object)
{
	ImGui::Begin("Object Editor", &showObjectEditor, ImGuiWindowFlags_MenuBar);
	lastPickedObject = object;
	static char name[128];
	bool newSelection = false;
	if (CreateNewAssetUI(name))
	{
		lastPickedObject = CreateNewObject(name);
		newSelection = true;
	}
	static Object* selectedObject = nullptr;
	selectedObject = lastPickedObject;
	ObjectInspector(selectedObject, newSelection);
	ImGui::End();
}

std::string Editor::GetComponentName(const std::type_index& type)
{
	enum class ComponentTypes
	{
		DirectionalLight,
		PointLight,
		SpotLight,
		RigidBody,
		Node,
		Bounds,
		FastInstanceSystem,
		InstanceSystem,
		BoundingBoxSystem,
		LineSystem,
		PointSystem,
		ParticleSystem,
		ScriptsComponent,
		CircleSystem
	};
	static std::array<std::type_index, 14> types {
		std::type_index(typeid(DirectionalLight*)),
		std::type_index(typeid(PointLight*)),
		std::type_index(typeid(SpotLight*)),
		std::type_index(typeid(RigidBody*)),
		std::type_index(typeid(Node*)),
		std::type_index(typeid(Bounds*)),
		std::type_index(typeid(FastInstanceSystem*)),
		std::type_index(typeid(InstanceSystem*)),
		std::type_index(typeid(BoundingBoxSystem*)),
		std::type_index(typeid(LineSystem*)),
		std::type_index(typeid(PointSystem*)),
		std::type_index(typeid(ParticleSystem*)),
		std::type_index(typeid(ScriptsComponent*)),
		std::type_index(typeid(CircleSystem*))
	};
	if (types[0] == type)
	{
		return "DirectionalLight";
	}
	else if (types[1] == type)
	{
		return "PointLight";
	}
	else if (types[2] == type)
	{
		return "SpotLight";
	}
	else if (types[3] == type)
	{
		return "RigidBody";
	}
	else if (types[4] == type)
	{
		return "Transform";
	}
	else if (types[5] == type)
	{
		return "Bounds";
	}
	else if (types[6] == type)
	{
		return "FastInstanceSystem";
	}
	else if (types[7] == type)
	{
		return "InstanceSystem";
	}
	else if (types[8] == type)
	{
		return "BoundingBoxSystem";
	}
	else if (types[9] == type)
	{
		return "LineSystem";
	}
	else if (types[10] == type)
	{
		return "PointSystem";
	}
	else if (types[11] == type)
	{
		return "ParticleSystem";
	}
	else if (types[12] == type)
	{
		return "ScriptsComponent";
	}
	else if (types[13] == type)
	{
		return "CircleSystem";
	}
	else
	{
		return nullptr;
	}
}

//TODO
//when duplicating instances we have to make we update dynamic arrays in scenegraph
//we might be duplicating a node that is in the dynamic array, that's when we want to add that duplicate to that array
void Editor::ObjectInspector(Object* selectedObject, bool newSelection)
{
	static glm::vec3 transformRotation;
	std::unordered_map<std::string, std::string> newObjectButtonNames = { std::pair<std::string,std::string>("Object",""), std::pair<std::string,std::string>("Cube","cube"), std::pair<std::string,std::string>("Sphere","sphere"), std::pair<std::string,std::string>("Plane","plane") };
	{
		ImGui::Text("New");
		ImGui::PushID("new_template_buttons");

		float buttonWidth = 128;
		int buttons_count = 20000;
		float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
		float borderWidth = 2;
		float groupSize = buttonWidth + borderWidth;
		ImGui::PushItemWidth(groupSize);
		ImGuiStyle& style = ImGui::GetStyle();

		for (auto newObjectButtonName : newObjectButtonNames)
		{
			ImGui::BeginGroup();

			if (ImGui::Button(newObjectButtonName.first.c_str()))
			{
				Object* newObject = CreateDefaultObject(newObjectButtonName.second.data());
				if (newObject != nullptr)
				{
					selectedObject = newObject;
					newSelection = true;
				}
			}

			float last_button_x2 = ImGui::GetItemRectMax().x;
			float next_button_x2 = last_button_x2 + style.ItemSpacing.x + buttonWidth; // Expected position if next button was on same line
			ImGui::EndGroup();
			if (next_button_x2 < window_visible_x2)
				ImGui::SameLine();
		}
		ImGui::PopID();
		std::vector<std::string> templateNames;
		GraphicsManager::GetFileNames(templateNames, "resources/objects/", ".json");
		static std::string selectedTemplateName;
		if (ImGui::BeginCombo("Templates", selectedTemplateName.c_str()))
		{
			static ImGuiTextFilter filterTemplates;
			filterTemplates.Draw("##objectTemplateFilter");
			ImGui::BeginChild("template children", ImVec2(-1, 100), false);
			for (auto& templateName : templateNames)
			{
				if (filterTemplates.PassFilter(templateName.c_str()))
				{
					const bool is_selected = (selectedTemplateName == templateName);
					if (ImGui::Selectable(templateName.c_str(), is_selected))
					{
						selectedTemplateName = templateName;
						ImGui::CloseCurrentPopup();
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndChild();
			ImGui::EndCombo();
		}
		ImGui::SameLine();
		if (ImGui::Button("Create"))
		{
			if (!selectedTemplateName.empty())
			{
				std::string templatePath = "resources/objects/" + selectedTemplateName + ".json";
				Object* newObject = CreateObjectFromTemplate(templatePath.data());
				if (newObject != nullptr)
				{
					selectedObject = newObject;
					newSelection = true;
				}
			}
		}
		ImGui::PushID("new_template_combo");

		ImGui::PopID();
	}
	ImGui::Separator();
	if (selectedObject != nullptr) {

		static char dupname[128];
		ImGui::PushID("copy");
		if (ImGui::Button("Duplicate"))
		{
			// copy object is a bit complex but that's how it has to be done
			// 
			// we could do something like load from prefab which would instantiate a new copy of same object via lua
			// 
			// for that we need path of the object
			// 
			// we need to call it by new name
			// 
			// at the moment we load the objects and store their names etc, it's like they are level instances but stored in file
			// if we try to load same object again it will return the same object
			// I should probably change the constructor functionality and use explicit names
			// like create or get
			// we need another object loader that will load the object as new instance
			// or we make sure that we always create copy and not return existing?
			// the object json contains all info for the object to be created
			// ok it looks like we can easily use the object loader script we just have to make sure we return new object with new name
			// next are the materials, we get material and assign, this is ok but the problem is that the material contains object profile
			// that profile must be unique to the object, I think object profile should be owned by object and assigned to material when we add material to object?
			// can the material contain multiple object profiles?
			// what we would do then is add object profile to material for any object that uses this material
			// create material from the material prefab that will return always new material we just have to add new name for it
			// it should be the material name + number? with UUIDs we won't have to worry about name
			// I would like to reduce number of unnecesairly created objects
			// atm I am thinking that object profile should be a pointer
			// but who should own it?
			// it's nice in a way that we can easily swap which object profile the material is using, this is good
			// when we swap material profile we have it stored somewhere in memory, something else is using it or it is kept away for someone else to use
			// object profiles are not really meant to be reused by anyone else as they contain the object id
			// transforms are ok because we can use different transform for another object
			// id not so much
			// object profiles are just data containers and their contents change depending on shader and registry provided
			// object profiles are not always necessary elements of material
			// so they should stay pointers
			// I think all we really need is to create material instead of get material
			// We can't get because they are all unique, all unique because of object profile
			// can we do something to remove the object profile from material?
			// 
			// 
			// currently I don't like using names as keys as the identifiers for assets, we really need UUID
			// next how do we find assets if we use guids and not names
			// I think we still need paths to locate the files
			// we could go through all the files and store UUID and path info for lookups
			// 
			// it would be easier to create instance of same object
			Object* prevObject = selectedObject;
			Object* newObject = DuplicateObject(prevObject);
			selectedObject = newObject;
			newSelection = true;
		}
		if (ImGui::Button("New From This"))
		{
			Object* prevObject = selectedObject;
			Object* newObject = DuplicateObject(prevObject);
			newObject->path = prevObject->path;
			selectedObject = newObject;
			newSelection = true;
		}
		ImGui::PopID();
		Object* object = selectedObject;
		float calculatedWidth = ImGui::CalcItemWidth();
		ImGui::Text("Selection: %s", object->name.c_str());
		ImGui::Text("Picking ID %d", object->ID);
		ImGui::Text("Path: %s", object->path.c_str());

		ImGui::Separator();
		std::vector<const char*> availableComponents = { "Transform", "DirectionalLight", "PointLight", "SpotLight", "RigidBody", "Bounds", "ParticleSystem", "LineSystem", "PointSystem", "BoundingBoxSystem", "ScriptsComponent"};
		//component factory
		//new component -> (Name)
		//return component of desired type
		//maybe we can do this with <Type>

		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_HorizontalScrollbar |
			ImGuiWindowFlags_NoSavedSettings;

		std::string componentsCountLabel = std::format("Components({}):###ObjectComponents", object->components.size());
		std::string popUpLabel = "addComponentPopup";
		if (ImGui::CollapsingHeader(componentsCountLabel.c_str()))
		{
			if (ImGui::Button("Add Component", ImVec2(-1, 0.f)))
			{
				ImGui::OpenPopup(popUpLabel.c_str());
			}
			ImVec2 addComponentMin = ImGui::GetItemRectMin();
			ImVec2 addComponentMax = ImGui::GetItemRectMax();

			ImVec2 componentPopupPosition;
			componentPopupPosition.x = addComponentMin.x;
			componentPopupPosition.y = addComponentMin.y;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::SetNextWindowPos(componentPopupPosition, ImGuiCond_Appearing);
			if (ImGui::BeginPopup(popUpLabel.c_str(), flags))
			{
				ImGui::PushItemWidth(calculatedWidth);
				static ImGuiTextFilter filterAvailableComponents;
				filterAvailableComponents.Draw("##searchbar1");
				if (ImGui::BeginListBox("##add component"))
				{
					std::unordered_map<std::string, Component*> componentsMap;
					for (auto comp : object->components)
					{
						componentsMap[GetComponentName(comp.first)] = comp.second;
					}
					for (size_t i = 0; i < availableComponents.size(); i++)
					{
						if (componentsMap.find(availableComponents[i]) == componentsMap.end())
						{
							if (filterAvailableComponents.PassFilter(availableComponents[i])) {
								//ImGui::Separator();
								if (ImGui::Selectable(availableComponents[i]))
								{
									filterAvailableComponents.Clear();
								
									Component* newComponent = nullptr;
									switch (i)
									{
									case 0:
										object->AddComponent(GraphicsStorage::assetRegistry.AllocAsset<Node>());
										break;
									case 1:
										object->AddComponent(GraphicsStorage::assetRegistry.AllocAsset<DirectionalLight>());
										break;
									case 2:
										object->AddComponent(GraphicsStorage::assetRegistry.AllocAsset<PointLight>());
										break;
									case 3:
										object->AddComponent(GraphicsStorage::assetRegistry.AllocAsset<SpotLight>());
										break;
									case 4:
										object->AddComponent(GraphicsStorage::assetRegistry.AllocAsset<RigidBody>());
										break;
									case 5:
										object->AddComponent(GraphicsStorage::assetRegistry.AllocAsset<Bounds>());
										break;
									case 6:
										object->AddComponent(GraphicsStorage::assetRegistry.AllocAsset<ParticleSystem>(7000,700));
										break;
									case 7:
										object->AddComponent(GraphicsStorage::assetRegistry.AllocAsset<LineSystem>(7000));
										break;
									case 8:
										object->AddComponent(GraphicsStorage::assetRegistry.AllocAsset<PointSystem>(7000));
										break;
									case 9:
										object->AddComponent(GraphicsStorage::assetRegistry.AllocAsset<BoundingBoxSystem>(7000));
										break;
									case 10:
										object->AddComponent(GraphicsStorage::assetRegistry.AllocAsset<ScriptsComponent>());
										break;
									default:
										break;
									}
								}
							}
						}
					}
					ImGui::EndListBox();
				}
				ImGui::PopItemWidth();
				ImGui::EndPopup();
			}
			ImGui::PopStyleVar(2);

			ImGui::PushItemWidth(-1);

			ImGui::Text("Static Components(%d):", object->components.size() - object->dynamicComponents.size());
			static ImGuiTextFilter filterStaticComponentsOnObject;
			filterStaticComponentsOnObject.Draw("##staticComponentsSearchbar");
			std::unordered_map<std::type_index, Component*> staticComponents = object->GetStaticComponents();
			for (auto& component : staticComponents)
			{
				DrawComponentUI(filterStaticComponentsOnObject, object, component.second, GetComponentName(component.first), false, newSelection);
			}
			ImGui::Text("Dynamic Components(%d):", object->dynamicComponents.size());
			static ImGuiTextFilter filterDynamicComponentsOnObject;
			filterDynamicComponentsOnObject.Draw("##dynamicComponentsSearchbar");
			std::unordered_map<std::type_index, Component*> dynamicComponents = object->dynamicComponents;
			for (auto& component : dynamicComponents)
			{
				DrawComponentUI(filterDynamicComponentsOnObject, object, component.second, GetComponentName(component.first), true, newSelection);
			}
			
			ImGui::PopItemWidth();
		}

		ImGui::Separator();
		std::string materialsCountLabel = std::format("Material Sequences({}):###MaterialSequences", object->materials.size());
		if (ImGui::CollapsingHeader(materialsCountLabel.c_str()))
		{
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Name:"); ImGui::SameLine();
			static char name[128] = "";
			
			ImGui::PushItemWidth(ImGui::GetFontSize() * -16.f);
			ImGui::InputText("##new material name for new sequence", name, IM_ARRAYSIZE(name));
			ImGui::PopItemWidth();
			ImGui::SameLine();

			if (ImGui::Button("Create New Sequence Material##new material btn for new sequence"))
			{
				Material* newMaterial = GraphicsStorage::assetRegistry.AllocAsset<Material>();
				newMaterial->name = name;
				object->AddMaterial(newMaterial);
			}

			std::string popUpLabel = "addMaterialToNewSequencePopup";
			if (ImGui::Button("Add Marterial To New Sequence"))
			{
				ImGui::OpenPopup(popUpLabel.c_str());
			}
			

			ImVec2 addMaterialItemMin = ImGui::GetItemRectMin();
			ImVec2 addMaterialItemMax = ImGui::GetItemRectMax();

			ImVec2 materialPopupPosition;
			materialPopupPosition.x = addMaterialItemMin.x;
			materialPopupPosition.y = addMaterialItemMin.y;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::SetNextWindowPos(materialPopupPosition, ImGuiCond_Appearing);
			if (ImGui::BeginPopup(popUpLabel.c_str(), flags))
			{
				ImGui::PushItemWidth(calculatedWidth);
				static ImGuiTextFilter filterAvailableMaterials;
				filterAvailableMaterials.Draw("##searchbarAvailableMaterials");
				if (ImGui::BeginListBox("##Available Materials"))
				{
					int i = 0;
					for (auto& material : *GraphicsStorage::assetRegistry.GetPool<Material>())
					{
						if (filterAvailableMaterials.PassFilter(material.name.c_str())) {
							std::string selectableName = std::format("{}##{}", material.name, i);
							if (ImGui::Selectable(selectableName.c_str()))
							{
								/*
								Material* newMaterial = new Material();
								*newMaterial = *material.second;
								//const void * address = static_cast<const void*>(newMaterial);
								std::stringstream ss;
								ss << newMaterial;
								newMaterial->name = material.first + "_" + ss.str();
								GraphicsStorage::materials[newMaterial->name] = newMaterial;
								*/
								object->AddMaterial(&material);
							}
							i++;
						}
					}
					ImGui::EndListBox();
				}
				ImGui::PopItemWidth();
				ImGui::EndPopup();
			}
			ImGui::PopStyleVar(2);

			std::vector<Material*> materialsToRemove;
			int sequenceIndex = 0;
			for (auto& matSq : object->materials)
			{
				if (ImGui::CollapsingHeader(std::string("Sequence: " + std::to_string(sequenceIndex) + " Size: " + std::to_string(matSq.size())).c_str()))
				{

					ImGui::Separator();

					ImGui::PushItemWidth(ImGui::GetFontSize() * -7.f);
					ImGui::InputText("##new material name", name, IM_ARRAYSIZE(name));
					ImGui::PopItemWidth();
					ImGui::SameLine();

					if (ImGui::Button("Create New##new material btn", ImVec2(-1, 0)))
					{
						Material* newMaterial = GraphicsStorage::assetRegistry.AllocAsset<Material>();
						newMaterial->name = name;
						object->AddMaterial(newMaterial, sequenceIndex);
					}

					std::string popUpLabel = "addMaterialPopupIntoSameSequence";
					if (ImGui::Button(std::string("Add Marterial##addmaterialintosamesequence" + std::to_string(sequenceIndex)).c_str()))
					{
						ImGui::OpenPopup(popUpLabel.c_str());
					}


					ImVec2 addMaterialItemMin = ImGui::GetItemRectMin();
					ImVec2 addMaterialItemMax = ImGui::GetItemRectMax();

					ImVec2 materialPopupPosition;
					materialPopupPosition.x = addMaterialItemMin.x;
					materialPopupPosition.y = addMaterialItemMin.y;

					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
					ImGui::SetNextWindowPos(materialPopupPosition, ImGuiCond_Appearing);
					if (ImGui::BeginPopup(popUpLabel.c_str(), flags))
					{
						ImGui::PushItemWidth(calculatedWidth);
						static ImGuiTextFilter filterAvailableMaterials;
						filterAvailableMaterials.Draw("##searchbarAvailableMaterials2");
						if (ImGui::BeginListBox("##Available Materials2"))
						{
							int i = 0;
							for (auto& material : *GraphicsStorage::assetRegistry.GetPool<Material>())
							{
								if (filterAvailableMaterials.PassFilter(material.name.c_str())) {
									std::string selectableName = std::format("{}##{}", material.name, i);
									if (ImGui::Selectable(selectableName.c_str()))
									{
										/*
										Material* newMaterial = new Material();
										*newMaterial = *material.second;
										//const void * address = static_cast<const void*>(newMaterial);
										std::stringstream ss;
										ss << newMaterial;
										newMaterial->name = material.first + "_" + ss.str();
										GraphicsStorage::materials[newMaterial->name] = newMaterial;
										*/
										object->AddMaterial(&material, sequenceIndex);
									}
									i++;
								}
							}
							ImGui::EndListBox();
						}
						ImGui::PopItemWidth();
						ImGui::EndPopup();
					}
					ImGui::PopStyleVar(2);

				
					for (auto mat : matSq)
					{
						ImGui::Separator();
						std::string materialButtonLabel = mat->name + "##MaterialButton";
						//float buttonWidth = ImGui::GetWindowWidth()*0.5-13.f;
						float buttonWidth = ImGui::GetContentRegionAvail().x * 0.5f - 4.5f;

						if (ImGui::Button(materialButtonLabel.c_str(), ImVec2(buttonWidth, 0)))
						{

						}
						ImGui::SameLine();
						std::string removeButtonLabel = "Remove##" + mat->name;
						if (ImGui::Button(removeButtonLabel.c_str(), ImVec2(buttonWidth, 0)))
						{
							materialsToRemove.push_back(mat);
						}
						if (ImGui::TreeNode(mat->name.c_str(), "Elements"))
						{
							GenerateMaterialElementsUI(mat);

							ImGui::TreePop();
						}
					}
				}
				sequenceIndex++;
			}

			for (auto& material : materialsToRemove)
			{
				object->RemoveMaterial(material);
			}
		}
		ImGui::Separator();
		//ObjectProfile* matOp = object->GetComponent<ObjectProfile>();
		//DataRegistry* opRegistry = matOp != nullptr ? &matOp->registry : nullptr;
		DataRegistry* opRegistry = &object->registry;

		//if (opRegistry == nullptr)
		//{
		//	if (ImGui::Button("Add ObjectProfile"))
		//	{
		//		std::string oname = object->name + "_op##" + std::to_string(GraphicsStorage::objectProfiles.size());
		//		ObjectProfile* newOp = GraphicsStorage::objectProfiles[oname] = GraphicsStorage::assetRegistry.AllocAsset<ObjectProfile>();
		//		newOp->name = oname;
		//		for (auto& matSq : object->materials)
		//		{
		//			for (auto mat : matSq)
		//			{
		//				mat->AssignObjectProfile(newOp);
		//			}
		//		}
		//		object->AddComponent(newOp);
		//	}
		//}
		//else
		{
			if (LuaProperties(opRegistry, "obj", "resources/objects/properties/", object))
			{
				//for (auto& matSq : object->materials)
				//{
				//	for (auto mat : matSq)
				//	{
				//		if (mat->op != nullptr)
				//		{
				//			mat->op->UpdateProfileFromDataRegistry(*opRegistry);
				//		}
				//	}
				//}
			}
			ImGui::Separator();

			if (ObjectProperties(opRegistry, "Object"))
			{
				//for (auto& matSq : object->materials)
				//{
				//	for (auto mat : matSq)
				//	{
				//		if (mat->op != nullptr)
				//		{
				//			mat->op->UpdateProfileFromDataRegistry(*opRegistry);
				//		}
				//	}
				//}
			}
			ImGui::Separator();

			if (DataRegistryEditor(opRegistry))
			{
				//for (auto& matSq : object->materials)
				//{
				//	for (auto mat : matSq)
				//	{
				//		if (mat->op != nullptr)
				//		{
				//			mat->op->UpdateProfileFromDataRegistry(*opRegistry);
				//		}
				//	}
				//}
			}
		}

		if (ImGui::Button("Revert"))
		{
		}
		ImGui::SameLine();
		{
			std::string popUpLabel = "newnamepop";
			if (ImGui::Button("Save"))
			{
				if (object->path.empty())
				{
					ImGui::OpenPopup(popUpLabel.c_str());
				}
				else
				{
					lua_State* L = luaL_newstate();
					luaL_openlibs(L);
					LuaTools::dofile(L, "resources/serialization/object_saver.lua");
					lua_getglobal(L, "SaveObject");
					if (lua_isfunction(L, -1))
					{
						lua_pushlightuserdata(L, object);
						int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
						lua_pop(L, 1);
					}
					lua_close(L);
				}
			}
			float calculatedWidth = ImGui::CalcItemWidth();

			ImGuiWindowFlags flags =
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoDecoration |
				ImGuiWindowFlags_HorizontalScrollbar |
				ImGuiWindowFlags_NoSavedSettings;
			ImVec2 addComponentMin = ImGui::GetItemRectMin();
			ImVec2 addComponentMax = ImGui::GetItemRectMax();

			ImVec2 componentPopupPosition;
			componentPopupPosition.x = addComponentMin.x;
			componentPopupPosition.y = addComponentMin.y;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::SetNextWindowPos(componentPopupPosition, ImGuiCond_Appearing);
			if (ImGui::BeginPopup(popUpLabel.c_str(), flags))
			{
				ImGui::PushItemWidth(calculatedWidth);
				std::string path = "resources/objects";
				bool fileAlreadyExists = false;
				static char propertyName[128] = "";
				for (const auto& entry : std::filesystem::directory_iterator(path))
				{
					if (strcmp(entry.path().stem().string().c_str(), propertyName) == 0)
					{
						fileAlreadyExists = true;
					}
				}
				if (fileAlreadyExists)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
				}
				ImGui::InputTextWithHint("##New Property", "New Object File Name", propertyName, IM_ARRAYSIZE(propertyName));
				if (fileAlreadyExists)
				{
					ImGui::PopStyleColor();
				}
				else
				{
					if (ImGui::Button("Save##saveObject"))
					{
						object->path = std::format("{}/{}.json", path, propertyName);
						lua_State* L = luaL_newstate();
						luaL_openlibs(L);
						LuaTools::dofile(L, "resources/serialization/object_saver.lua");
						lua_getglobal(L, "SaveObject");
						if (lua_isfunction(L, -1))
						{
							lua_pushlightuserdata(L, object);
							int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
							lua_pop(L, 1);
						}
						lua_close(L);
					}
				}
				ImGui::PopItemWidth();
				ImGui::EndPopup();
			}
			ImGui::PopStyleVar(2);

			ImGui::PushItemWidth(-1);
		}
	}
	else {
		ImGui::Text("Selection: none");
	}
}

void Editor::EditTransform(float* matrix, Object* object)
{
	static ImGuizmo::OPERATION operation(ImGuizmo::TRANSLATE);
	static ImGuizmo::MODE mode(ImGuizmo::LOCAL);
	operation = (ImGuizmo::OPERATION&)operation;
	mode = (ImGuizmo::MODE&)mode;
	if (ImGui::IsKeyPressed(ImGuiKey_T))
		operation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(ImGuiKey_E))
		operation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(ImGuiKey_R)) // r Key
		operation = ImGuizmo::SCALE;
	if (ImGui::RadioButton("Translate", operation == ImGuizmo::TRANSLATE))
		operation = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", operation == ImGuizmo::ROTATE))
		operation = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Scale", operation == ImGuizmo::SCALE))
		operation = ImGuizmo::SCALE;
	
	if (operation != ImGuizmo::SCALE)
	{
		if (ImGui::RadioButton("Local", mode == ImGuizmo::LOCAL))
			mode = ImGuizmo::LOCAL;
		ImGui::SameLine();
		if (ImGui::RadioButton("World", mode == ImGuizmo::WORLD))
			mode = ImGuizmo::WORLD;
	}
	glm::vec3 originalMatrixTranslation = {}, originalMatrixRotation = {}, originalMatrixScale = {};
	ImGuizmo::DecomposeMatrixToComponents(matrix, &originalMatrixTranslation.x, &originalMatrixRotation.x, &originalMatrixScale.x);
	glm::vec3 matrixRotation = originalMatrixRotation;

	if (ImGui::DragFloat3("Translation", &originalMatrixTranslation.x) || (ImGuizmo::IsUsing() && operation == ImGuizmo::TRANSLATE))
	{
		object->node->SetPosition(originalMatrixTranslation);
	}
	bool isMouseDragging = false;
	if (ImGui::DragFloat3("Rotation", &matrixRotation.x))
	{
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			isMouseDragging = true;
		}
		if (ImGui::IsItemDeactivatedAfterEdit() || !ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			glm::quat newOri = glm::quat(glm::radians(matrixRotation));
			object->node->SetOrientation(newOri);
		}
		else
		{
			glm::vec3 delta = glm::radians(matrixRotation - originalMatrixRotation);
			glm::quat oriToT = glm::quat(delta);
			switch (mode)
			{
			case ImGuizmo::LOCAL:
				object->node->SetOrientation(glm::normalize(object->node->localOrientation * oriToT));
				break;
			case ImGuizmo::WORLD:
				object->node->SetOrientation(glm::normalize(oriToT * object->node->localOrientation));
				break;
			default:
				break;
			}
		}
	}
	ImGui::Checkbox("is mouse dragging", &isMouseDragging);
	if (ImGui::DragFloat3("Scale", &originalMatrixScale.x) || (ImGuizmo::IsUsing() && operation == ImGuizmo::SCALE))
	{
		object->node->SetScale(originalMatrixScale);
	}
	
	
	static bool useSnap(false);
	ImGui::Checkbox("##snapGizmo", &useSnap);
	ImGui::SameLine();
	glm::vec3 snap = glm::vec3(0.5f, 0.5f, 0.5f);
	switch (operation)
	{
	case ImGuizmo::TRANSLATE:
		ImGui::InputFloat3("Snap", &snap.x);
		break;
	case ImGuizmo::ROTATE:
		snap = snap = glm::vec3(45.0f, 45.0f, 45.0f);
		ImGui::InputFloat("Angle Snap", &snap.x);
		break;
	case ImGuizmo::SCALE:
		ImGui::InputFloat("Scale Snap", &snap.x);
		break;
	}
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	if (ImGuizmo::Manipulate(&CameraManager::Instance()->ViewF[0][0], &CameraManager::Instance()->ProjectionF[0][0], (ImGuizmo::OPERATION&)operation, (ImGuizmo::MODE&)mode, matrix, NULL, useSnap ? &snap.x : NULL))
	{
		ImGuizmo::DecomposeMatrixToComponents(matrix, &originalMatrixTranslation.x, &originalMatrixRotation.x, &originalMatrixScale.x);
		glm::quat oriToT;
		switch (operation)
		{
		case ImGuizmo::TRANSLATE:
			object->node->SetPosition(originalMatrixTranslation);
			break;
		case ImGuizmo::ROTATE:
			oriToT = glm::quat(glm::radians(originalMatrixRotation));
			object->node->SetOrientation(oriToT);
			break;
		case ImGuizmo::SCALE:
			object->node->SetScale(originalMatrixScale);
			break;
		}
	}
}

void Editor::DrawComponentUI(ImGuiTextFilter& filter, Object* object, Component* component, const std::string& name, bool isDynamic, bool newSelection)
{
	if (filter.PassFilter(name.c_str())) {
		const void* address = static_cast<const void*>(component);
		std::stringstream ss;
		ss << address;
		std::string componentAddress = ss.str();
		std::string label = std::string(name) + "##" + componentAddress;

		if (dynamic_cast<Node*>(component))
		{
			DrawComponentBasic(component, name, isDynamic, object, [&](auto component)
			{
				std::string parentName;
				if (object->node->parent == nullptr)
				{
					parentName = "no parent node";
				}
				else if (object->GetParentObject() == nullptr)
				{
					parentName = "no parent object";
				}
				else
				{
					parentName = object->GetParentObject()->name.c_str();
				}
				ImGui::Text("Parent: %s", parentName.c_str());
				ImGui::Text("Child Count: %d", object->node->children.size());
				ImGui::Separator();
				ImGui::PushItemWidth(ImGui::GetFontSize() * -7.f);
				glm::mat4 worldTransform = object->node->TopDownTransformF;
				EditTransform(&worldTransform[0][0], object);
				ImGui::PopItemWidth();
				ImGui::Separator();
				ImGui::Text("Local Movable: %s", object->node->GetMovable() ? "True" : "False");
				ImGui::Text("Final Movable: %s", object->node->GetTotalMovable() ? "True" : "False");
				bool isMovable = object->node->GetMovable();
				if (ImGui::Checkbox("Movable", &isMovable)) {
					SceneGraph::Instance()->SwitchObjectMovableMode(object, isMovable);
				}
			}, [&](auto component) {});
		}
		else if (dynamic_cast<Bounds*>(component))
		{
			DrawComponentBasic(component, name, isDynamic, object, [](auto& component)
			{
				Bounds* bc = (Bounds*)component;
				glm::vec3 center = bc->centerOfMesh;
				glm::vec3 dimensions = bc->dimensions;
				std::string meshName = bc->name.empty() ? "custom" : bc->name.c_str();
				float calculatedWidth = ImGui::CalcItemWidth();

				ImGuiWindowFlags flags =
					ImGuiWindowFlags_NoMove |
					ImGuiWindowFlags_NoDecoration |
					ImGuiWindowFlags_HorizontalScrollbar |
					ImGuiWindowFlags_NoSavedSettings;

				std::string popUpLabel = "meshesPopup";

				if (ImGui::Button("Select Mesh", ImVec2(-1, 0.f)))
				{
					ImGui::OpenPopup(popUpLabel.c_str());
				}
				ImVec2 addComponentMin = ImGui::GetItemRectMin();
				ImVec2 addComponentMax = ImGui::GetItemRectMax();

				ImVec2 componentPopupPosition;
				componentPopupPosition.x = addComponentMin.x;
				componentPopupPosition.y = addComponentMin.y;

				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
				ImGui::SetNextWindowPos(componentPopupPosition, ImGuiCond_Appearing);
				if (ImGui::BeginPopup(popUpLabel.c_str(), flags))
				{
					ImGui::PushItemWidth(calculatedWidth);
					static ImGuiTextFilter filterAvailableMeshes;
					filterAvailableMeshes.Draw("##meshesSearchbar");
					if (ImGui::BeginListBox("##select mesh"))
					{
						int i = 0;
						for (auto& asset : *GraphicsStorage::assetRegistry.GetPool<VertexArray>())
						{
							if (filterAvailableMeshes.PassFilter(asset.name.c_str())) {
								std::string selectableName = std::format("{}##{}", asset.name, i);
								if (ImGui::Selectable(selectableName.c_str()))
								{
									bc->SetUp(asset.center, asset.dimensions, asset.name);
									bc->Update();
									ImGui::CloseCurrentPopup();
								}
							}
							i++;
						}
						ImGui::EndListBox();
					}
					ImGui::PopItemWidth();
					ImGui::EndPopup();
				}
				ImGui::PopStyleVar(2);

				ImGui::PushItemWidth(-1);

				if (ImGui::InputFloat3("Center", &center.x))
				{
					Bounds* bc = (Bounds*)component;
					bc->name = "custom";
					bc->SetBoundsCenter(glm::vec3(center.x, center.y, center.z));
					bc->Update();
				}
				if (ImGui::InputFloat3("Dimensions", &dimensions.x))
				{
					Bounds* bc = (Bounds*)component;
					bc->name = "custom";
					bc->SetBoundsDimensions(glm::vec3(dimensions.x, dimensions.y, dimensions.z));
					bc->Update();
				}
				ImGui::Text("%s Measurements", meshName.c_str());
			}, [&](auto component) {});
		}
		else if (dynamic_cast<ScriptsComponent*>(component))
		{
			DrawComponentBasic(component, name, isDynamic, object, [&](auto& component)
			{
				if (ImGui::Button("AddScript"))
				{
					((ScriptsComponent*)component)->AddScript("");
				}
				std::vector<Script*> scriptsToRemove;
				int j = 0;
				for (auto script : ((ScriptsComponent*)component)->scripts)
				{
					ScriptSelector(script, "resources/objects/scripts");
					ImGui::SameLine();
					if (ImGui::Button(std::format("Remove##removeScript{}", j).c_str()))
					{
						scriptsToRemove.push_back(script);
					}
					j++;
				}
				for (auto script : scriptsToRemove)
				{
					((ScriptsComponent*)component)->RemoveScript(script);
				}
			}, [&](auto component) {});
		}
		else if (dynamic_cast<ParticleSystem*>(component))
		{
			DrawComponentBasic(component, name, isDynamic, object, [this](auto& component)
			{
				ParticleSystem* pc = (ParticleSystem*)component;
				float lifeTime = pc->LifeTime;
				int emissionRate = pc->DesiredEmissionRate;
				int maxParticles = pc->MaxParticles;
				int newParticles = pc->NewParticles;
				int aliveParticles = pc->GetAliveParticlesCount();
				int reallyAliveParticles = pc->ReallyAliveParticles;
				int calculatedRate = lifeTime <= 0.0f ? 0 : (int)(aliveParticles / lifeTime);
				int maxPossibleEmissionRate = lifeTime <= 0.0f ? 0 : (int)(maxParticles / lifeTime);
				int expectedEmissionRate = std::min(emissionRate, maxPossibleEmissionRate);
				int newParticlesPerFrame = pc->GetNewParticlesCount();
				int perFrameEmissionRate = (int)(newParticlesPerFrame / Times::Instance()->deltaTime);
				ImGui::PushItemWidth(ImGui::GetFontSize() * -10.f);
				
				static std::pair<std::string, Texture*> texturePair(*renderTargets.begin());

				ImVec2 uv_min = ImVec2(1.0f, 0.0f);                 // Top-left
				ImVec2 uv_max = ImVec2(0.0f, 1.0f);                 // Lower-right
				ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
				ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
				ImVec2 comboImageSize = ImVec2(10.0f, 10.0f);

				if (texturePair.second != nullptr)
				{
					if (ImGui::BeginCombo("Textures", texturePair.first.c_str()))
					{
						int i = 0;
						for (auto& v_texture : renderTargets)
						{
							DrawSelectableTextureThumbnail(v_texture.second, comboImageSize, uv_min, uv_max, 0, border_col, tint_col);
							ImGui::SameLine();
							const bool is_selected = (texturePair.first == v_texture.first);
							std::string selectableName = std::format("{}##{}", v_texture.first, i);
							if (ImGui::Selectable(selectableName.c_str(), is_selected))
							{
								texturePair = v_texture;
								pc->SetTexture(v_texture.second->handle);
							}
							if (is_selected)
								ImGui::SetItemDefaultFocus();
							i++;
						}
						for (auto& texture : textures)
						{
							DrawSelectableTextureThumbnail(texture.second, comboImageSize, uv_min, uv_max, 0, border_col, tint_col);
							ImGui::SameLine();
							const bool is_selected = (texturePair.first == texture.first);
							std::string selectableName = std::format("{}##{}", texture.first, i);
							if (ImGui::Selectable(texture.first.c_str(), is_selected))
							{
								texturePair = texture;
								pc->SetTexture(texture.second->handle);
							}
							if (is_selected)
								ImGui::SetItemDefaultFocus();
							i++;
						}
						ImGui::EndCombo();
					}

					float imageSize = 30;
					ImVec2 size = ImVec2(imageSize, imageSize);
					DrawSelectableTextureThumbnail(texturePair.second, size, uv_min, uv_max, 0, border_col, tint_col);
				}
				ImGui::Checkbox("Additive Blending", &pc->additive);
				ImGui::ColorEdit4("Particle Color Tint", &pc->Color.x);
				ImGui::DragFloat("Size", &pc->Size);
				ImGui::DragFloat("Spread", &pc->Spread);
				ImGui::DragFloat3("Force", &pc->Force.x);
				
				ImGui::Text("Alive Particles: %d", aliveParticles);
				ImGui::Text("Really Alive Particles: %d", reallyAliveParticles);
				ImGui::Text("New Particles: %d", newParticles);
				ImGui::Text("Accumulated Emission Rate: %d", calculatedRate);
				ImGui::Text("Max Possible Emission Rate (Max Particles / Lifetime): %d", maxPossibleEmissionRate);
				ImGui::Text("Expected Emission Rate: %d", expectedEmissionRate);
				ImGui::Text("Last Particle Index: %d", pc->LastParticleIndex);
				ImGui::Text("Dead Particles: %d", pc->DeadParticles);
				static float maxValue = (float)maxPossibleEmissionRate;
				if (ImGui::DragFloat("Lifetime", &lifeTime))
				{
					lifeTime = std::max(0.0f, lifeTime);
					pc->SetLifeTime(lifeTime);
				}
				if (ImGui::DragInt("Desired Emission Rate", &emissionRate))
				{
					pc->SetEmissionRate(emissionRate);
				}
				if (ImGui::DragInt("Max Particles", &maxParticles))
				{
					pc->SetMaxParticles(maxParticles);
					maxValue = std::min(lifeTime <= 0.0f ? (float)maxParticles : (float)(maxParticles / lifeTime), maxValue);
				}
				// we might want to control the MaxParticles with Emission and LifeTime
				// to optimize the memory footprint
				// emission rate * lifetime -> max particles
				ImGui::DragFloat("Zoom", &maxValue, 1.f, 0.f, (float)maxPossibleEmissionRate);
				static float values[90] = {};
				static int values_offset = 0;

				values[values_offset] = (float)perFrameEmissionRate;
				values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);

				{
					float average = 0.0f;
					for (int n = 0; n < IM_ARRAYSIZE(values); n++)
					{
						average += values[n];
					}
					average /= (float)IM_ARRAYSIZE(values);
					char overlay[32];
					sprintf(overlay, "avg %f", average);
					ImGui::PlotLines("Emission Rate", values, IM_ARRAYSIZE(values), values_offset, overlay, -1.0f, maxValue, ImVec2(0, 40.0f));
				}

				ImGui::PopItemWidth();
			}, [&](auto component) {});
		}
		else if (dynamic_cast<LineSystem*>(component))
		{
			DrawComponentBasic(component, name, isDynamic, object, [](auto& component) {}, [&](auto component) {});
		}
		else if (dynamic_cast<PointSystem*>(component))
		{
			DrawComponentBasic(component, name, isDynamic, object, [](auto& component) {}, [&](auto component) {});
		}
		else if (dynamic_cast<BoundingBoxSystem*>(component))
		{
			DrawComponentBasic(component, name, isDynamic, object, [](auto& component) {}, [&](auto component) {});
		}
		else if (dynamic_cast<FastInstanceSystem*>(component))
		{
			DrawComponentBasic(component, name, isDynamic, object, [](auto& component) {}, [&](auto component) {});
		}
		else
		{
			DrawComponentBasic(component, name, isDynamic, object, [](auto& component) {}, [&](auto component) {});
		}
	}
}

bool Editor::DataRegistryEditor(DataRegistry* registry)
{
	if (registry == nullptr)
	{
		return false;
	}
	bool dirty = false;
	std::string propertiesCountLabel = "DataRegistry Properties(" + std::to_string(registry->bindings.size()) + "):###DataRegistryProperties";
	if (ImGui::CollapsingHeader(propertiesCountLabel.c_str()))
	{
		ImGui::PushItemWidth(-1);
		float calculatedWidth = ImGui::CalcItemWidth();

		ImGui::Separator();
		std::string popUpLabel = "Add_Property_Window";
		if (ImGui::Button("Add Property", ImVec2(-1, 0)))
		{
			ImGui::OpenPopup(popUpLabel.c_str());
		}

		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_HorizontalScrollbar |
			ImGuiWindowFlags_NoSavedSettings;

		ImVec2 addMaterialItemMin = ImGui::GetItemRectMin();
		ImVec2 addMaterialItemMax = ImGui::GetItemRectMax();

		ImVec2 materialPopupPosition;
		materialPopupPosition.x = addMaterialItemMin.x;
		materialPopupPosition.y = addMaterialItemMin.y;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::SetNextWindowPos(materialPopupPosition, ImGuiCond_Appearing);
		if (ImGui::BeginPopup(popUpLabel.c_str(), flags))
		{
			static char propertyName[128] = "";
			ImGui::PushItemWidth(calculatedWidth);
			ImGui::InputTextWithHint("##New Property", "New Property Name", propertyName, IM_ARRAYSIZE(propertyName));
			static std::string currentPropertyType = "INT";
			static PropertyType type = PropertyType::INT;
			static void* data = nullptr;
			static int size = 0;
			ImGuiStyle& style = ImGui::GetStyle();
			float w = calculatedWidth;
			float spacing = style.ItemInnerSpacing.x;
			float button_sz = ImGui::GetFrameHeight();
			ImGui::PushItemWidth(w - spacing * 2.0f - button_sz * 2.0f);
			if (ImGui::BeginCombo("##TypeCombo", currentPropertyType.c_str(), ImGuiComboFlags_NoArrowButton))
			{
				int typeIndex = 0;
				for (auto& propertyType : DataInfo::PropertyTypesAsString)
				{
					const bool is_selected = (strcmp(currentPropertyType.c_str(), propertyType) == 0);
					if (ImGui::Selectable(propertyType, is_selected))
					{
						type = (PropertyType)typeIndex;
						currentPropertyType = propertyType;
						memset(data, 0, size);
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
					typeIndex++;
				}
				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();
			ImGui::SameLine(0, spacing);
			if (ImGui::ArrowButton("##leftArrow", ImGuiDir_Left))
			{
				if ((int)type <= 0)
				{
					type = (PropertyType)(IM_ARRAYSIZE(DataInfo::PropertyTypesAsString) - 1);
				}
				else
				{
					type = (PropertyType)((int)type - 1);
				}
				currentPropertyType = DataInfo::PropertyTypesAsString[(int)type];
			}
			ImGui::SameLine(0, spacing);
			if (ImGui::ArrowButton("##rightArrow", ImGuiDir_Right))
			{
				if ((int)type >= (IM_ARRAYSIZE(DataInfo::PropertyTypesAsString) - 1))
				{
					type = PropertyType::INT;
				}
				else
				{
					type = (PropertyType)((int)type + 1);
				}
				currentPropertyType = DataInfo::PropertyTypesAsString[(int)type];
			}
			//ImGui::SameLine(0, style.ItemInnerSpacing.x);
			//ImGui::Text("Type");

			static int ivalue;
			static float fvalue;
			static char cvalue[128];
			static bool bvalue;
			static glm::vec2 row21;
			static glm::vec2 row22;
			static glm::vec3 row31;
			static glm::vec3 row32;
			static glm::vec3 row33;
			static glm::vec4 row41;
			static glm::vec4 row42;
			static glm::vec4 row43;
			static glm::vec4 row44;
			static glm::vec2 row2;
			static glm::vec3 row3;
			static glm::vec4 row4;
			static glm::mat4 mat4;
			static glm::mat3 mat3;
			static glm::vec4 mat2;
			switch (type)
			{
			case PropertyType::INT:
				ImGui::DragInt("##value", &ivalue);
				data = &ivalue;
				size = sizeof(int);
				break;
			case PropertyType::UINT:
				ImGui::DragInt("##value", &ivalue);
				data = &ivalue;
				size = sizeof(unsigned int);
				break;
			case PropertyType::FLOAT:
				ImGui::DragFloat("##value", &fvalue);
				data = &fvalue;
				size = sizeof(float);
				break;
			case PropertyType::CHAR:
				ImGui::InputText("##value", cvalue, IM_ARRAYSIZE(cvalue));
				data = &cvalue;
				size = (int)(sizeof(char)*strlen(cvalue));
				break;
			case PropertyType::BOOL:
				ImGui::Checkbox("##value", &bvalue);
				data = &bvalue;
				size = sizeof(bool);
				break;
			case PropertyType::MAT2:
				ImGui::DragFloat2("##value1", &row21.x);
				ImGui::DragFloat2("##value2", &row22.x);
				data = &mat2;
				size = sizeof(glm::vec4);
				break;
			case PropertyType::MAT3:
				ImGui::DragFloat3("##value1", &row31.x);
				ImGui::DragFloat3("##value2", &row32.x);
				ImGui::DragFloat3("##value3", &row33.x);
				data = &mat3;
				size = sizeof(glm::mat3);
				break;
			case PropertyType::MAT4:
				ImGui::DragFloat4("##value1", &row41.x);
				ImGui::DragFloat4("##value2", &row42.x);
				ImGui::DragFloat4("##value3", &row43.x);
				ImGui::DragFloat4("##value4", &row44.x);
				data = &mat4;
				size = sizeof(glm::mat4);
				break;
			case PropertyType::VEC2:
				ImGui::DragFloat2("##value", &row2.x);
				data = &row2;
				size = sizeof(glm::vec2);
				break;
			case PropertyType::VEC3:
				ImGui::DragFloat3("##value", &row3.x);
				data = &row3;
				size = sizeof(glm::vec3);
				break;
			case PropertyType::VEC4:
				ImGui::DragFloat4("##value", &row4.x);
				data = &row4;
				size = sizeof(glm::vec4);
				break;
			default:
				break;
			}
			if (ImGui::Button("Add", ImVec2(-1, 0)))
			{
				if (strcmp(propertyName, "") != 0)
				{
					registry->AddAndRegisterProperty(propertyName, data, size, type);
					memset(propertyName, 0, IM_ARRAYSIZE(propertyName));
					dirty = true;
				}
			}
			ImGui::PopItemWidth();
			ImGui::EndPopup();
		}

		ImGui::PopStyleVar(2);
		ImGui::Separator();
		ImGui::PushItemWidth(-1);
		static ImGuiTextFilter filterProperties;
		filterProperties.Draw("##searchbarProperties");
		ImGui::PushItemWidth(ImGui::GetFontSize() * -14);
		ImGui::Separator();
		ImGui::Text("Existing Built-In Properties");
		ImGui::Separator();
		std::vector<std::string> propertiesToRemove;
		for (auto& nameBindingIt : registry->bindings)
		{
			if (registry->pb.bindings.find(nameBindingIt.first) == registry->pb.bindings.end())
			{
				if (filterProperties.PassFilter(nameBindingIt.first.c_str()))
				{
					if (nameBindingIt.second.dataAddress == nullptr)
					{
						ImGui::Text("Property has no data");
						// we can either
						// get properties of components <- pull
						// and register in object profile
						// for updating of uniform buffers
						// of components register properties in object profile -> push
						// we could also have scripts on object profile, material profile, render profile and render pass
						// the scripts would be executed when needed and they would upload the necessary properties
						// the scripts would get the properties they need at init
					}
					else
					{
						DrawPropertyUI(nameBindingIt.second, nameBindingIt.first);
					}
					ImGui::SameLine();
					if (ImGui::Button(std::string("X##removeBuiltInProperty" + nameBindingIt.first).c_str()))
					{
						propertiesToRemove.push_back(nameBindingIt.first);
						dirty = true;
					}
					ImGui::Separator();
				}
			}
		}
		ImGui::Separator();
		ImGui::Text("Added Properties");
		ImGui::Separator();

		for (auto& nameBindingIt : registry->pb.bindings)
		{
			if (filterProperties.PassFilter(nameBindingIt.first.c_str()))
			{
				DrawPropertyUI(nameBindingIt.second.info, nameBindingIt.first);
				ImGui::SameLine();
				if (ImGui::Button(std::string("X##removeProperty" + nameBindingIt.first).c_str()))
				{
					propertiesToRemove.push_back(nameBindingIt.first);
					dirty = true;
				}
				ImGui::Separator();
			}
		}

		for (auto& propertyName : propertiesToRemove)
		{
			registry->UnregisterProperty(propertyName.c_str());
		}

		ImGui::PopItemWidth();
		ImGui::PopItemWidth();
	}
	return dirty;
}

void Editor::DrawPropertyUI(DataInfo& info, const std::string& name)
{
	float* floatValue = nullptr;
	int* intValue = nullptr;
	switch (info.type)
	{
	case PropertyType::INT:
		intValue = (int*)info.dataAddress;
		ImGui::DragInt(name.c_str(), intValue);
		break;
	case PropertyType::UINT:
		intValue = (int*)info.dataAddress;
		ImGui::DragInt(name.c_str(), intValue);
		break;
	case PropertyType::FLOAT:
		floatValue = (float*)info.dataAddress;
		ImGui::DragFloat(name.c_str(), floatValue, 0.1f);
		break;
	case PropertyType::CHAR:
		ImGui::Text("Property Name: %s", name.c_str());
		ImGui::Text("Property Value: %s", std::string((char*)info.dataAddress, info.size).c_str());
		break;
	case PropertyType::BOOL:
		ImGui::Checkbox(name.c_str(), (bool*)info.dataAddress);
		break;
	case PropertyType::MAT2:
		floatValue = (float*)info.dataAddress;
		ImGui::DragFloat2(std::string("##1" + name).c_str(), floatValue);
		ImGui::SameLine(); ImGui::Text(name.c_str());
		ImGui::DragFloat2(std::string("##2" + name).c_str(), floatValue + 4);
		break;
	case PropertyType::MAT3:
		floatValue = (float*)info.dataAddress;
		ImGui::DragFloat3(std::string("##1" + name).c_str(), floatValue);
		ImGui::SameLine(); ImGui::Text(name.c_str());
		ImGui::DragFloat3(std::string("##2" + name).c_str(), floatValue + 4);
		ImGui::DragFloat3(std::string("##3" + name).c_str(), floatValue + 8);
		break;
	case PropertyType::MAT4:
		floatValue = (float*)info.dataAddress;
		ImGui::DragFloat4(std::string("##1" + name).c_str(), floatValue);
		ImGui::SameLine(); ImGui::Text(name.c_str());
		ImGui::DragFloat4(std::string("##2" + name).c_str(), floatValue + 4);
		ImGui::DragFloat4(std::string("##3" + name).c_str(), floatValue + 8);
		ImGui::DragFloat4(std::string("##4" + name).c_str(), floatValue + 12);
		break;
	case PropertyType::VEC2:
		floatValue = (float*)info.dataAddress;
		ImGui::DragFloat2(name.c_str(), floatValue);
		break;
	case PropertyType::VEC3:
		floatValue = (float*)info.dataAddress;
		ImGui::DragFloat3(name.c_str(), floatValue);
		break;
	case PropertyType::VEC4:
		floatValue = (float*)info.dataAddress;
		ImGui::DragFloat4(name.c_str(), floatValue);
		break;
	default:
		break;
	}
}

bool Editor::LuaProperties(DataRegistry* registry, const char* label, const char* pathToLua, void* registerArgument)
{
	if (registry == nullptr)
	{
		return false;
	}
	ImGui::PushID(label);
	bool dirty = false;
	int nrOfLuaProperties = 0;
	if (GraphicsStorage::luaProperties.find(registry) != GraphicsStorage::luaProperties.end()) nrOfLuaProperties = (int)GraphicsStorage::luaProperties[registry].size();
	std::string propertiesCountLabel = "Lua Properties(" + std::to_string(nrOfLuaProperties) + "):###LuaProperties";
	std::string popUpLabel = "Add_Lua_Property_Window";
	
	//collect all properties that already exist
	std::unordered_set<std::string> registeredProperties;
	for (auto& binding : registry->bindings)
	{
		registeredProperties.insert(binding.first);
	}
	if (ImGui::CollapsingHeader(propertiesCountLabel.c_str()))
	{
		ImGui::PushItemWidth(-1);
		float calculatedWidth = ImGui::CalcItemWidth();

		if (ImGui::Button("Add Lua Properties##AddLuaPropertiesButton", ImVec2(-1, 0)))
		{
			ImGui::OpenPopup(popUpLabel.c_str());
		}

		ImVec2 addMaterialItemMin = ImGui::GetItemRectMin();
		ImVec2 addMaterialItemMax = ImGui::GetItemRectMax();

		ImVec2 materialPopupPosition;
		materialPopupPosition.x = addMaterialItemMin.x;
		materialPopupPosition.y = addMaterialItemMin.y;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::SetNextWindowPos(materialPopupPosition, ImGuiCond_Appearing);

		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_HorizontalScrollbar |
			ImGuiWindowFlags_NoSavedSettings;

		std::string addedScript = "";
		if (ImGui::BeginPopup(popUpLabel.c_str(), flags))
		{
			ImGui::PushItemWidth(calculatedWidth);
			static ImGuiTextFilter filterAvailablePropertyScripts;
			filterAvailablePropertyScripts.Draw("##filterAvailableLuaPropertyScripts");

			std::vector<std::string> scriptNames;
			GraphicsManager::GetFileNames(scriptNames, pathToLua, ".lua");
			if (ImGui::BeginListBox("##AvailableLuaPropertyScriptsList"))
			{
				for (auto& scriptName : scriptNames)
				{
					if (filterAvailablePropertyScripts.PassFilter(scriptName.c_str())) {
						if (ImGui::Selectable(scriptName.c_str()))
						{
							addedScript = scriptName;
							lua_State* L = luaL_newstate();
							luaL_openlibs(L);
							std::string scriptPath = std::string(pathToLua) + scriptName + std::string(".lua");
							LuaTools::dofile(L, scriptPath.c_str());
							lua_getglobal(L, "Register");
							if (lua_isfunction(L, -1))
							{
								lua_pushlightuserdata(L, registerArgument);
								lua_pushstring(L, scriptPath.c_str());
								int result = LuaTools::report(L, LuaTools::docall(L, 2, 1));
								lua_pop(L, 1);
								dirty = true;
							}
							lua_close(L);
							filterAvailablePropertyScripts.Clear();
						}
					}
				}
				ImGui::EndListBox();
			}
			ImGui::PopItemWidth();
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar(2);

		if (GraphicsStorage::luaProperties.find(registry) != GraphicsStorage::luaProperties.end()) nrOfLuaProperties = (int)GraphicsStorage::luaProperties[registry].size();
		ImGui::Text("Added Lua Properties(%d):", nrOfLuaProperties);

		std::vector<std::string> luaPropertiesToRemove;
		static ImGuiTextFilter filterAddedPropertyScripts;
		filterAddedPropertyScripts.Draw("##filterAddedLuaPropertyScripts");
		if (ImGui::BeginListBox("##AddedLuaPropertyList"))
		{
			if (nrOfLuaProperties > 0)
			{
				ImGui::Columns(2, "LuaPropertiesColumns", false);
				for (auto& scriptName : GraphicsStorage::luaProperties[registry])
				{
					if (filterAddedPropertyScripts.PassFilter(scriptName.first.c_str())) {
						ImGui::AlignTextToFramePadding();
						if (ImGui::Selectable(scriptName.first.c_str()))
						{
						}
						ImGui::NextColumn();
						std::string removeButtonLabel = "Remove##RemoveLuaPropertyFile" + scriptName.first;
						if (ImGui::Button(removeButtonLabel.c_str(), ImVec2(-1, 0)))
						{
							luaPropertiesToRemove.push_back(scriptName.first);
						}
						ImGui::NextColumn();
						if (ImGui::GetColumnIndex() == 0)
							ImGui::Separator();
					}
					for (auto& lProperty : scriptName.second)
					{
						if (registry->GetProperty(lProperty.c_str()) == nullptr)
						{
							luaPropertiesToRemove.push_back(scriptName.first);
							break;
						}
					}
				}
			}
			ImGui::EndListBox();
		}

		for (auto& scriptName : luaPropertiesToRemove)
		{
			for (auto& lproperty: GraphicsStorage::luaProperties[registry][scriptName])
			{
				registry->UnregisterProperty(lproperty.c_str());
				dirty = true;
			}
			
			GraphicsStorage::luaProperties[registry].erase(scriptName);
			if (GraphicsStorage::luaProperties[registry].size() == 0)
			{
				GraphicsStorage::luaProperties.erase(registry);
			}
		}
	}
	ImGui::PopID();
	return dirty;
}

bool Editor::ObjectProperties(DataRegistry* registry, const char* label)
{
	if (registry == nullptr)
	{
		return false;
	}
	ImGui::PushID(label);
	bool dirty = false;
	int propertiesCount = 0;
	if (GraphicsStorage::objectProperties.find(registry) != GraphicsStorage::objectProperties.end())
	{
		for (auto op : GraphicsStorage::objectProperties[registry])
		{
			propertiesCount += (int)op.second.size();
		}
	}
	std::string propertiesCountLabel = "Object Properties(" + std::to_string(propertiesCount) + "):###ObjectProperties";
	std::string popUpLabel = "Add_Object_Property_Window";

	if (ImGui::CollapsingHeader(propertiesCountLabel.c_str()))
	{
		ImGui::PushItemWidth(-1);
		float calculatedWidth = ImGui::CalcItemWidth();

		if (ImGui::Button("Add Object Properties##AddObjectPropertiesButton", ImVec2(-1, 0)))
		{
			ImGui::OpenPopup(popUpLabel.c_str());
		}

		ImVec2 itemMin = ImGui::GetItemRectMin();
		ImVec2 itemMax = ImGui::GetItemRectMax();

		ImVec2 popupPosition;
		popupPosition.x = itemMin.x;
		popupPosition.y = itemMin.y;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::SetNextWindowPos(popupPosition, ImGuiCond_Appearing);

		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_HorizontalScrollbar |
			ImGuiWindowFlags_NoSavedSettings;

		if (ImGui::BeginPopup(popUpLabel.c_str(), flags))
		{
			static char propertyName[128] = "";
			ImGui::PushItemWidth(calculatedWidth);
			ImGui::InputTextWithHint("##New Object Property", "New Property Name", propertyName, IM_ARRAYSIZE(propertyName));

			ImGuiStyle& style = ImGui::GetStyle();
			float spacing = style.ItemSpacing.x;
			ImGui::PushItemWidth((calculatedWidth - spacing) / 2.f);
			//ImGui::PushItemWidth(calculatedWidth/2.f);
			static ImGuiTextFilter filterAvailableObjects;
			filterAvailableObjects.Draw("##filterAvailableObjects");
			ImGui::SameLine();
			static ImGuiTextFilter filterAvailableProperties;
			filterAvailableProperties.Draw("##filterAvailableObjectProperties");
			static Object* currentObject = nullptr;
			bool objectChanged = false;
			if (ImGui::BeginListBox("##AvailableObjectsList"))
			{
				int i = 0;
				for (auto& object : *GraphicsStorage::assetRegistry.GetPool<Object>())
				{
					if (filterAvailableObjects.PassFilter(object.name.c_str())) {
						std::string selectableName = std::format("{}##{}", object.name, i);
						const bool is_selected = &object == currentObject;
						if (ImGui::Selectable(selectableName.c_str(), is_selected))
						{
							currentObject = &object;
							objectChanged = true;
							filterAvailableObjects.Clear();
						}
						if (is_selected)
						{
							ImGui::SetItemDefaultFocus();
						}
						i++;
					}
				}
				ImGui::EndListBox();
			}
			ImGui::SameLine();
			static std::string currentProperty = "";
			if (objectChanged) currentProperty = "";
			if (ImGui::BeginListBox("##AvailableObjectPropertiesList"))
			{
				if (currentObject != nullptr)
				{
					//ObjectProfile* objectOp = object->GetComponent<ObjectProfile>();
					//if (objectOp != nullptr)
					{
						//auto& registry = objectOp->registry;
						auto& registry = currentObject->registry;
						for (auto& oproperty : registry.bindings)
						{
							if (filterAvailableObjects.PassFilter(oproperty.first.c_str())) {
								const bool is_selected = oproperty.first == currentProperty;
								if (ImGui::Selectable(oproperty.first.c_str(), is_selected))
								{
									currentProperty = oproperty.first;
									filterAvailableObjects.Clear();
								}
								if (is_selected)
								{
									ImGui::SetItemDefaultFocus();
								}
							}
						}
					}
					
				}
				ImGui::EndListBox();
			}
			if (ImGui::Button("Add Property", ImVec2(-1, 0)))
			{
				if (currentObject != nullptr && !currentProperty.empty() && strcmp(propertyName, "") != 0)
				{
					//ObjectProfile* objectOp = object->GetComponent<ObjectProfile>();
					//if (objectOp != nullptr)
					{
						//DataRegistry* objectDr = objectOp != nullptr ? &objectOp->registry : nullptr;
						//auto oproperty = objectDr->GetProperty(currentProperty.c_str());
						auto oproperty = currentObject->registry.GetProperty(currentProperty.c_str());

						registry->RegisterProperty(std::string(propertyName), oproperty);
						std::string oguid = GraphicsStorage::assetRegistry.GetAssetIDAsString(currentObject);
						GraphicsStorage::objectProperties[registry][oguid][currentProperty] = std::string(propertyName);
						memset(propertyName, 0, IM_ARRAYSIZE(propertyName));
						dirty = true;
					}
				}
			}
			ImGui::PopItemWidth();
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar(2);

		int propertiesCount = 0;
		if (GraphicsStorage::objectProperties.find(registry) != GraphicsStorage::objectProperties.end())
		{
			for (auto op : GraphicsStorage::objectProperties[registry])
			{
				propertiesCount += (int)op.second.size();
			}
		}
		ImGui::Text("Added Object Properties(%d):", propertiesCount);

		std::unordered_map<std::string, std::unordered_map<std::string,std::string>> objectPropertiesToRemove;
		static ImGuiTextFilter filterAddedProperties;
		filterAddedProperties.Draw("##filterAddedProperties");
		if (ImGui::BeginListBox("##AddedPropertiesList"))
		{
			if (propertiesCount > 0)
			{
				ImGui::Columns(4, "ObjectPropertiesColumns", false);
				for (auto& objectProperties : GraphicsStorage::objectProperties[registry])
				{
					for (auto& oproperty : objectProperties.second)
					{
						if (filterAddedProperties.PassFilter(oproperty.first.c_str())) {
							ImGui::AlignTextToFramePadding();
							ImGui::Text(objectProperties.first.c_str());
							ImGui::NextColumn();
							ImGui::Text(oproperty.first.c_str());
							ImGui::NextColumn();
							ImGui::Text(oproperty.second.c_str());
							ImGui::NextColumn();
							std::string removeButtonLabel = "Remove##RemoveObjectProperty" + oproperty.first;
							if (ImGui::Button(removeButtonLabel.c_str(), ImVec2(-1, 0)))
							{
								objectPropertiesToRemove[objectProperties.first].insert(oproperty);
							}
							ImGui::NextColumn();
							if (ImGui::GetColumnIndex() == 0)
								ImGui::Separator();
						}
						if (registry->GetProperty(oproperty.second.c_str()) == nullptr)
						{
							objectPropertiesToRemove[objectProperties.first].insert(oproperty);
						}
					}
				}
			}
			ImGui::EndListBox();
		}

		for (auto& oproperties : objectPropertiesToRemove)
		{
			for (auto& oproperty : oproperties.second)
			{
				GraphicsStorage::objectProperties[registry][oproperties.first].erase(oproperty.first); //erase property of an object
				registry->UnregisterProperty(oproperty.second.c_str());
				dirty = true;
			}
			if (GraphicsStorage::objectProperties[registry][oproperties.first].size() == 0)
			{
				GraphicsStorage::objectProperties[registry].erase(oproperties.first); //erase the object if there are no more properties
			}
		}
		if (GraphicsStorage::objectProperties.find(registry) != GraphicsStorage::objectProperties.end())
		{
			if (GraphicsStorage::objectProperties[registry].size() == 0)
			{
				GraphicsStorage::objectProperties.erase(registry); //erase owner since we don't have any objects
			}
		}
	}
	ImGui::PopID();
	return dirty;
}

void Editor::AddObjectToSceneGrapghSelection(Object* obj, bool isSelected)
{
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
	{
		if (isSelected)
		{
			auto res = std::find(selectionList.begin(), selectionList.end(), obj);
			if (res != selectionList.end())
			{
				selectionList.erase(res);
			}
		}
		else
		{
			selectionList.push_back(obj);
			lastPickedObject = obj;
		}
	}
	else
	{
		if (!isSelected)
		{
			selectionList.clear();
			selectionList.push_back(obj);
		}
		lastPickedObject = obj;
	}
}

void Editor::SceneGraphInspector()
{
	ImGui::Begin("Scene Graph", &showSceneGraphInspector);
	ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable;
	if (ImGui::BeginTabBar("Objects", tab_bar_flags))
	{
		ImGui::PushID("SceneGraph");
		if (ImGui::BeginTabItem("Scene Graph"))
		{
			if (ImGui::Button("Save Scane"))
			{
				if (lastPickedObject != nullptr)
				{
					lua_State* L = luaL_newstate();
					luaL_openlibs(L);
					LuaTools::dofile(L, "resources/serialization/scene_saver.lua");
					lua_getglobal(L, "SaveScene");
					if (lua_isfunction(L, -1))
					{
						lua_pushlightuserdata(L, lastPickedObject);
						int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
						lua_pop(L, 1);
					}
					lua_close(L);
				}
			}
			
			ImGui::Text("Objects under SceneGraph Root");
			GenerateSceneGraphChildren(&SceneGraph::Instance()->SceneRoot);
			ImGui::Separator();

			std::vector<Object*> orphanedObjectsWithParentAsThemselves;
			std::vector<Object*> noParentObjects;
			std::vector<Object*> noTransformObjects;
			for (auto& obj : *GraphicsStorage::assetRegistry.GetPool<Object>())
			{
				if (obj.node != nullptr)
				{
					if (obj.node->parent == obj.node)
					{
						orphanedObjectsWithParentAsThemselves.push_back(&obj);
					}
					else if (obj.node->parent == nullptr)
					{
						noParentObjects.push_back(&obj);
					}
				}
				else
				{
					noTransformObjects.push_back(&obj);
				}
			}
			ImGui::Text("Orphaned Nodes with Parent as Themselves");
			for (auto obj : orphanedObjectsWithParentAsThemselves)
			{
				DrawSceneGraphNode(obj->node);
			}
			ImGui::Separator();
			ImGui::Text("No Parent Objects");
			for (auto obj : noParentObjects)
			{
				DrawSceneGraphNode(obj->node);
			}
			ImGui::Separator();
			ImGui::Text("No Transform Objects");
			int i = 0;
			for (auto obj : noTransformObjects)
			{
				auto res = std::find(selectionList.begin(), selectionList.end(), obj);
				std::string selectableName = std::format("{}##{}", obj->name, i);
				const bool isSelected = res != selectionList.end();
				bool nodeOpened = ImGui::Selectable(selectableName.c_str(), isSelected);
				if (ImGui::IsItemClicked())
				{
					AddObjectToSceneGrapghSelection(obj, isSelected);
				}
				i++;
			}
			ImGui::EndTabItem();
		}
		ImGui::PopID();
		if (ImGui::BeginTabItem("Objects in Frustum"))
		{
			int i = 0;
			for (auto& obj : SceneGraph::Instance()->objectsInFrustum)
			{
				std::string selectableName = std::format("{}##{}", obj->name, i);
				if (ImGui::Selectable(selectableName.c_str(), lastPickedObject == obj))
				{
					lastPickedObject = obj;
				}
				i++;
			}
			ImGui::EndTabItem();
		}

		if (UnParentAction)
		{
			UnParentSceneGraphSelection(NewUnparentParent);
			UnParentAction = false;
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}

void Editor::GenerateSceneGraphChildren(Node* node)
{
	for (auto& child : node->children)
	{
		DrawSceneGraphNode(child);
	}
}

void Editor::DrawSceneGraphNode(Node* child)
{
	auto res = std::find(selectionList.begin(), selectionList.end(), child->object);
	bool isSelected = res != selectionList.end();
	ImGuiTreeNodeFlags flags = ((isSelected) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | (child->children.size() == 0 ? ImGuiTreeNodeFlags_Leaf : 0);
	bool nodeOpened = ImGui::TreeNodeEx(std::format("{}##{}", child->object->name, child->object->ID).c_str(), flags);
	if (ImGui::IsItemClicked())
	{
		AddObjectToSceneGrapghSelection(child->object, isSelected);
	}

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		ImGui::SetDragDropPayload("SCENEGRAPH_SELECTION", child->object, sizeof(Object*));
		for (auto obj : selectionList)
		{
			std::string delimiter = "##";
			std::string objectName = obj->name.substr(0, obj->name.find(delimiter));
			ImGui::Text("%s", objectName.c_str());
		}
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENEGRAPH_SELECTION"))
		{
			IM_ASSERT(payload->DataSize == sizeof(Object*));
			//Object* payload_n = (Object*)payload->Data;
			selectionList.push_back(child->object);
			ParentSceneGraphSelection();
		}
		ImGui::EndDragDropTarget();
	}

	if (ImGui::BeginPopupContextItem())
	{
		AddObjectToSceneGrapghSelection(child->object, isSelected);
		if (ImGui::MenuItem("Unparent To Scene Root", NULL)) { UnParentAction = true; NewUnparentParent = &SceneGraph::Instance()->SceneRoot; }
		if (ImGui::MenuItem("Unparent from SceneGraph", NULL)) { UnParentAction = true; NewUnparentParent = nullptr; }
		ImGui::EndPopup();
	}

	if (nodeOpened)
	{
		GenerateSceneGraphChildren(child);
		ImGui::TreePop();
	}
}

void Editor::DatabaseInspector() 
{
	ImGui::Begin("Database", &showDatabaseInspector);
	ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable;
	if (ImGui::BeginTabBar("Assets", tab_bar_flags))
	{
/*done*/if (ImGui::BeginTabItem("Objects"))
		{
			ObjectsInspector();
			ImGui::EndTabItem();
		}
/*done*/if (ImGui::BeginTabItem("Materials"))
		{
			MaterialsInspector();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("OBJ"))
		{
			OBJsInspector();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("VAO"))
		{
			VAOsInspector();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("VBO"))
		{
			VBOsInspector();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Buffer Definitions"))
		{
			BuffersDefinitionsInspector();
			ImGui::EndTabItem();
		}
/*done*/if (ImGui::BeginTabItem("Textures"))
		{
			TexturesInspector();
			ImGui::EndTabItem();
		}
/*done*/if (ImGui::BeginTabItem("Render Targets"))
		{
			RenderTargetsInspector();
			ImGui::EndTabItem();
		}
/*done*/if (ImGui::BeginTabItem("CubeMaps"))
		{
			CubeMapsInspector();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("All Textures"))
		{
			AllTexturesInMemoryInspector();
			ImGui::EndTabItem();
		}
/*done*/if (ImGui::BeginTabItem("RenderBuffers"))
		{
			RenderBuffersInspector();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Frame Buffers"))
		{
			FrameBuffersInspector();
			ImGui::EndTabItem();
		}
/*done*/if (ImGui::BeginTabItem("RenderPasses"))
		{
			RenderPassesInspector();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("RenderQueue"))
		{
			RenderQueueInspector();
			ImGui::EndTabItem();
		}
/*done*/if (ImGui::BeginTabItem("Shaders"))
		{
			ShadersInspector();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Uniform Buffers"))
		{
			if (ImGui::BeginTabBar("Uniform Buffers Tab Bar", tab_bar_flags))
			{
				ShaderBlockInspector();
				ImGui::EndTabBar();
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Material Elements"))
		{
			if (ImGui::BeginTabBar("Material Elements Tab Bar", tab_bar_flags))
			{
/*done*/		if (ImGui::BeginTabItem("Material Profiles"))
				{
					MaterialProfilesInspector();
					ImGui::EndTabItem();
				}
/*done*/		if (ImGui::BeginTabItem("Texture Profiles"))
				{
					TextureProfilesInspector();
					ImGui::EndTabItem();
				}
/*done*/		if (ImGui::BeginTabItem("Render Profiles"))
				{
					RenderProfilesInspector();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Object Profiles"))
				{
					ObjectProfilesInspector();
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
	ImGui::End();
}

void Editor::AssetImport(const char** paths, int count)
{
	std::set<std::string> meshTypes{ ".obj" };
	std::set<std::string> textureTypes{ ".dds", ".png", ".hdr", ".jpg", ".bmp", ".pvr", ".tga", ".pkm" };
	for (int i = 0; i < count; i++)
	{
		auto relativePath = (std::filesystem::relative(paths[i], std::filesystem::current_path()));
		if (meshTypes.find(relativePath.extension().string()) != meshTypes.end())
		{
			//we have to show ui
			//where you select the configuration for each asset
			//auto t = std::thread(GraphicsManager::LoadOBJ, &objs, relativePath.string());
			//t.detach();
			std::string relative = relativePath.string();
			std::replace(relative.begin(), relative.end(), '\\', '/');
			pathsAndConfigurations[relative] = "";
		}
		if (textureTypes.find(relativePath.extension().string()) != textureTypes.end())
		{
			std::string relative = relativePath.string();
			std::replace(relative.begin(), relative.end(), '\\', '/');
			pathsAndConfigurations[relative] = "";
		}
	}
	if (pathsAndConfigurations.size() > 0)
	{
		showImportWindow = true;
	}
}

void Editor::LoadAndSaveMeshes()
{
	std::scoped_lock<std::mutex> lock(createVaoMutex);
	std::vector<OBJ*> meshesToExport;
	for (auto& obj : importedObjs)
	{
		auto saveMeshDataThread = std::thread(&Editor::SaveMeshData, this, obj);
		saveMeshDataThread.detach();
		//SaveMeshData(pathObj.second);
		meshesToExport.push_back(obj);
	}
	importedObjs.clear();
	for (auto& obj : meshesToExport)
	{
		std::string meshPath = "resources/meshes/" + obj->name + ".lua";
		std::string configPath = pathsAndConfigurations[obj->path];
		CreateAndSaveVao(obj, configPath, meshPath);
		pathsAndConfigurations.erase(obj->path);
		//auto createAndSaveVaoThread = std::thread(&Editor::CreateAndSaveVao, this, pathObj.second, configPath, meshPath);
		//createAndSaveVaoThread.detach();
	}
	//saving could be done multithreaded but I would really need the created vao
	//maybe lua after creating vao can call c++ multithreaded function to save it?	
}

void Editor::SaveMeshData(OBJ* obj)
{
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	std::string scriptPath = "resources/serialization/mesh_data_saver.lua";
	LuaTools::dofile(L, scriptPath.c_str());
	lua_getglobal(L, "SaveMeshData");
	if (lua_isfunction(L, -1))
	{
		lua_pushlightuserdata(L, obj);
		int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
		lua_pop(L, 1);
	}
	lua_close(L);
}

void Editor::CreateAndSaveVao(OBJ* obj, std::string configPath, std::string meshPath)
{
	static Script vaoCreatorScript("resources/serialization/vao_creator.lua");
	if (vaoCreatorScript.GetFunction("CreateAndSaveVao"))
	{
		lua_pushlightuserdata(vaoCreatorScript.L, obj);
		lua_pushstring(vaoCreatorScript.L, configPath.c_str());
		lua_pushstring(vaoCreatorScript.L, meshPath.c_str());
		int result = LuaTools::report(vaoCreatorScript.L, LuaTools::docall(vaoCreatorScript.L, 3, 1));
	}
}

void Editor::LoadOBJSToVaos(std::unordered_map<std::string, OBJ*>& objs)
{
	std::scoped_lock<std::mutex> lock(meshLoadMutex);
	for (auto& objp : objs)
	{
		printf("\nloading vao %s", objp.first.c_str());
		VertexArray* newVao = GraphicsStorage::assetRegistry.AllocAsset<VertexArray>();
		newVao->name = objp.first;
		GraphicsManager::LoadOBJToVAO(objp.second, newVao);
	}
	objs.clear();
}

void Editor::ImportUI()
{
	//TODO make generic importer
	ImGui::Begin("Import Assets", &showImportWindow);
	std::vector<std::string> vaoConfigNames;
	GraphicsManager::GetFileNames(vaoConfigNames, "resources/vao_configurations/", ".json");
	static std::vector<std::string> defaultConfigs;
	if (vaoConfigNames.size() < 1)
	{
		ImGui::Text("Could not find any mesh configs");
	}
	else
	{
		if (defaultConfigs.size() != pathsAndConfigurations.size() && vaoConfigNames.size() > 0)
		{
			defaultConfigs.resize(pathsAndConfigurations.size(), vaoConfigNames.front());
			for (auto& pathConfigPair : pathsAndConfigurations)
			{
				if (pathConfigPair.second.empty())
				{
					pathConfigPair.second = vaoConfigNames.front();
				}
			}
		}
		int i = 0;
		for (auto& pathConfigPair : pathsAndConfigurations)
		{
			ImGui::Text(pathConfigPair.first.c_str());
			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetFontSize() * -7.f);
			ImGui::PushID(i);
			if (ImGui::BeginCombo("Config", defaultConfigs[i].c_str(), ImGuiComboFlags_NoArrowButton))
			{
				int typeIndex = 0;
				for (auto& vaoConfigName : vaoConfigNames)
				{
					const bool is_selected = (strcmp(pathConfigPair.second.c_str(), vaoConfigName.c_str()) == 0);
					if (ImGui::Selectable(vaoConfigName.c_str(), is_selected))
					{
						pathConfigPair.second = vaoConfigName;
						defaultConfigs[i] = vaoConfigName;
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
					typeIndex++;
				}
				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();
			ImGui::PopID();
			i++;
		}
		if (ImGui::Button("Import"))
		{
			std::vector<std::string> configs;
			for (auto& pathAndConfig : pathsAndConfigurations)
			{
				pathAndConfig.second = std::format("resources/vao_configurations/{}.json", pathAndConfig.second);
				auto t = std::thread(GraphicsManager::LoadOBJ, &importedObjs, pathAndConfig.first);
				t.detach();
			}
		}
		if (ImGui::Button("Cancel"))
		{
			pathsAndConfigurations.clear();
			showImportWindow = false;
		}
		ImGui::End();
	}
}

void Editor::ObjectsInspector()
{
	// Left
	static Object* selectedObject = nullptr;
	bool newSelection = false;
	{
		ImGui::BeginChild("left pane", ImVec2(150, 0), true);

		static ImGuiTextFilter filterAvailableObjects;
		filterAvailableObjects.Draw("##filterAvailableObjectsFilter", -1.0f);
		std::map<std::string, Object*> objectsSorted;
		int i = 0;
		for (auto& asset : *GraphicsStorage::assetRegistry.GetPool<Object>())
		{
			objectsSorted.emplace(std::make_pair(asset.name + "##" + std::to_string(i), &asset));
			i++;
		}
		if (ImGui::BeginListBox("##filterable_objs", ImVec2(-1, -1)))
		{
			for (auto& [name, obj] : objectsSorted)
			{
				if (filterAvailableObjects.PassFilter(name.c_str()))
				{
					if (!obj->path.empty())
					{
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
					}
					else
					{
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
					}
					if (ImGui::Selectable(name.c_str(), selectedObject == obj))
					{
						selectedObject = obj;
						newSelection = true;
					}
					ImGui::PopStyleColor();
				}
			}
			ImGui::EndListBox();
		}
		ImGui::EndChild();
	}
	ImGui::SameLine();

	// Right
	{
		ImGui::BeginGroup();
		static char name[128];
		if (CreateNewAssetUI(name))
		{
			selectedObject = CreateNewObject(name);
			newSelection = true;
		}
		if (selectedObject != nullptr)
		{
			ImGui::Text(selectedObject->name.c_str());
			ImGui::Separator();
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
			ObjectInspector(selectedObject, newSelection);
			ImGui::EndChild();
		}
		ImGui::EndGroup();
	}
}

void Editor::MaterialsInspector()
{
	// Left
	static Material* selectedMaterial = nullptr;
	{
		ImGui::BeginChild("left pane", ImVec2(150, 0), true);

		static ImGuiTextFilter filterAvailableMaterials;
		filterAvailableMaterials.Draw("##filterAvailableMaterialsFilter", -1.0f);
		std::map<std::string, Material*> materialsSorted;
		int i = 0;
		for (auto& asset : *GraphicsStorage::assetRegistry.GetPool<Material>())
		{
			materialsSorted.emplace(std::make_pair(asset.name+"##"+std::to_string(i), &asset));
			i++;
		}
		if (ImGui::BeginListBox("##filterable_materials", ImVec2(-1, -1)))
		{
			for (auto& [name, material] : materialsSorted)
			{
				if (filterAvailableMaterials.PassFilter(name.c_str()))
				{
					if (!material->path.empty())
					{
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
					}
					else
					{
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
					}
					if (ImGui::Selectable(name.c_str(), selectedMaterial == material))
					{
						selectedMaterial = material;
					}
					ImGui::PopStyleColor();
				}
			}
			ImGui::EndListBox();
		}
		ImGui::EndChild();
	}
	ImGui::SameLine();

	// Right
	{
		ImGui::BeginGroup();
	
		bool newSelection = false;
		static char name[128];
		ImGui::PushID("new");
		if (CreateNewAssetUI(name))
		{
			selectedMaterial = CreateNewMaterial(name);
		}
		ImGui::PopID();
		if (ImGui::Button("Load Material"))
		{
			ImGui::OpenPopup("ListOfMaterials");
		}
		ImGui::Separator();
		if (selectedMaterial != nullptr)
		{
			std::vector<Material*> materialsToUnload;
			ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("ListOfMaterials", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Select Material To Load");
				static ImGuiTextFilter filterAvailableMaterialsOnDisk;
				filterAvailableMaterialsOnDisk.Draw("##filterAvailableMaterialsOnDiskFilter", -1.0f);
				std::vector<std::string> assetNames;
				GraphicsManager::GetFileNames(assetNames, "resources/materials/", ".json");
				static std::string selectedMaterialToLoad = "";
				if (ImGui::BeginListBox("##filterable_materialsOnDisk", ImVec2(-1, 0)))
				{
					for (auto& sname : assetNames)
					{
						if (filterAvailableMaterialsOnDisk.PassFilter(sname.c_str()))
						{
							if (ImGui::Selectable(sname.c_str(), selectedMaterialToLoad == sname))
							{
								selectedMaterialToLoad = sname;
							}
						}
					}
					ImGui::EndListBox();
				}
				ImGui::Separator();
				if (ImGui::Button("Load", ImVec2(120, 0)))
				{
					if (!selectedMaterialToLoad.empty())
					{
						std::string assetPath = std::format("resources/materials/{}.json", selectedMaterialToLoad);
						lua_State* L = luaL_newstate();
						luaL_openlibs(L);
						LuaTools::dofile(L, "resources/luaexts/loadAndUnloadAssetFromList.lua");
						lua_getglobal(L, "LoadIntoList");
						if (lua_isfunction(L, -1))
						{
							lua_pushstring(L, assetPath.c_str());
							lua_pushstring(L, "resources/materials_setup/materials.json");
							int result = LuaTools::report(L, LuaTools::docall(L, 2, 1));
							lua_pop(L, 1);
						}
						lua_close(L);

						L = luaL_newstate();
						luaL_openlibs(L);
						LuaTools::dofile(L, "resources/serialization/material_loader.lua");
						lua_getglobal(L, "LoadMaterial");
						if (lua_isfunction(L, -1))
						{
							lua_pushstring(L, assetPath.c_str());
							int result = LuaTools::report(L, LuaTools::docall(L, 2, 1));
							lua_pop(L, 1);
						}
						lua_close(L);

						newSelection = true;
						//would be nice to select newly loaded material
					}
					ImGui::CloseCurrentPopup();
				}
				ImGui::SetItemDefaultFocus();
				ImGui::SameLine();
				if (ImGui::Button("Close", ImVec2(120, 0)))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
			if (selectedMaterial != nullptr)
			{
				ImGui::AlignTextToFramePadding();
				ImGui::Text(selectedMaterial->name.c_str());
				if (ImGui::Button("Unload Material"))
				{
					materialsToUnload.push_back(selectedMaterial);
				}
				ImGui::Separator();
			}

			for (auto mat : materialsToUnload)
			{
				std::string materialName = mat->name;
				for (auto& object : *GraphicsStorage::assetRegistry.GetPool<Object>())
				{
					object.RemoveMaterial(mat);
				}

				std::string assetPath = std::format("resources/materials/{}.json", materialName);

				lua_State* L = luaL_newstate();
				luaL_openlibs(L);
				LuaTools::dofile(L, "resources/luaexts/loadAndUnloadAssetFromList.lua");
				lua_getglobal(L, "UnloadFromList");
				if (lua_isfunction(L, -1))
				{
					lua_pushstring(L, assetPath.c_str());
					lua_pushstring(L, "resources/materials_setup/materials.json");
					int result = LuaTools::report(L, LuaTools::docall(L, 2, 1));
					lua_pop(L, 1);
				}
				lua_close(L);
				selectedMaterial = nullptr;
			}
			static char dupname[128];
			ImGui::PushID("copy");
			if (CreateNewAssetUI(dupname, true))
			{
				Material* newMat = CreateNewMaterial(dupname);
				Material* prevMat = selectedMaterial;
				newMat->elements = prevMat->elements;
				selectedMaterial = newMat;
			}
			ImGui::PopID();
			if (selectedMaterial != nullptr)
			{
				ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
				ImGui::Text("Elements:");
				GenerateMaterialElementsUI(selectedMaterial);
				ImGui::EndChild();
			}
		}
		ImGui::EndGroup();
		
		if (ImGui::Button("Revert")) {}
		ImGui::SameLine();
		if (ImGui::Button("Save"))
		{
			if (selectedMaterial != nullptr)
			{
				selectedMaterial->path = std::format("resources/materials/{}.json", selectedMaterial->name);
				lua_State* L = luaL_newstate();
				luaL_openlibs(L);
				LuaTools::dofile(L, "resources/serialization/material_saver.lua");
				lua_getglobal(L, "SaveMaterial");
				if (lua_isfunction(L, -1))
				{
					lua_pushlightuserdata(L, selectedMaterial);
					int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
					lua_pop(L, 1);
				}
				lua_close(L);
			}
		}
	}
}

void Editor::OBJsInspector()
{
	// Left
	static OBJ* selectedOBJ = nullptr;
	{
		ImGui::BeginChild("left pane", ImVec2(150, 0), true);

		static ImGuiTextFilter filterAvailableObjs;
		filterAvailableObjs.Draw("##filterAvailableObjsFilter", -1.0f);
		std::map<std::string, OBJ*> objsSorted;
		int i = 0;
		for (auto& asset : *GraphicsStorage::assetRegistry.GetPool<OBJ>())
		{
			objsSorted.emplace(std::make_pair(asset.name + "##" + std::to_string(i), &asset));
			i++;
		}
		if (ImGui::BeginListBox("##filterable_vaos", ImVec2(-1, -1)))
		{
			for (auto& [name, obj] : objsSorted)
			{
				if (filterAvailableObjs.PassFilter(name.c_str()))
				{
					if (ImGui::Selectable(name.c_str(), selectedOBJ == obj))
					{
						selectedOBJ = obj;
					}
				}
			}
			ImGui::EndListBox();
		}
		ImGui::EndChild();
	}
	ImGui::SameLine();

	// Right
	{
		ImGui::BeginGroup();
		if (selectedOBJ != nullptr)
		{
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
			ImGui::Text(selectedOBJ->name.c_str());
			ImGui::Separator();
			ImGui::EndChild();
		}
		ImGui::EndGroup();
		
		if (ImGui::Button("Revert")) {}
		ImGui::SameLine();
		if (ImGui::Button("Save")) {
			if (selectedOBJ != nullptr)
			{
				lua_State* L = luaL_newstate();
				luaL_openlibs(L);
				LuaTools::dofile(L, "resources/serialization/mesh_data_saver.lua");
				lua_getglobal(L, "SaveMeshData");
				if (lua_isfunction(L, -1))
				{
					lua_pushlightuserdata(L, selectedOBJ);
					int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
					lua_pop(L, 1);
				}
				lua_close(L);
			}
		}
	}
}

void Editor::VAOsInspector()
{
	// Left
	static VertexArray* selectedVao = nullptr;
	std::vector<VertexArray*> modelsToUnload;
	bool newSelection = false;
	{
		ImGui::BeginChild("left pane", ImVec2(150, 0), true);

		static ImGuiTextFilter filterAvailableVaos;
		filterAvailableVaos.Draw("##filterAvailableVaosFilter", -1.0f);
		std::map<std::string, VertexArray*> vaosSorted;
		int i = 0;
		for (auto& asset : *GraphicsStorage::assetRegistry.GetPool<VertexArray>())
		{
			vaosSorted.emplace(std::make_pair(asset.name+"##" + std::to_string(i), &asset));
			i++;
		}
		if (ImGui::BeginListBox("##filterable_vaos", ImVec2(-1, -1)))
		{
			for (auto& [name, vao] : vaosSorted)
			{
				if (!vao->path.empty())
				{
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
				}
				else
				{
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
				}
				if (filterAvailableVaos.PassFilter(name.c_str()))
				{
					if (ImGui::Selectable(name.c_str(), selectedVao == vao))
					{
						selectedVao = vao;
					}
				}
				ImGui::PopStyleColor();
			}
			ImGui::EndListBox();
		}
		ImGui::EndChild();
	}
	ImGui::SameLine();

	// Right
	{
		ImGui::BeginGroup();
		

		if (ImGui::Button("Load Vao"))
		{
			ImGui::OpenPopup("ListOfVaos");
		}
		ImGui::Separator();
		ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("ListOfVaos", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Select Vao To Load");
			static ImGuiTextFilter filterAvailableVaosOnDisk;
			filterAvailableVaosOnDisk.Draw("##filterAvailableVosOnDiskFilter", -1.0f);
			static std::filesystem::directory_entry selectedVaoToLoad;
			if (ImGui::BeginListBox("##filterable_vaosOnDisk", ImVec2(-1, 0)))
			{
				for (auto& entry : std::filesystem::recursive_directory_iterator("resources/vaos"))
				{
					if (filterAvailableVaosOnDisk.PassFilter(entry.path().string().c_str()))
					{
						if (entry.is_regular_file())
						{
							if (ImGui::Selectable(entry.path().string().c_str(), selectedVaoToLoad == entry))
							{
								selectedVaoToLoad = entry;
							}
						}
					}
				}
				ImGui::EndListBox();
			}
			ImGui::Separator();
			if (ImGui::Button("Load", ImVec2(120, 0)))
			{
				if (selectedVaoToLoad.exists())
				{
					std::string modelPath = selectedVaoToLoad.path().string();
					std::replace(modelPath.begin(), modelPath.end(), '\\', '/');

					lua_State* L = luaL_newstate();
					luaL_openlibs(L);
					LuaTools::dofile(L, "resources/serialization/vao_loader.lua");
					lua_getglobal(L, "LoadVertexArray"); 
					if (lua_isfunction(L, -1))
					{
						lua_pushstring(L, modelPath.c_str());
						int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
						lua_pop(L, 1);
					}
					lua_close(L);

					newSelection = true;
					//this was cool after selecting asset to load it would make that one the new selection just based on the name
					//for that we really need the name to be the new selection but name is not going to work unless it's 100% unique
					//selectedVao = selectedVaoToLoad;

					//i will have the path once we do the proper open file dialog then selected file will have the path
					//now having path I will have to push it into the loadintolist, saving path instead of name is better, path is always going to be unique
					//std::string modelPath = std::string("resources/models/") + selectedVaoToLoad + ".obj";
					//GraphicsManager::AddPairToJson(selectedVaoToLoad, modelPath, std::string("resources/models.json"));


					//GraphicsManager::LoadOBJ(&GraphicsStorage::objs, modelPath);
					//GraphicsManager::LoadAllOBJsToVAOs();
					//newSelection = true;
					//selectedName = selectedVaoToLoad;
				}
				ImGui::CloseCurrentPopup();
			}
			
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Close", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		for (auto model : modelsToUnload)
		{
			std::string modelName = model->name;
			for (auto& mat : *GraphicsStorage::assetRegistry.GetPool<Material>())
			{
				if (mat.vao == model)
				{
					mat.AssignMesh(nullptr);
				}
			}
			//remove from the pool
			//delete GraphicsStorage::vaos[modelName];
			//GraphicsStorage::vaos.erase(modelName);

			std::string assetPath = std::string("resources/models/") + modelName + ".json";

			lua_State* L = luaL_newstate();
			luaL_openlibs(L);
			LuaTools::dofile(L, "resources/luaexts/loadAndUnloadAssetFromList.lua");
			lua_getglobal(L, "UnloadFromList");
			if (lua_isfunction(L, -1))
			{
				lua_pushstring(L, assetPath.c_str());
				lua_pushstring(L, "resources/models.json");
				int result = LuaTools::report(L, LuaTools::docall(L, 2, 1));
				lua_pop(L, 1);
			}
			lua_close(L);
			selectedVao = nullptr;

			//GraphicsManager::RemoveItemFromJson(modelName, std::string("resources/models.json"));
			//selectedName = "";
		}

		if (selectedVao != nullptr)
		{
			if (ImGui::Button("Unload Vao"))
			{
				modelsToUnload.push_back(selectedVao);
			}
			ImGui::AlignTextToFramePadding();
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
			ImGui::Text(selectedVao->name.c_str());
			ImGui::Text(std::format("Handle: {}", selectedVao->handle).c_str());
			ImGui::Text(std::format("Mesh: {}", selectedVao->meshPath).c_str());

			std::string defaultVaoConfig = std::filesystem::path(selectedVao->configPath).stem().string();
			if (std::filesystem::is_empty("resources/vao_configurations"))
			{
				ImGui::Text("Could not find any mesh configs");
			}
			else
			{
				ImGui::PushItemWidth(ImGui::GetFontSize() * -7.f);
				if (ImGui::BeginCombo("Config", defaultVaoConfig.c_str(), ImGuiComboFlags_NoArrowButton))
				{
					for (auto& dirEntry : std::filesystem::directory_iterator("resources/vao_configurations"))
					{
						if (dirEntry.is_regular_file())
						{
							std::string vaoConfigName = dirEntry.path().stem().string();
							const bool is_selected = (defaultVaoConfig.compare(vaoConfigName) == 0);
							if (ImGui::Selectable(vaoConfigName.data(), is_selected))
							{
								defaultVaoConfig = vaoConfigName;
								selectedVao->configPath = std::format("resources/vao_configurations/{}.json", defaultVaoConfig);
							}
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
				ImGui::PopItemWidth();
			}

			ImGui::Text(std::string("Config: " + selectedVao->configPath).c_str());
			ImGui::Text(std::string("Path: " + selectedVao->path).c_str());
			ImGui::Text("Vertex Buffers:");
			int i = 0;
			for (auto& vbo : selectedVao->vbos)
			{
				ImGui::Text((std::string("Handle: ") + std::to_string(selectedVao->vbos[i]->handle)).c_str());
				for (auto& location : vbo->layout.locations)
				{
					ImGui::BulletText(std::format("Location: {} {}", ShaderDataType::Str(location.type), location.name).c_str());
				}
				i++;
			}
			if (selectedVao->ebo != nullptr)
			{
				ImGui::Text(("Element Buffer Handle: " + std::to_string(selectedVao->ebo->handle)).c_str());
				//ImGui::BulletText(std::string("Guid: " + GraphicsStorage::assetRegistry.GetAssetIDAsString(GraphicsStorage::vaos[selectedName]->ebo)).c_str());
				ImGui::BulletText(std::string("Indices Count: " + std::to_string(selectedVao->ebo->indicesCount)).c_str());
				ImGui::BulletText(std::string("Indices Type: " + std::string(indicesTypes[selectedVao->ebo->indicesType])).c_str());
			}
			ImGui::EndChild();
		}
		ImGui::EndGroup();
		
		if (selectedVao != nullptr)
		{
			if (selectedVao->configPath.empty())
			{
				ImGui::Text("Configuration not selected!");
			}
			else
			{
				if (ImGui::Button("Revert")) {}
				ImGui::SameLine();
				if (ImGui::Button("Save"))
				{
					std::string nameconfig = selectedVao->name;
					std::replace(nameconfig.begin(), nameconfig.end(), '.', '/');
					selectedVao->path = std::format("resources/vaos/{}.json", nameconfig);
					lua_State* L = luaL_newstate();
					luaL_openlibs(L);
					LuaTools::dofile(L, "resources/serialization/vao_saver.lua");
					lua_getglobal(L, "SaveVertexArray");
					if (lua_isfunction(L, -1))
					{
						lua_pushlightuserdata(L, selectedVao);
						int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
						lua_pop(L, 1);
					}
					lua_close(L);
				}
			}
		}
	}
}

void Editor::VBOsInspector()
{
	// Left
	static VertexBuffer* selectedVbo = nullptr;
	{
		ImGui::BeginChild("left pane", ImVec2(150, 0), true);

		static ImGuiTextFilter filterAvailableVbos;
		filterAvailableVbos.Draw("##filterAvailableVbosFilter", -1.0f);
		std::map<std::string, VertexBuffer*> vbosSorted;
		int i = 0;
		for (auto& asset : *GraphicsStorage::assetRegistry.GetPool<VertexBuffer>())
		{
			vbosSorted.emplace(std::make_pair(asset.name + "##" + std::to_string(i), &asset));
			i++;
		}
		if (ImGui::BeginListBox("##filterable_vbos", ImVec2(-1, -1)))
		{
			for (auto& vbo : vbosSorted)
			{
				if (filterAvailableVbos.PassFilter(vbo.first.c_str()))
				{
					if (ImGui::Selectable(vbo.first.c_str(), selectedVbo == vbo.second))
					{
						selectedVbo = vbo.second;
					}
				}
			}
			ImGui::EndListBox();
		}
		ImGui::EndChild();
	}
	ImGui::SameLine();

	// Right
	{
		ImGui::BeginGroup();

		if (selectedVbo != nullptr)
		{
			ImGui::AlignTextToFramePadding();
			ImGui::Text(selectedVbo->name.c_str());
			ImGui::Separator();
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
			ImGui::Text(std::format("Handle: {}", selectedVbo->handle).c_str());
			for (auto& location : selectedVbo->layout.locations)
			{
				ImGui::BulletText(std::format("Location: {} {}", ShaderDataType::Str(location.type), location.name).c_str());
			}
			ImGui::EndChild();
		}
		ImGui::EndGroup();
		if (ImGui::Button("Revert")) {}
		ImGui::SameLine();
		if (ImGui::Button("Save")) {}
	}
}

void Editor::TexturesInspector()
{
	static std::string selectedName = "";
	bool newSelection = false;
	if (ImGui::Button("Load Texture"))
	{
		ImGui::OpenPopup("ListOfTextures");
	}
	ImGui::Separator();
	ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("ListOfTextures", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Select Texture To Load");
		static ImGuiTextFilter filterAvailableTexturesOnDisk;
		filterAvailableTexturesOnDisk.Draw("##filterAvailableTexturesOnDiskFilter", -1.0f);
		std::vector<std::string> textureNames;
		std::sort(textureNames.begin(), textureNames.end());

		GraphicsManager::GetFileNames(textureNames, "resources/texture_settings/");
		static std::filesystem::directory_entry selectedTextureToLoad;
		if (ImGui::BeginListBox("##filterable_texturesOnDisk", ImVec2(-1, 0)))
		{
			for (auto& entry : std::filesystem::recursive_directory_iterator("resources/texture_settings"))
			{
				if (entry.is_regular_file())
				{
					if (filterAvailableTexturesOnDisk.PassFilter(entry.path().string().c_str()))
					{
						if (ImGui::Selectable(entry.path().string().c_str(), selectedTextureToLoad == entry))
						{
							selectedTextureToLoad = entry;
						}
					}
				}
			}
			ImGui::EndListBox();
		}
		ImGui::Separator();
		if (ImGui::Button("Load", ImVec2(120, 0)))
		{
			if (selectedTextureToLoad.exists())
			{
				std::string texturePath = selectedTextureToLoad.path().string();
				std::replace(texturePath.begin(), texturePath.end(), '\\', '/');
				//while we can load texture with c++ just fine we should do this via lua
				//here we should use lua script to load texture
				//it would be great if we also added the texture to the editor textures container
				//std::string texturePath = std::format("resources/textures/{}.dds", selectedTextureToLoad);
				//GraphicsManager::AddPairToJson(selectedTextureToLoad, texturePath, std::string("resources/textures.json"));
				//this is for importing, not loading!
				//GraphicsManager::LoadTextureInfo(&GraphicsStorage::texturesToLoad, texturePath, 0);
				//GraphicsManager::LoadTexturesIntoGPU(GraphicsStorage::texturesToLoad);
				newSelection = true;
				//selectedName = selectedTextureToLoad;
			}
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Close", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	TexturesInspector(textures, selectedName);
}

void Editor::RenderTargetsInspector()
{
	{
		static std::string selectedName = "";
		static char name[128];
		if (CreateNewRenderTargetUI(name))
		{
			CreateNewRenderTarget(name);
			selectedName = name;
		}
		TexturesInspector(renderTargets, selectedName, true);
	}
}

void Editor::CubeMapsInspector()
{
	static std::string selectedName = "";
	TexturesInspector(cubemaps, selectedName);
}

void Editor::RenderBuffersInspector()
{
	// Left
	static RenderBuffer* selectedRenderBuffer = nullptr;
	{
		ImGui::BeginChild("left pane", ImVec2(150, 0), true);

		static ImGuiTextFilter filterAvailableRenderBuffers;
		filterAvailableRenderBuffers.Draw("##filterAvailableRenderBuffersFilter", -1.0f);
		std::map<std::string, RenderBuffer*> renderBuffersSorted;
		int i = 0;
		for (auto& asset : *GraphicsStorage::assetRegistry.GetPool<RenderBuffer>())
		{
			renderBuffersSorted.emplace(std::make_pair(asset.name + "##" + std::to_string(i), &asset));
			i++;
		}
		if (ImGui::BeginListBox("##filterable_renderBuffers", ImVec2(-1, -1)))
		{
			for (auto& [name, rbuffer] : renderBuffersSorted)
			{
				if (filterAvailableRenderBuffers.PassFilter(name.c_str()))
				{
					if (!rbuffer->path.empty())
					{
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
					}
					else
					{
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
					}
					if (ImGui::Selectable(name.c_str(), selectedRenderBuffer == rbuffer))
					{
						selectedRenderBuffer = rbuffer;
					}
					ImGui::PopStyleColor();
				}
			}
			ImGui::EndListBox();
		}
		ImGui::EndChild();
	}
	ImGui::SameLine();

	// Right
	{
		
			ImGui::BeginGroup();
			static char name[128];
			if (CreateNewRenderBufferUI(name))
			{
				selectedRenderBuffer = CreateNewRenderBuffer(name);
			}
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
			if (selectedRenderBuffer != nullptr)
			{
				ImGui::Text(selectedRenderBuffer->name.c_str());
				ImGui::Text(selectedRenderBuffer->path.c_str());
			}
			ImGui::Separator();
			
			if (selectedRenderBuffer != nullptr)
			{
				RenderBufferInspector(selectedRenderBuffer);
			}
			ImGui::EndChild();
			ImGui::EndGroup();		
	}
}

void Editor::FrameBuffersInspector()
{
	bool resetInspectors = false;
	// Left
	static FrameBuffer* selectedFrameBuffer = nullptr;
	{
		ImGui::BeginChild("left pane", ImVec2(150, 0), true);

		static ImGuiTextFilter filterAvailableMaterials;
		filterAvailableMaterials.Draw("##filterAvailableFbosFilter", -1.0f);
		std::map<std::string, FrameBuffer*> fbosSorted;
		int i = 0;
		for (auto& asset : *GraphicsStorage::assetRegistry.GetPool<FrameBuffer>())
		{
			fbosSorted.emplace(std::make_pair(asset.name + "##" + std::to_string(i), &asset));
			i++;
		}
		if (ImGui::BeginListBox("##filterable_fbos", ImVec2(-1, -1)))
		{
			for (auto& fbo : fbosSorted)
			{
				if (filterAvailableMaterials.PassFilter(fbo.first.c_str()))
				{
					if (!fbo.second->path.empty())
					{
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
					}
					else
					{
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
					}
					if (ImGui::Selectable(fbo.first.c_str(), selectedFrameBuffer == fbo.second))
					{
						selectedFrameBuffer = fbo.second;
						resetInspectors = true;
					}
					ImGui::PopStyleColor();
				}
			}
			ImGui::EndListBox();
		}
		ImGui::EndChild();
	}
	ImGui::SameLine();

	// Right
	{
		ImGui::BeginGroup();

		if (ImGui::Button("Load Frame Buffer"))
		{
			ImGui::OpenPopup("ListOfFrameBuffers");
		}
		ImGui::Separator();
		ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("ListOfFrameBuffers", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Select Frame Buffer To Load");
			static ImGuiTextFilter filterAvailableVaosOnDisk;
			filterAvailableVaosOnDisk.Draw("##filterAvailableFbosOnDiskFilter", -1.0f);
			static std::filesystem::directory_entry selectedAssetToLoad;
			if (ImGui::BeginListBox("##filterable_fbosOnDisk", ImVec2(-1, 0)))
			{
				for (auto& entry : std::filesystem::recursive_directory_iterator("resources/frame_buffers"))
				{
					if (filterAvailableVaosOnDisk.PassFilter(entry.path().string().c_str()))
					{
						if (entry.is_regular_file())
						{
							if (ImGui::Selectable(entry.path().string().c_str(), selectedAssetToLoad == entry))
							{
								selectedAssetToLoad = entry;
							}
						}
					}
				}
				ImGui::EndListBox();
			}
			ImGui::Separator();
			if (ImGui::Button("Load", ImVec2(120, 0)))
			{
				if (selectedAssetToLoad.exists())
				{
					std::string assetPath = selectedAssetToLoad.path().string();
					std::replace(assetPath.begin(), assetPath.end(), '\\', '/');

					lua_State* L = luaL_newstate();
					luaL_openlibs(L);
					LuaTools::dofile(L, "resources/serialization/frame_buffer_loader.lua");
					lua_getglobal(L, "LoadFrameBuffer");
					if (lua_isfunction(L, -1))
					{
						lua_pushstring(L, assetPath.c_str());
						int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
						lua_pop(L, 1);
					}
					lua_close(L);
				}
				ImGui::CloseCurrentPopup();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Close", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		static char name[128];
		if (CreateNewAssetUI(name))
		{
			selectedFrameBuffer = CreateNewFrameBuffer(name);
			resetInspectors = true;
		}
		if (selectedFrameBuffer != nullptr)
		{
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
			ImGui::Text(selectedFrameBuffer->name.c_str());
			ImGui::Text(selectedFrameBuffer->path.c_str());
			ImGui::Separator();
			FrameBufferInspector(selectedFrameBuffer, resetInspectors);
			ImGui::EndChild();
		}
		ImGui::EndGroup();
	}
}

void Editor::FrameBufferInspector(FrameBuffer* fbo, bool resetInspectors)
{
	ImGui::Text("%s", FBOManager::Instance()->IsDynamic(fbo) ? "Dynamic" : "Static");
	if (ImGui::Button("Set Dynamic")) {
		FBOManager::Instance()->MakeDynamic(fbo);
	}
	ImGui::SameLine();
	if (ImGui::Button("Set Static")) {
		FBOManager::Instance()->MakeStatic(fbo);
	}
	static int fboSize[2] = { 1024, 1024 };
	if (fbo->textures.size() > 0)
	{
		ImGui::Text(std::format("Current Size: Width:{}, Height:{}", fbo->textures[0]->width, fbo->textures[0]->height).c_str());

		if (ImGui::DragInt2("New Size", fboSize))
		{
			fbo->UpdateTextures(fboSize[0] = std::max(fboSize[0], 1), fboSize[1] = std::max(fboSize[1], 1));
		}
	}
	else
	{
		ImGui::Text("Current Size: Width: 0, Height: 0");
	}
	float fboScale[2];
	fboScale[0] = (float)fbo->scaleXFactor;
	fboScale[1] = (float)fbo->scaleYFactor;
	ImGui::Text(std::format("Current Scale: X:{}, Y:{}", fbo->scaleXFactor, fbo->scaleYFactor).c_str());
	if (ImGui::DragFloat2("New Scale", fboScale))
	{
		fbo->scaleXFactor = std::max(fboScale[0], 0.001f);
		fbo->scaleYFactor = std::max(fboScale[1], 0.001f);
		if (fbo->textures.size() > 0)
		{
			fbo->UpdateTextures(fboSize[0] = std::max(fboSize[0], 1), fboSize[1] = std::max(fboSize[1], 1));
		}
	}
	ImGui::Separator();
	bool flip = true;
	float flipCoords = 0.0f;
	if (flip)
		flipCoords = 1.0;
	ImVec2 uv_min = ImVec2(1.0f - flipCoords, 0.0f + flipCoords);                 // Top-left
	ImVec2 uv_max = ImVec2(0.0f + flipCoords, 1.0f - flipCoords);                 // Lower-right
	ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
	ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
	float imageSize = 128;
	ImVec2 size = ImVec2(imageSize, imageSize);

	ImGuiStyle& style = ImGui::GetStyle();
	int buttons_count = 20000;
	float visibleWindowWidth = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
	float groupSize = imageSize + 2;
	ImGui::PushItemWidth(groupSize);
	int borderWidth = 2;
	float comboImageWidth = 50;
	ImVec2 comboImageSize = ImVec2(comboImageWidth, comboImageWidth);

	std::string popUpLabel = "add texture attachment popup";
	std::string popUpLabel2 = "add renerbuffer attachment popup";

	static Texture* textureToAdd = nullptr;
	static RenderBuffer* rbufferToAdd = nullptr;

	ImGui::Text("Add Texture:");
	if (DrawSelectableTextureThumbnail(textureToAdd, comboImageSize, uv_min, uv_max, borderWidth, border_col, tint_col))
	{
		ImGui::OpenPopup(popUpLabel.c_str());
	}
	ImGui::SameLine();
	if (ImGui::Button("Add##addTextureButton"))
	{
		if (textureToAdd != nullptr)
		{
			fbo->RegisterTexture(textureToAdd);
		}
	}
	ImGui::Separator();
	/////////////////////////////////////////////

	ImVec2 button_min = ImGui::GetItemRectMin();
	ImVec2 button_max = ImGui::GetItemRectMax();

	ImVec2 popupPosition;
	popupPosition.x = button_min.x;
	popupPosition.y = button_min.y;

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::SetNextWindowPos(popupPosition, ImGuiCond_Appearing);
	if (ImGui::BeginPopup(popUpLabel.c_str(), flags))
	{
		//ImGui::PushItemWidth(calculatedWidth);
		//float offset = 0.f;
		static ImGuiTextFilter texturesfilter;
		ImGui::PushItemWidth(-1);
		texturesfilter.Draw("##textureSearchFilter");
		ImGui::PopItemWidth();
		//bool isFilterInFocus = ImGui::IsItemActive();

		DrawTexturesThumbnailsWithLabel(texturesfilter, &textureToAdd, renderTargets, visibleWindowWidth);

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);
	ImGui::Separator();

	button_min = ImGui::GetItemRectMin();
	button_max = ImGui::GetItemRectMax();
	popupPosition.x = button_min.x;
	popupPosition.y = button_min.y;

	ImGui::Text("Add Render Buffer:");
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, borderWidth);
	ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(200, 68, 0, 255));
	ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 255));
	ImGui::PushID("RenderBuffers");
	if (DrawSelectableTextureThumbnail(rbufferToAdd, comboImageSize, uv_min, uv_max, borderWidth, border_col, tint_col))
	{
		ImGui::OpenPopup(popUpLabel2.c_str());
	}
	
	ImGui::PopStyleVar(1);
	ImGui::PopStyleColor(2);
	ImGui::SameLine();
	if (ImGui::Button("Add##addRenderBufferButton"))
	{
		if (rbufferToAdd != nullptr)
		{
			fbo->RegisterRenderBuffer(rbufferToAdd);
			//std::sort(fbo->renderBuffers.begin(), fbo->renderBuffers.end());
		}
	}
	ImGui::Separator();

	ImVec2 button_min2 = ImGui::GetItemRectMin();
	ImVec2 button_max2 = ImGui::GetItemRectMax();
	ImVec2 popupPosition2;
	popupPosition2.x = button_min2.x;
	popupPosition2.y = button_min2.y;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::SetNextWindowPos(popupPosition2, ImGuiCond_Appearing);
	if (ImGui::BeginPopup(popUpLabel2.c_str(), flags))
	{
		//ImGui::PushItemWidth(calculatedWidth);
		//float offset = 0.f;
		static ImGuiTextFilter renderBuffersfilter;
		ImGui::PushItemWidth(-1);
		renderBuffersfilter.Draw("##renderBuffersSearchFilter");
		ImGui::PopItemWidth();
		//bool isFilterInFocus = ImGui::IsItemActive();

		DrawTexturesThumbnailsWithLabel(renderBuffersfilter, &rbufferToAdd, renderBuffers, visibleWindowWidth);

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);
	ImGui::PopID();
	float prevItemSize = ImGui::CalcItemWidth();
	float windowWidth = ImGui::GetWindowWidth();
	float windowHeight = ImGui::GetWindowHeight();
	static Texture* selectedTexture = nullptr;
	ImGui::BeginChild("fbo textures", ImVec2(windowWidth * 0.6f, windowHeight * 0.4f), true);
	float window_visible_x_child = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
	ImGui::AlignTextToFramePadding();
	int i = 0;
	std::vector<Texture*> texturesToUnregister;
	std::vector<Texture*> texturesToRegister;
	for (auto texture : fbo->textures)
	{
		ImGui::BeginGroup();
		if (ImGui::Button(std::format("X##removeTexture{}", i).c_str()))
		{
			fbo->UnregisterTexture(texture);
		}
		else
		{
			if (DrawSelectableTextureThumbnail(texture, size, uv_min, uv_max, borderWidth, border_col, tint_col))
			{
				selectedTexture = texture;
			}
			//ImGui::PushItemWidth(groupSize + borderWidth);
			ImGui::PushItemWidth(groupSize);
			std::string popUpLabel = std::format("opfitex{}", i);
			float calculatedWidth = ImGui::CalcItemWidth();
			if (ImGui::Button(std::format("{}##TexturesToSet{}", texture->name, i).c_str(), ImVec2(calculatedWidth, 0.f)))
			{
				ImGui::OpenPopup(popUpLabel.c_str());
			}

			ImVec2 button_min = ImGui::GetItemRectMin();
			ImVec2 button_max = ImGui::GetItemRectMax();

			ImVec2 popupPosition;
			popupPosition.x = button_min.x;
			popupPosition.y = button_min.y;

			ImGuiWindowFlags flags =
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoSavedSettings;
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::SetNextWindowPos(popupPosition, ImGuiCond_Appearing);
			if (ImGui::BeginPopup(popUpLabel.c_str(), flags))
			{
				ImGui::PushItemWidth(calculatedWidth);
				std::string filterLabel = "##searchTextures" + texture->name;
				static ImGuiTextFilter filter;
				ImGui::PushItemWidth(-1);
				filter.Draw(filterLabel.c_str());
				ImGui::PopItemWidth();
				bool isFilterInFocus = ImGui::IsItemActive();
				Texture* textureToRegister = nullptr;
				if (DrawTexturesThumbnailsWithLabel(filter, &textureToRegister, renderTargets, visibleWindowWidth))
				{
					texturesToUnregister.push_back(texture);
					texturesToRegister.push_back(textureToRegister);
				}
				ImGui::PopItemWidth();
				ImGui::EndPopup();
			}
			ImGui::PopStyleVar(2);
			ImGui::PopItemWidth();
		}
		
		float lastItem_x2 = ImGui::GetItemRectMax().x;
		float nextItem_x2 = lastItem_x2 + style.ItemSpacing.x + size.x;
		ImGui::EndGroup();
		if (nextItem_x2 < window_visible_x_child)
			ImGui::SameLine();
		else
			ImGui::AlignTextToFramePadding();
		i++;
	}
	ImGui::EndChild();
	for (auto tex : texturesToUnregister)
	{
		fbo->UnregisterTexture(tex);
	}
	for (auto tex : texturesToRegister)
	{
		fbo->RegisterTexture(tex);
	}
	
	ImGui::SameLine();
	if (resetInspectors)
	{
		selectedTexture = nullptr;
	}
	// Right
	{
		ImGui::BeginGroup();
		ImGui::BeginChild("left pane details", ImVec2(windowWidth * 0.4f, windowHeight * 0.4f));
		TextureBasicDetails(selectedTexture);
		ImGui::EndChild();
		ImGui::EndGroup();
	}


	static RenderBuffer* selectedRBuffer = nullptr;
	ImGui::BeginChild("fbo render buffers", ImVec2(windowWidth * 0.6f, windowHeight * 0.4f), true);
	ImGui::AlignTextToFramePadding();
	i = 0;
	std::vector<RenderBuffer*> buffersToRegister;
	std::vector<RenderBuffer*> buffersToUnregister;
	for (auto texture : fbo->renderBuffers)
	{
		ImGui::BeginGroup();
		if (ImGui::Button(std::format("X##removeRB{}", i).c_str()))
		{
			fbo->UnregisterRenderBuffer(texture);
		}
		else
		{
			if (DrawSelectableTextureThumbnail(texture, size, uv_min, uv_max, borderWidth, border_col, tint_col))
			{
				selectedRBuffer = texture;
			}
			//ImGui::PushItemWidth(groupSize + borderWidth);
			ImGui::PushItemWidth(groupSize);
			std::string popUpLabel = std::format("opfilrbs{}", i);
			float calculatedWidth = ImGui::CalcItemWidth();
			if (ImGui::Button(std::format("{}##RBToSet{}", texture->name, i).c_str(), ImVec2(calculatedWidth, 0.f)))
			{
				ImGui::OpenPopup(popUpLabel.c_str());
			}

			ImVec2 button_min = ImGui::GetItemRectMin();
			ImVec2 button_max = ImGui::GetItemRectMax();

			ImVec2 popupPosition;
			popupPosition.x = button_min.x;
			popupPosition.y = button_min.y;

			ImGuiWindowFlags flags =
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoSavedSettings;
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::SetNextWindowPos(popupPosition, ImGuiCond_Appearing);
			if (ImGui::BeginPopup(popUpLabel.c_str(), flags))
			{
				ImGui::PushItemWidth(calculatedWidth);
				std::string filterLabel = "##searchRenderBuffers" + texture->name;
				static ImGuiTextFilter filter;
				ImGui::PushItemWidth(-1);
				filter.Draw(filterLabel.c_str());
				ImGui::PopItemWidth();
				bool isFilterInFocus = ImGui::IsItemActive();
				static RenderBuffer* bufferToRegister = nullptr;
				if (DrawTexturesThumbnailsWithLabel(filter, &bufferToRegister, renderBuffers, visibleWindowWidth))
				{
					buffersToUnregister.push_back(texture);
					buffersToRegister.push_back(bufferToRegister);
				}

				ImGui::PopItemWidth();
				ImGui::EndPopup();
			}
			ImGui::PopStyleVar(2);
			ImGui::PopItemWidth();
		}
		
		float lastItem_x2 = ImGui::GetItemRectMax().x;
		float nextItem_x2 = lastItem_x2 + style.ItemSpacing.x + size.x;
		ImGui::EndGroup();
		if (nextItem_x2 < window_visible_x_child)
			ImGui::SameLine();
		else
			ImGui::AlignTextToFramePadding();
		i++;
	}
	ImGui::EndChild();
	for (auto buffer : buffersToUnregister)
	{
		fbo->UnregisterRenderBuffer(buffer);
	}
	for (auto buffer : buffersToRegister)
	{
		fbo->RegisterRenderBuffer(buffer);
	}
	ImGui::SameLine();
	if (resetInspectors)
	{
		selectedRBuffer = nullptr;
	}
	// Right
	{
		ImGui::BeginGroup();
		ImGui::BeginChild("left pane details", ImVec2(windowWidth * 0.4f, windowHeight * 0.4f));
		RenderBufferBasicDetails(selectedRBuffer);
		ImGui::EndChild();
		ImGui::EndGroup();
	}

	if (ImGui::Button("Revert")) {}
	ImGui::SameLine();
	if (ImGui::Button("Save"))
	{
		fbo->path = std::format("resources/frame_buffers/{}.json", fbo->name);
		lua_State* L = luaL_newstate();
		luaL_openlibs(L);
		LuaTools::dofile(L, "resources/serialization/frame_buffer_saver.lua");
		lua_getglobal(L, "SaveFrameBuffer");
		if (lua_isfunction(L, -1))
		{
			lua_pushlightuserdata(L, fbo);
			int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
			lua_pop(L, 1);
		}
		lua_close(L);
	}
}

void Editor::RenderPassesInspector()
{
	// Left
	static RenderPass* selectedPass = nullptr;
	{
		ImGui::BeginChild("left pane", ImVec2(150, 0), true);

		static ImGuiTextFilter filterAvailableRenderPasses;
		filterAvailableRenderPasses.Draw("##filterAvailableRenderPassesFilter", -1.0f);
		std::map<std::string, RenderPass*> passesSorted;
		for (auto& asset : *GraphicsStorage::assetRegistry.GetPool<RenderPass>())
		{
			passesSorted.emplace(std::make_pair(asset.name, &asset));
		}
		if (ImGui::BeginListBox("##filterable_renderPasses", ImVec2(-1, -1)))
		{
			for (auto& [name, pass] : passesSorted)
			{
				if (filterAvailableRenderPasses.PassFilter(name.c_str()))
				{
					if (!pass->path.empty())
					{
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
					}
					else
					{
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
					}
					if (ImGui::Selectable(name.c_str(), selectedPass == pass))
					{
						selectedPass = pass;
					}
					ImGui::PopStyleColor();
				}
			}
			ImGui::EndListBox();
		}
		ImGui::EndChild();
	}
	ImGui::SameLine();

	// Right
	{
		ImGui::BeginGroup();
		static char name[128];
		if (CreateNewAssetUI(name))
		{
			selectedPass = CreateNewRenderPass(name);
		}
		if (selectedPass != nullptr)
		{
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
			ImGui::Text(selectedPass->name.c_str());
			ImGui::Text(selectedPass->path.c_str());
			ImGui::Separator();
			RenderPassInspector(selectedPass);
			ImGui::EndChild();
		}
		ImGui::EndGroup();
	}
}

void Editor::RenderQueueInspector()
{
	struct PassAndEnabled
	{
		RenderElement* rps;
		bool enabled;
	};
	static std::vector<PassAndEnabled> enabled = []() {
		std::vector<PassAndEnabled> out;
		for (auto c : GraphicsStorage::renderingQueue)
			out.emplace_back(c, true);
		return out;
	}();

	if (ImGui::Button("Save"))
	{
		lua_State* L = luaL_newstate();
		luaL_openlibs(L);
		LuaTools::dofile(L, "resources/serialization/render_queue_saver.lua");
		lua_getglobal(L, "SaveRenderQueue");
		if (lua_isfunction(L, -1))
		{
			int result = LuaTools::report(L, LuaTools::docall(L, 0, 1));
			lua_pop(L, 1);
		}
		lua_close(L);
	}
	
	if (ImGui::Button("Load Pass"))
	{
		ImGui::OpenPopup("ListOfPasses");
	}
	ImGui::Separator();
	ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("ListOfPasses", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Select Pass To Load");
		static ImGuiTextFilter filterAvailablePassesOnDisk;
		filterAvailablePassesOnDisk.Draw("##filterAvailableVosOnDiskFilter", -1.0f);
		
		static std::filesystem::directory_entry selectedPassToLoad;
		if (ImGui::BeginListBox("##filterable_vaosOnDisk", ImVec2(-1, 0)))
		{
			for (auto& entry : std::filesystem::directory_iterator("resources/passes"))
			{
				if (filterAvailablePassesOnDisk.PassFilter(entry.path().string().c_str()))
				{
					if (ImGui::Selectable(entry.path().string().c_str(), selectedPassToLoad == entry))
					{
						selectedPassToLoad = entry;
					}
				}
			}
			ImGui::EndListBox();
		}
		ImGui::Separator();
		if (ImGui::Button("Load", ImVec2(120, 0)))
		{
			if (selectedPassToLoad.exists())
			{
				std::string passPath = selectedPassToLoad.path().string();
				std::replace(passPath.begin(), passPath.end(), '\\', '/');
				lua_State* L = luaL_newstate();
				luaL_openlibs(L);
				LuaTools::dofile(L, "resources/serialization/pass_loader.lua");
				lua_getglobal(L, "LoadPass");
				if (lua_isfunction(L, -1))
				{
					lua_pushstring(L, passPath.c_str());
					int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
					lua_pop(L, 1);
				}
				lua_close(L);

				for (auto& pass : *GraphicsStorage::assetRegistry.GetPool<RenderPass>())
				{
					if (pass.path.compare(passPath) == 0)
					{
						enabled.emplace_back(&pass, false);
						break;
					}
				}
			}
			ImGui::CloseCurrentPopup();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Close", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	
	ImVec2 contentRegionAvaliable = ImGui::GetContentRegionAvail();
	contentRegionAvaliable.x = contentRegionAvaliable.x - ImGui::GetStyle().FramePadding.x*2.0;
	ImGui::BeginChild("left pane", ImVec2(contentRegionAvaliable.x/2.0, 0), true);
	if (ImGui::TreeNodeEx("Render Queue Order", ImGuiTreeNodeFlags_DefaultOpen))
	{
		static int activeItemIndex = -1;
		static int targetItemIndex = -1;
		//ImGui::Text(std::format("active item {}", activeItemIndex).c_str());
		for (int n = 0; n < enabled.size(); n++)
		{
			if (enabled[n].enabled)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(127, 127, 127, 255));
			}
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 0.0f));
			if (ImGui::IsMouseDown(0) && n == targetItemIndex)
			{
				ImGui::Separator();
			}
			if (ImGui::Checkbox(std::format("##{}", n).c_str(), &enabled[n].enabled))
			{
				auto res = std::find(GraphicsStorage::renderingQueue.begin(), GraphicsStorage::renderingQueue.end(), enabled[n].rps);
				if (!enabled[n].enabled)
				{
					if (res != GraphicsStorage::renderingQueue.end())
					{
						GraphicsStorage::renderingQueue.erase(res);
					}
				}
				else
				{
					if (res == GraphicsStorage::renderingQueue.end())
					{
						int enabledCount = 0;
						for (size_t i = 0; i < n; i++)
						{
							if (enabled[i].enabled)
							{
								enabledCount++;
							}
						}
						GraphicsStorage::renderingQueue.insert(GraphicsStorage::renderingQueue.begin() + enabledCount, enabled[n].rps);
					}
				}
			}
			ImGui::PopStyleVar();
			ImGui::SameLine();
			ImGui::Selectable(enabled[n].rps->name.c_str(), false, ImGuiSelectableFlags_AllowItemOverlap);
			ImGui::PopStyleColor();
			if (ImGui::IsItemActive())
			{
				activeItemIndex = n;
			}
			if (ImGui::IsMouseDown(0) && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
			{
				targetItemIndex = n;
			}
			if (targetItemIndex != -1 && activeItemIndex != -1 && activeItemIndex != targetItemIndex)
			{
				auto activeItem = enabled[activeItemIndex];
				auto targetItem = enabled[targetItemIndex];
				enabled[activeItemIndex] = targetItem;
				enabled[targetItemIndex] = activeItem;
				activeItemIndex = targetItemIndex;
				auto aI = std::find(GraphicsStorage::renderingQueue.begin(), GraphicsStorage::renderingQueue.end(), activeItem.rps);
				auto tI = std::find(GraphicsStorage::renderingQueue.begin(), GraphicsStorage::renderingQueue.end(), targetItem.rps);
				if (aI != GraphicsStorage::renderingQueue.end() && tI != GraphicsStorage::renderingQueue.end())
				{
					std::iter_swap(aI, tI);
				}
			}
			//ImGui::Text(std::format("target item {}", targetItemIndex).c_str());
		}
		if (!ImGui::IsMouseDown(0))
		{
			activeItemIndex = -1;
			targetItemIndex = -1;
		}
		ImGui::TreePop();
		
	}
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("item view", ImVec2(contentRegionAvaliable.x / 2.0, 0), true);
	if (ImGui::TreeNodeEx("Render Queue Result", ImGuiTreeNodeFlags_DefaultOpen))
	{
		int rqi = 0;
		for (auto rps : GraphicsStorage::renderingQueue)
		{
			ImGui::Selectable(std::format("{}##rqi{}", rps->name, rqi).c_str());
		}
		ImGui::TreePop();
	}
	ImGui::EndChild();
}

void Editor::ShadersInspector()
{
	float windowWidth = ImGui::GetWindowContentRegionWidth();
	float leftPanePercentSize = 150 / windowWidth;
	float otherTwoPanesRemainingPercent = 1.f - leftPanePercentSize;
	std::vector<Shader*> shadersToUnload;
	std::vector<Shader*> shadersToDelete;

	// Left
	static Shader* selectedShader = nullptr;
	bool newSelection = false;
	{
		float leftPaneSize = ImGui::GetWindowContentRegionWidth() * leftPanePercentSize;
		ImGui::BeginChild("left pane", ImVec2(ImGui::GetWindowContentRegionWidth() * leftPanePercentSize, 0), true);

		static ImGuiTextFilter filterAvailableShaders;
		filterAvailableShaders.Draw("##filterAvailableShadersFilter", -1.0f);
		std::map<std::string, Shader*> sortedShaders;
		int i = 0;
		for (auto& asset : *GraphicsStorage::assetRegistry.GetPool<Shader>())
		{
			sortedShaders.emplace(std::make_pair(asset.name + "##" + std::to_string(i), &asset));
			i++;
		};
		if (ImGui::BeginListBox("##filterable_shaders", ImVec2(-1, -1)))
		{
			for (auto& [name, shader] : sortedShaders)
			{
				if (filterAvailableShaders.PassFilter(name.c_str()))
				{
					if (ImGui::Selectable(name.c_str(), selectedShader == shader))
					{
						selectedShader = shader;
						newSelection = true;
					}
				}
			}
			ImGui::EndListBox();
		}
		ImGui::EndChild();
	}
	ImGui::SameLine();

	// Right
	{
		ImGui::BeginGroup();
		float itemViewSize = ImGui::GetWindowContentRegionWidth() * otherTwoPanesRemainingPercent * 0.5f;
		static char name[128];
		if (CreateNewAssetUI(name))
		{
			selectedShader = CreateNewShader(name);
			newSelection = true;
		}
		if (ImGui::Button("Load Shader"))
		{
			ImGui::OpenPopup("ListOfShaders");
		}
		ImGui::SameLine();
		if (ImGui::Button("Reload Shaders"))
		{
			GraphicsManager::ReloadShaders();
		}
		ImGui::Separator();
		ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("ListOfShaders", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Select Shader To Load");
			static ImGuiTextFilter filterAvailableShadersOnDisk;
			filterAvailableShadersOnDisk.Draw("##filterAvailableShadersOnDiskFilter", -1.0f);
			std::vector<std::string> shaderNames;
			GraphicsManager::GetFileNames(shaderNames, "resources/shaders/", ".vs");
			static std::string selectedShaderToLoad = "";
			if (ImGui::BeginListBox("##filterable_shadersOnDisk", ImVec2(-1, 0)))
			{
				for (auto& sname : shaderNames)
				{
					if (filterAvailableShadersOnDisk.PassFilter(sname.c_str()))
					{
						if (ImGui::Selectable(sname.c_str(), selectedShaderToLoad == sname))
						{
							selectedShaderToLoad = sname;
						}
					}
				}
				ImGui::EndListBox();
			}
			ImGui::Separator();
			if (ImGui::Button("Load", ImVec2(120, 0)))
			{
				if (!selectedShaderToLoad.empty())
				{
					std::string shaderPath = std::format("resources/shaders/{}", selectedShaderToLoad);
					ShaderPaths sp = GraphicsManager::LoadShaderPaths(shaderPath);
					//GraphicsManager::AddPairToJson(selectedShaderToLoad, shaderPath, std::string("resources/shaders.json"));
					selectedShader = GraphicsManager::ReloadShaderFromPath(selectedShaderToLoad.c_str(), sp);
					newSelection = true;
				}
				ImGui::CloseCurrentPopup();
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Close", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (selectedShader != nullptr)
		{
			ImGui::BeginChild("item view", ImVec2(ImGui::GetWindowContentRegionWidth() * otherTwoPanesRemainingPercent * 0.4f, -ImGui::GetFrameHeightWithSpacing()));
			ImGui::AlignTextToFramePadding();
			ImGui::Text(selectedShader->name.c_str());
			std::string oguid = GraphicsStorage::assetRegistry.GetAssetIDAsString(selectedShader);
			ImGui::Text(oguid.c_str());
			ImGui::Text(std::to_string(selectedShader->shaderID).c_str());
			if (ImGui::Button("Reload Shader"))
			{
				GraphicsManager::ReloadShader(selectedShader);
			}
			ImGui::SameLine();
			if (ImGui::Button("Unload Shader"))
			{
				shadersToUnload.push_back(selectedShader);
			}
			ImGui::SameLine();
			if (ImGui::Button("Delete Shader"))
			{
				ImGui::OpenPopup("Delete Shader?");
			}
			ImGui::Separator();
			if (ImGui::BeginPopupModal("Delete Shader?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Are you sure you want to delete %s shader?\n\n", selectedShader->name.c_str());
				ImGui::Separator();
				if (ImGui::Button("Yes", ImVec2(120, 0)))
				{
					shadersToDelete.push_back(selectedShader);
					ImGui::CloseCurrentPopup();
				}
				ImGui::SetItemDefaultFocus();
				ImGui::SameLine();
				if (ImGui::Button("No", ImVec2(120, 0)))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
			Shader* shader = selectedShader;
			if (shader->outputs.size() > 0) ImGui::Text("Outputs:");
			ImGui::Separator();
			for (auto& output : shader->outputs)
			{
				ImGui::Text(std::format("Index: {}\nType: {}\nName: {}", output.index, ShaderDataType::Str(output.type), output.name).c_str());
				ImGui::Separator();
			}
			if (shader->samplers.size() > 0) ImGui::Text("Samplers:");
			ImGui::Separator();
			for (auto& sampler : shader->samplers)
			{
				ImGui::Text(std::format("Index: {}\nType: {}\nName: {}", sampler.index, sampler.type, sampler.name).c_str());
				ImGui::Separator();
			}
			
			if (shader->attributes.size() > 0)
			{
				ImGui::Text("Attributes:");
				for (auto& attribute : shader->attributes)
				{
					//ImGui::Text("%d %s %s", attribute.index, attribute.type.c_str(), attribute.name.c_str());
					//ImGui::Text("%d %s %s", attribute.index, attribute.type.c_str(), attribute.name.c_str());
					ImGui::Text(std::format("{} {} {}", attribute.index, ShaderDataType::Str(attribute.type), attribute.name).c_str());
					//if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
					//{
					//	ImGui::SetDragDropPayload("ShaderAttribute_SELECTION", &attribute, sizeof(ShaderOutput));
						//add here anything for tooltip
					//	ImGui::EndDragDropSource();
					//}
				}
				ImGui::Separator();
			}

			ImGui::Separator();
			if (ImGui::CollapsingHeader("Uniform Buffers"))
			{
				ImGui::Separator();

				if (shader->globalUniformBuffers.size() > 0)
				{
					if (ImGui::CollapsingHeader("Global Uniform Buffers"))
					{
						for (auto& uniformBuffer : shader->globalUniformBuffers)
						{
							ShaderBlocksInspector(uniformBuffer);
							ImGui::Separator();
						}
					}
				}
				if (shader->materialUniformBuffers.size() > 0)
				{
					if (ImGui::CollapsingHeader("Material Uniform Buffers"))
					{
						for (auto& uniformBuffer : shader->materialUniformBuffers)
						{
							ShaderBlocksInspector(uniformBuffer);
							ImGui::Separator();
						}
					}
				}
				

				if (shader->objectUniformBuffers.size() > 0)
				{
					if (ImGui::CollapsingHeader("Object Uniform Buffers"))
					{
						for (auto& uniformBuffer : shader->objectUniformBuffers)
						{
							ShaderBlocksInspector(uniformBuffer);
							ImGui::Separator();
						}
					}
				}
			}
			ImGui::EndChild();

			ImGui::SameLine();
			{
				ImGuiStyle& style = ImGui::GetStyle();
				float itemViewSize = ImGui::GetWindowContentRegionWidth() * otherTwoPanesRemainingPercent * 0.6f - 4 * style.ItemSpacing.x;
				ImGui::BeginChild("shader code editor", ImVec2(itemViewSize, -ImGui::GetFrameHeightWithSpacing()), true);
				ShaderCodeEditor(selectedShader, newSelection);
				ImGui::EndChild();
			}
		}
		ImGui::EndGroup();
	}

	

	for (auto sh : shadersToUnload)
	{
		std::string shaderName = sh->name;
		for (auto& mat : *GraphicsStorage::assetRegistry.GetPool<Material>())
		{
			if (mat.shader == sh)
			{
				mat.shader = nullptr;
			}
		}
		//GraphicsStorage::shaderIDs.erase(shaderName);
		//GraphicsManager::RemoveItemFromJson(shaderName, std::string("resources/shaders.json"));
		selectedShader = nullptr;
	}

	for (auto sh : shadersToDelete)
	{
		std::string shaderName = sh->name;
		for (auto& mat : *GraphicsStorage::assetRegistry.GetPool<Material>())
		{
			if (mat.shader == sh)
			{
				mat.shader = nullptr;
			}
		}
		std::remove(sh->shaderPaths.vs.c_str());
		std::remove(sh->shaderPaths.fs.c_str());
		std::remove(sh->shaderPaths.gs.c_str());
		//GraphicsStorage::shaderIDs.erase(shaderName);
		//GraphicsManager::RemoveItemFromJson(shaderName, std::string("resources/shaders.json"));
		selectedShader = nullptr;
	}
}

void Editor::ShaderBlockInspector()
{
	if (ImGui::BeginTabItem("All"))
	{
		// Left
		static std::string selectedName = "";
		static ShaderBlock* selectedBuffer = nullptr;
		{
			ImGui::BeginChild("left pane", ImVec2(150, 0), true);

			static ImGuiTextFilter filterAvailableUniformBuffers;
			filterAvailableUniformBuffers.Draw("##filterAvailableUniformBuffersFilter", -1.0f);

			if (ImGui::BeginListBox("##filterable_uniformBuffers", ImVec2(-1, -1)))
			{

				std::sort(std::begin(GraphicsStorage::uniformBuffers), std::end(GraphicsStorage::uniformBuffers),
				[](const ShaderBlock* a, const ShaderBlock* b) -> bool
				{
					return a->index < b->index;
				});
				for (auto& uniformBuffer : GraphicsStorage::uniformBuffers)
				{
					if (filterAvailableUniformBuffers.PassFilter(uniformBuffer->name.c_str()))
					{
						if (ImGui::Selectable(std::format("{}: {}", uniformBuffer->name, uniformBuffer->index).c_str(), selectedName == uniformBuffer->name))
						{
							selectedName = uniformBuffer->name;
							selectedBuffer = uniformBuffer;
						}
					}
				}
				ImGui::EndListBox();
			}
			ImGui::EndChild();
		}
		ImGui::SameLine();

		// Right
		{
			ImGui::BeginGroup();
			ImGui::Text(selectedName.c_str());
			ImGui::Separator();
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
			if (!selectedName.empty())
			{
				ShaderBlocksInspector(selectedBuffer);
			}
			ImGui::EndChild();
			if (ImGui::Button("Revert")) {}
			ImGui::SameLine();
			if (ImGui::Button("Save"))
			{
				std::string configPath = std::format("resources/shader_blocks/{}.json", selectedBuffer->name);
				lua_State* L = luaL_newstate();
				luaL_openlibs(L);
				LuaTools::dofile(L, "resources/serialization/shader_block_config_saver.lua");
				lua_getglobal(L, "SaveShaderBlockConfig");
				if (lua_isfunction(L, -1))
				{
					lua_pushlightuserdata(L, selectedBuffer);
					lua_pushstring(L, configPath.c_str());
					int result = LuaTools::report(L, LuaTools::docall(L, 2, 1));
					lua_pop(L, 1);
				}
				lua_close(L);
			}
			ImGui::EndGroup();
		}
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Global"))
	{
		// Left
		static std::string selectedName = "";
		static ShaderBlock* selectedBuffer = nullptr;
		{
			ImGui::BeginChild("left pane", ImVec2(150, 0), true);

			static ImGuiTextFilter filterAvailableUniformBuffers;
			filterAvailableUniformBuffers.Draw("##filterAvailableUniformBuffersFilter", -1.0f);

			if (ImGui::BeginListBox("##filterable_uniformBuffers", ImVec2(-1, -1)))
			{
				std::sort(GraphicsStorage::uniformBuffers.begin(), GraphicsStorage::uniformBuffers.end(),
				[](const ShaderBlock* a, const ShaderBlock* b) -> bool
				{
					return a->index < b->index;
				});
				for (auto& uniformBuffer : GraphicsStorage::uniformBuffers)
				{
					if (uniformBuffer->name.find("G_") != std::string::npos)
					{
						if (filterAvailableUniformBuffers.PassFilter(uniformBuffer->name.c_str()))
						{
							if (ImGui::Selectable(std::format("{}: {}", uniformBuffer->name, uniformBuffer->index).c_str(), selectedName == uniformBuffer->name))
							{
								selectedName = uniformBuffer->name;
								selectedBuffer = uniformBuffer;
							}
						}
					}
				}
				ImGui::EndListBox();
			}
			ImGui::EndChild();
		}
		ImGui::SameLine();

		// Right
		{
			ImGui::BeginGroup();
			ImGui::Text(selectedName.c_str());
			ImGui::Separator();
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
			if (!selectedName.empty())
			{
				ShaderBlocksInspector(selectedBuffer);
			}
			ImGui::EndChild();
			ImGui::EndGroup();
			if (ImGui::Button("Revert")) {}
			ImGui::SameLine();
			if (ImGui::Button("Save")) {}
		}
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Material"))
	{
		// Left
		static std::string selectedName = "";
		static ShaderBlock* selectedBuffer = nullptr;
		{
			ImGui::BeginChild("left pane", ImVec2(150, 0), true);

			static ImGuiTextFilter filterAvailableUniformBuffers;
			filterAvailableUniformBuffers.Draw("##filterAvailableUniformBuffersFilter", -1.0f);

			if (ImGui::BeginListBox("##filterable_uniformBuffers", ImVec2(-1, -1)))
			{

				std::sort(GraphicsStorage::uniformBuffers.begin(), GraphicsStorage::uniformBuffers.end(),
				[](const ShaderBlock* a, const ShaderBlock* b) -> bool
				{
					return a->index < b->index;
				});
				for (auto& uniformBuffer : GraphicsStorage::uniformBuffers)
				{
					if (uniformBuffer->name.find("M_") != std::string::npos)
					{
						if (filterAvailableUniformBuffers.PassFilter(uniformBuffer->name.c_str()))
						{
							if (ImGui::Selectable(std::format("{}: {}", uniformBuffer->name, uniformBuffer->index).c_str(), selectedName == uniformBuffer->name))
							{
								selectedName = uniformBuffer->name;
								selectedBuffer = uniformBuffer;
							}
						}
					}
				}
				ImGui::EndListBox();
			}
			ImGui::EndChild();
		}
		ImGui::SameLine();

		// Right
		{
			ImGui::BeginGroup();
			ImGui::Text(selectedName.c_str());
			ImGui::Separator();
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
			if (!selectedName.empty())
			{
				ShaderBlocksInspector(selectedBuffer);
			}
			ImGui::EndChild();
			ImGui::EndGroup();
			if (ImGui::Button("Revert")) {}
			ImGui::SameLine();
			if (ImGui::Button("Save")) {}
		}
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Object"))
	{
		// Left
		static std::string selectedName = "";
		static ShaderBlock* selectedBuffer = nullptr;
		{
			ImGui::BeginChild("left pane", ImVec2(150, 0), true);

			static ImGuiTextFilter filterAvailableUniformBuffers;
			filterAvailableUniformBuffers.Draw("##filterAvailableUniformBuffersFilter", -1.0f);

			if (ImGui::BeginListBox("##filterable_uniformBuffers", ImVec2(-1, -1)))
			{

				std::sort(GraphicsStorage::uniformBuffers.begin(), GraphicsStorage::uniformBuffers.end(),
				[](const ShaderBlock* a, const ShaderBlock* b) -> bool
				{
					return a->index < b->index;
				});
				for (auto& uniformBuffer : GraphicsStorage::uniformBuffers)
				{
					if (uniformBuffer->name.find("O_") != std::string::npos)
					{
						if (filterAvailableUniformBuffers.PassFilter(uniformBuffer->name.c_str()))
						{
							if (ImGui::Selectable(std::format("{}: {}", uniformBuffer->name, uniformBuffer->index).c_str(), selectedName == uniformBuffer->name))
							{
								selectedName = uniformBuffer->name;
								selectedBuffer = uniformBuffer;
							}
						}
					}
				}
				ImGui::EndListBox();
			}
			ImGui::EndChild();
		}
		ImGui::SameLine();

		// Right
		{
			ImGui::BeginGroup();
			ImGui::Text(selectedName.c_str());
			ImGui::Separator();
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
			if (!selectedName.empty())
			{
				ShaderBlocksInspector(selectedBuffer);
			}
			ImGui::EndChild();
			ImGui::EndGroup();
			if (ImGui::Button("Revert")) {}
			ImGui::SameLine();
			if (ImGui::Button("Save")) {}
		}
		ImGui::EndTabItem();
	}
}

void Editor::BuffersDefinitionsInspector()
{
	// Left
	static std::string selectedName = "";
	bool newSelection = false;
	{
		ImGui::BeginChild("left pane", ImVec2(150, 0), true);

		static ImGuiTextFilter filterAvailableVbos;
		filterAvailableVbos.Draw("##filterAvailableDefinitions", -1.0f);
		std::vector<std::string> names;
		for (auto& asset : GraphicsStorage::bufferDefinitions)
		{
			names.push_back(asset.first);
		}
		std::sort(names.begin(), names.end());
		if (ImGui::BeginListBox("##filterable_vbos", ImVec2(-1, -1)))
		{
			for (auto& definitionName : names)
			{
				if (filterAvailableVbos.PassFilter(definitionName.c_str()))
				{
					if (ImGui::Selectable(definitionName.c_str(), selectedName == definitionName))
					{
						selectedName = definitionName;
					}
				}
			}
			ImGui::EndListBox();
		}
		ImGui::EndChild();
	}
	ImGui::SameLine();

	// Right
	{
		ImGui::BeginGroup();

		if (!selectedName.empty())
		{
			ImGui::AlignTextToFramePadding();
			ImGui::Text(selectedName.c_str());
			ImGui::Separator();
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
			ImGui::PushItemWidth(ImGui::GetFontSize() * -10.f);
			ImGui::Text("Buffer Layouts:");
			std::vector<std::string> locationsToRemove;
			std::unordered_map<std::string, std::vector<BufferLayout>> bufferLayoutsToRemove;
			if (ImGui::Button("Add Buffer Layout##addBufferLayoutToList"))
			{
				GraphicsStorage::bufferDefinitions[selectedName].emplace_back();
			}
			int i = 0;
			for (auto& bufferLayout : GraphicsStorage::bufferDefinitions[selectedName])
			{
				//nice grid? table?
				const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
				ImVec2 contentRegionAvaliable = ImGui::GetContentRegionAvail();
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
				ImGui::Separator();
				std::string name = std::format("Buffer Layout##{}", i);
				bool open = ImGui::TreeNodeEx(name.c_str(), treeNodeFlags, name.c_str());

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ShaderAttribute_SELECTION"))
					{
						IM_ASSERT(payload->DataSize == sizeof(ShaderOutput));
						ShaderOutput droppedAttribute = *(ShaderOutput*)payload->Data;

						auto it = std::find_if(bufferLayout.locations.begin(), bufferLayout.locations.end(), [&droppedAttribute](LocationLayout& n) { return n.name == droppedAttribute.name; });
						if (it == bufferLayout.locations.end()) //not inside this buffer
						{
							//look for buffer with that location
							for (auto& buffer : GraphicsStorage::bufferDefinitions[selectedName])
							{
								auto it = std::find_if(buffer.locations.begin(), buffer.locations.end(), [&droppedAttribute](LocationLayout& n) { return n.name == droppedAttribute.name; });
								if (it != buffer.locations.end())
								{
									//found in another buffer
									buffer.locations.erase(it);
									break;
								}
							}
							bufferLayout.AddLocationLayout(LocationLayout(droppedAttribute.type, droppedAttribute.name));
							bufferLayout.CalculateOffsetsAndStride();
						}
					}
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("LocationLayout_SELECTION"))
					{
						IM_ASSERT(payload->DataSize == sizeof(LocationLayout));
						LocationLayout droppedLocationLayout = *(LocationLayout*)payload->Data;

						auto it = std::find_if(bufferLayout.locations.begin(), bufferLayout.locations.end(), [&droppedLocationLayout](LocationLayout& n) { return n.name == droppedLocationLayout.name; });
						if (it == bufferLayout.locations.end()) //not inside this buffer
						{
							//look for buffer with that location
							for (auto& buffer : GraphicsStorage::bufferDefinitions[selectedName])
							{
								auto it = std::find_if(buffer.locations.begin(), buffer.locations.end(), [&droppedLocationLayout](LocationLayout& n) { return n.name == droppedLocationLayout.name; });
								if (it != buffer.locations.end())
								{
									//found in another buffer
									buffer.locations.erase(it);
									break;
								}
							}
							bufferLayout.AddLocationLayout(droppedLocationLayout);
						}
						bufferLayout.CalculateOffsetsAndStride();
					}
					ImGui::EndDragDropTarget();
				}


				ImGui::PopStyleVar();
				ImGui::SameLine(contentRegionAvaliable.x - lineHeight * 2.4f);
				if (ImGui::Button("Remove", ImVec2{ lineHeight * 2.5f, lineHeight }))
				{
					bufferLayoutsToRemove[selectedName].push_back(bufferLayout);
				}

				if (open)
				{
					std::string bufferLayoutTableLabel = std::format("BufferLayoutTable{}", i);

					if (ImGui::BeginTable(bufferLayoutTableLabel.c_str(), 4, ImGuiTableFlags_SizingFixedFit))
					{
						for (auto& location : bufferLayout.locations)
						{
							//here we actually want to show buffer layouts and their locations with instancesPerAttribute and normalized
							ImGui::TableNextColumn();
							ImGui::AlignTextToFramePadding();
							ImGui::Text(ShaderDataType::Str(location.type).data());
							ImGui::SameLine();
							ImGui::Selectable(location.name.c_str());
							if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
							{
								ImGui::SetDragDropPayload("LocationLayout_SELECTION", &location, sizeof(LocationLayout));
								ImGui::Text(location.name.c_str());
								ImGui::EndDragDropSource();
							}

							ImGui::TableNextColumn();
							ImGui::PushItemWidth(40);
							ImGui::DragInt(std::format("Instances Per Attribute##instancesPerAttribute{}", location.name).c_str(), (int*)&location.instancesPerAttribute, 1.0f, 0, UINT32_MAX);
							ImGui::PopItemWidth();
							ImGui::TableNextColumn();
							ImGui::Checkbox(std::format("Normalized##normalized{}", location.name).c_str(), &location.normalized);
							ImGui::TableNextColumn();
							if (ImGui::Button("Remove"))
							{
								auto it = std::find_if(bufferLayout.locations.begin(), bufferLayout.locations.end(), [location](LocationLayout& n) { return n.name == location.name; });
								if (it != bufferLayout.locations.end())
								{
									locationsToRemove.push_back((*it).name);
								}
							}
						}
						ImGui::EndTable();
					}
					for (auto& location : locationsToRemove)
					{
						auto it = std::find_if(bufferLayout.locations.begin(), bufferLayout.locations.end(), [location](LocationLayout& n) { return n.name == location; });
						if (it != bufferLayout.locations.end())
						{
							bufferLayout.locations.erase(it);
						}
					}

					ImGui::TreePop();
				}
				i++;
			}
			for (auto& nameAndBufferLayouts : bufferLayoutsToRemove)
			{
				for (auto& bufferLayout : nameAndBufferLayouts.second)
				{
					auto it = std::find_if(GraphicsStorage::bufferDefinitions[nameAndBufferLayouts.first].begin(), GraphicsStorage::bufferDefinitions[nameAndBufferLayouts.first].end(), [bufferLayout](BufferLayout& n)
						{
							if (bufferLayout.GetOffset() == n.GetOffset() && bufferLayout.GetStride() == n.GetStride() && bufferLayout.locations.size() == n.locations.size())
							{
								if (bufferLayout.locations.size() > 0)
								{
									if (bufferLayout.locations[0].name == n.locations[0].name)
									{
										return true;
									}
									else
									{
										return false;
									}
								}
								return true;
							}
							return false;
						}
					);
					if (it != GraphicsStorage::bufferDefinitions[nameAndBufferLayouts.first].end())
					{
						GraphicsStorage::bufferDefinitions[nameAndBufferLayouts.first].erase(it);
					}
				}
			}
			ImGui::PopItemWidth();
			ImGui::EndChild();
		}
		ImGui::EndGroup();
		if (ImGui::Button("Revert")) {}
		ImGui::SameLine();
		if (ImGui::Button("Save")) {}
	}
}

void Editor::MaterialProfilesInspector()
{
	// Left
	static MaterialProfile* selectedMaterialProfile = nullptr;
	{
		ImGui::BeginChild("left pane", ImVec2(150, 0), true);

		static ImGuiTextFilter filterAvailableMaterialProfiles;
		filterAvailableMaterialProfiles.Draw("##filterAvailableMaterialProfilesFilter", -1.0f);
		std::map<std::string, MaterialProfile*> mpsSorted;
		int i = 0;
		for (auto& asset : *GraphicsStorage::assetRegistry.GetPool<MaterialProfile>())
		{
			mpsSorted.emplace(std::make_pair(asset.name + "##" + std::to_string(i), &asset));
			i++;
		}
		if (ImGui::BeginListBox("##filterable_materialProfiles", ImVec2(-1, -1)))
		{
			for (auto& profile : mpsSorted)
			{
				if (filterAvailableMaterialProfiles.PassFilter(profile.first.c_str()))
				{
					if (!profile.second->path.empty())
					{
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
					}
					else
					{
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
					}
					if (ImGui::Selectable(profile.first.c_str(), selectedMaterialProfile == profile.second))
					{
						selectedMaterialProfile = profile.second;
					}
					ImGui::PopStyleColor();
				}
			}
			ImGui::EndListBox();
		}
		ImGui::EndChild();
	}
	ImGui::SameLine();

	// Right
	{
		ImGui::BeginGroup();	
		static char name[128];
		if (CreateNewAssetUI(name))
		{
			selectedMaterialProfile = CreateNewMaterialProfile(name);
		}
		if (selectedMaterialProfile != nullptr)
		{
			ImGui::Text(selectedMaterialProfile->name.c_str());
			ImGui::Separator();
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
			if (DataRegistryInspector(&selectedMaterialProfile->registry, "resources/material_profiles/properties/", selectedMaterialProfile))
			{
				selectedMaterialProfile->UpdateProfileFromDataRegistry(selectedMaterialProfile->registry);
			}
			if (selectedMaterialProfile->shaderBlockDatas.size() > 0)
			{
				if (ImGui::CollapsingHeader("Uniform Buffers"))
				{
					ImGui::Separator();
					for (auto& ubd : selectedMaterialProfile->shaderBlockDatas)
					{
						ShaderBlocksInspector(ubd.shaderBlock);
						ImGui::Separator();
					}
				}
			}
			ImGui::EndChild();
			if (ImGui::Button("Revert")) {}
			ImGui::SameLine();
			if (!selectedMaterialProfile->path.empty())
			{
				if (ImGui::Button("Save"))
				{
					lua_State* L = luaL_newstate();
					luaL_openlibs(L);
					LuaTools::dofile(L, "resources/serialization/material_profile_saver.lua");
					lua_getglobal(L, "SaveMaterialProfile");
					if (lua_isfunction(L, -1))
					{
						lua_pushlightuserdata(L, selectedMaterialProfile);
						int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
						lua_pop(L, 1);
					}
					lua_close(L);
				}
				ImGui::SameLine();
				if (ImGui::Button("Save As"))
				{
					selectedMaterialProfile->path = std::format("resources/material_profiles/{}.json", selectedMaterialProfile->name);
					lua_State* L = luaL_newstate();
					luaL_openlibs(L);
					LuaTools::dofile(L, "resources/serialization/material_profile_saver.lua");
					lua_getglobal(L, "SaveMaterialProfile");
					if (lua_isfunction(L, -1))
					{
						lua_pushlightuserdata(L, selectedMaterialProfile);
						int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
						lua_pop(L, 1);
					}
					lua_close(L);
				}
			}
			else
			{
				if (ImGui::Button("Save As"))
				{
					selectedMaterialProfile->path = std::format("resources/material_profiles/{}", selectedMaterialProfile->name);
					lua_State* L = luaL_newstate();
					luaL_openlibs(L);
					LuaTools::dofile(L, "resources/serialization/material_profile_saver.lua");
					lua_getglobal(L, "SaveMaterialProfile");
					if (lua_isfunction(L, -1))
					{
						lua_pushlightuserdata(L, selectedMaterialProfile);
						int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
						lua_pop(L, 1);
					}
					lua_close(L);
				}
			}
		}
		ImGui::EndGroup();
	}
}

void Editor::TextureProfilesInspector()
{
	// Left
	static TextureProfile* selectedTextureProfile = nullptr;
	{
		ImGui::BeginChild("left pane", ImVec2(150, 0), true);

		static ImGuiTextFilter filterAvailableTextureProfiles;
		filterAvailableTextureProfiles.Draw("##filterAvailableTextureProfilesFilter", -1.0f);
		std::map<std::string, TextureProfile*> tpsSorted;
		int i = 0;
		for (auto& asset : *GraphicsStorage::assetRegistry.GetPool<TextureProfile>())
		{
			tpsSorted.emplace(std::make_pair(asset.name + "##" + std::to_string(i), &asset));
			i++;
		}
		if (ImGui::BeginListBox("##filterable_textureProfiles", ImVec2(-1, -1)))
		{
			for (auto& profile : tpsSorted)
			{
				if (filterAvailableTextureProfiles.PassFilter(profile.first.c_str()))
				{
					if (!profile.second->path.empty())
					{
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
					}
					else
					{
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
					}
					if (ImGui::Selectable(profile.first.c_str(), selectedTextureProfile == profile.second))
					{
						selectedTextureProfile = profile.second;
						//std::sort(selectedTextureProfile->textures.begin(), selectedTextureProfile->textures.end());
					}
					ImGui::PopStyleColor();
				}
			}
			ImGui::EndListBox();
		}
		ImGui::EndChild();
	}
	ImGui::SameLine();

	// Right
	{
		ImGui::BeginGroup();
		static char name[128];
		if (CreateNewAssetUI(name))
		{
			selectedTextureProfile = CreateNewTextureProfile(name);
		}
		if (selectedTextureProfile != nullptr)
		{
			ImGui::Text(selectedTextureProfile->name.c_str());
			ImGui::Separator();
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
			ImVec2 uv_min = ImVec2(0.0f, 1.0f);                 // Top-left
			ImVec2 uv_max = ImVec2(1.0f, 0.0f);                 // Lower-right
			ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
			ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
			float imageSize = 128;
			ImVec2 size = ImVec2(imageSize, imageSize);

			ImGuiStyle& style = ImGui::GetStyle();
			int buttons_count = 20000;
			float visibleWindowWidth = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
			float groupSize = imageSize + 2;
			ImGui::PushItemWidth(groupSize);
			int borderWidth = 2;
			float comboImageWidth = 50;
			ImVec2 comboImageSize = ImVec2(comboImageWidth, comboImageWidth);

			std::string popUpLabel = "add texture slot popup";

			static Texture* textureToAdd = nullptr;
			ImGui::Text("Add Texture:");
			if (DrawSelectableTextureThumbnail(textureToAdd, comboImageSize, uv_min, uv_max, borderWidth, border_col, tint_col))
			{
				ImGui::OpenPopup(popUpLabel.c_str());
			}
			static int slotToAdd = 0;
			if (ImGui::BeginCombo("Slot", std::to_string(slotToAdd).c_str()))
			{
				int nrOfAvailableSlotsOnThisGpu = 16;
				for (size_t i = 0; i < nrOfAvailableSlotsOnThisGpu; i++)
				{
					const bool is_selected = (i == slotToAdd);
					if (ImGui::Selectable(std::to_string(i).c_str(), is_selected))
					{
						slotToAdd = i;
					}
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();
			if (ImGui::Button("Add##addTextureAndSlotButton"))
			{
				if (textureToAdd != nullptr)
				{
					selectedTextureProfile->AssignTexture(textureToAdd, slotToAdd);
					std::sort(selectedTextureProfile->textures.begin(), selectedTextureProfile->textures.end());
				}
			}
			ImGui::Separator();
			/////////////////////////////////////////////

			ImVec2 button_min = ImGui::GetItemRectMin();
			ImVec2 button_max = ImGui::GetItemRectMax();

			ImVec2 popupPosition;
			popupPosition.x = button_min.x;
			popupPosition.y = button_min.y;

			ImGuiWindowFlags flags =
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoSavedSettings;
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::SetNextWindowPos(popupPosition, ImGuiCond_Appearing);
			if (ImGui::BeginPopup(popUpLabel.c_str(), flags))
			{
				//ImGui::PushItemWidth(calculatedWidth);
				std::string filterAddSlotsLabel = "##addSlotSearchElements";
				//float offset = 0.f;
				static ImGuiTextFilter filterAddSlots;
				ImGui::PushItemWidth(-1);
				filterAddSlots.Draw(filterAddSlotsLabel.c_str());
				ImGui::PopItemWidth();
				//bool isFilterInFocus = ImGui::IsItemActive();
				DrawTexturesThumbnailsWithLabel(filterAddSlots, &textureToAdd, textures, visibleWindowWidth);
				DrawTexturesThumbnailsWithLabel(filterAddSlots, &textureToAdd, cubemaps, visibleWindowWidth);
				DrawTexturesThumbnailsWithLabel(filterAddSlots, &textureToAdd, renderTargets, visibleWindowWidth);

				ImGui::EndPopup();
			}
			ImGui::PopStyleVar(2);

			//////////////////////////////////////////////

			ImGui::AlignTextToFramePadding();
			int i = 0;
			for (auto& element : selectedTextureProfile->textures)
			{
				ImGui::BeginGroup();
				int textureSlot = element.slot;
				ImGui::PushItemWidth(45);
				if (ImGui::BeginCombo(std::format("Slot##{}", i).c_str(), std::to_string(textureSlot).c_str()))
				{
					int nrOfAvailableSlotsOnThisGpu = 16;
					for (size_t slotNr = 0; slotNr < nrOfAvailableSlotsOnThisGpu; slotNr++)
					{
						const bool is_selected = (slotNr == textureSlot);
						if (ImGui::Selectable(std::to_string(slotNr).c_str(), is_selected))
						{
							selectedTextureProfile->SetTextureSlot(element.texture, slotNr);
							textureSlot = slotNr;
						}
					}
					ImGui::EndCombo();
				}
				ImGui::PopItemWidth();
				ImGui::SameLine(groupSize - 13);
				std::string removeButtonLabel = std::format("X##rmbtn{}", i);
				if (ImGui::Button(removeButtonLabel.c_str()))
				{
					selectedTextureProfile->RemoveTextureAtSlot(element.texture, element.slot);
					std::sort(selectedTextureProfile->textures.begin(), selectedTextureProfile->textures.end());
				}
				else
				{
					DrawSelectableTextureThumbnail(element.texture, size, uv_min, uv_max, borderWidth, border_col, tint_col);
					//ImGui::PushItemWidth(groupSize + borderWidth);
					ImGui::PushItemWidth(groupSize);
					std::string popUpLabel = std::format("opnfiltx{}", i);
					float calculatedWidth = ImGui::CalcItemWidth();
					if (ImGui::Button(std::format("{}##TexturesToSetInSlotButton{}", element.texture->name, i).c_str(), ImVec2(calculatedWidth, 0.f)))
					{
						ImGui::OpenPopup(popUpLabel.c_str());
					}

					ImVec2 button_min = ImGui::GetItemRectMin();
					ImVec2 button_max = ImGui::GetItemRectMax();

					ImVec2 popupPosition;
					popupPosition.x = button_min.x;
					popupPosition.y = button_min.y;

					ImGuiWindowFlags flags =
						ImGuiWindowFlags_NoMove |
						ImGuiWindowFlags_NoSavedSettings;
					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
					ImGui::SetNextWindowPos(popupPosition, ImGuiCond_Appearing);
					if (ImGui::BeginPopup(popUpLabel.c_str(), flags))
					{
						ImGui::PushItemWidth(calculatedWidth);
						std::string filterLabel = std::format("##sel{}", i);
						static ImGuiTextFilter filter;
						ImGui::PushItemWidth(-1);
						filter.Draw(filterLabel.c_str());
						ImGui::PopItemWidth();
						bool isFilterInFocus = ImGui::IsItemActive();
						static Texture* selectedTexture = nullptr;
						bool textureSelected1 = DrawTexturesThumbnailsWithLabel(filter, &selectedTexture, renderTargets, visibleWindowWidth);
						bool textureSelected2 = DrawTexturesThumbnailsWithLabel(filter, &selectedTexture, textures, visibleWindowWidth);
						bool textureSelected3 = DrawTexturesThumbnailsWithLabel(filter, &selectedTexture, cubemaps, visibleWindowWidth);
						if (textureSelected1 || textureSelected2 || textureSelected3)
						{
							selectedTextureProfile->AssignTexture(selectedTexture, element.slot);
							std::sort(selectedTextureProfile->textures.begin(), selectedTextureProfile->textures.end());
						}
						ImGui::PopItemWidth();
						ImGui::EndPopup();
					}
					ImGui::PopItemWidth();
					ImGui::PopStyleVar(2);
				}
				
				float last_button_x2 = ImGui::GetItemRectMax().x;
				float next_button_x2 = last_button_x2 + style.ItemSpacing.x + size.x; // Expected position if next button was on same line
				ImGui::EndGroup();
				if (i + 1 < buttons_count && next_button_x2 < visibleWindowWidth)
					ImGui::SameLine();
				else
					ImGui::AlignTextToFramePadding();
				i++;
			}
			ImGui::EndChild();

			if (ImGui::Button("Revert")) {}
			ImGui::SameLine();
			if (selectedTextureProfile != nullptr)
			{
				if (!selectedTextureProfile->path.empty())
				{
					if (ImGui::Button("Save"))
					{
						lua_State* L = luaL_newstate();
						luaL_openlibs(L);
						LuaTools::dofile(L, "resources/serialization/texture_profile_saver.lua");
						lua_getglobal(L, "SaveTextureProfile");
						if (lua_isfunction(L, -1))
						{
							lua_pushlightuserdata(L, selectedTextureProfile);
							int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
							lua_pop(L, 1);
						}
						lua_close(L);
					}
					ImGui::SameLine();
					if (ImGui::Button("Save As"))
					{
						//we need a dialog to save new files or file with paths for all of the configuration data
						selectedTextureProfile->path = std::format("resources/texture_profiles/{}.json", selectedTextureProfile->name);
						lua_State* L = luaL_newstate();
						luaL_openlibs(L);
						LuaTools::dofile(L, "resources/serialization/texture_profile_saver.lua");
						lua_getglobal(L, "SaveTextureProfile");
						if (lua_isfunction(L, -1))
						{
							lua_pushlightuserdata(L, selectedTextureProfile);
							int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
							lua_pop(L, 1);
						}
						lua_close(L);
					}
				}
				else
				{
					if (ImGui::Button("Save As"))
					{
						//we need a dialog to save new files or file with paths for all of the configuration data
						selectedTextureProfile->path = std::format("resources/texture_profiles/{}", selectedTextureProfile->name);
						lua_State* L = luaL_newstate();
						luaL_openlibs(L);
						LuaTools::dofile(L, "resources/serialization/texture_profile_saver.lua");
						lua_getglobal(L, "SaveTextureProfile");
						if (lua_isfunction(L, -1))
						{
							lua_pushlightuserdata(L, selectedTextureProfile);
							int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
							lua_pop(L, 1);
						}
						lua_close(L);
					}
				}
			}
		}
		ImGui::EndGroup();
	}
}

template<typename T>
bool Editor::DrawTexturesThumbnailsWithLabel(ImGuiTextFilter& filter, T** selectedTexture, std::unordered_map<std::string, T*>& textures, float visibleWindowWidth)
{
	float comboImageWidth = 50;
	ImVec2 comboImageSize = ImVec2(comboImageWidth, comboImageWidth);
	int buttons_count = 20000;
	int borderWidth = 2;
	bool flip = true;
	float flipCoords = 0.0f;
	if (flip)
		flipCoords = 1.0;
	ImVec2 uv_min = ImVec2(1.0f - flipCoords, 0.0f + flipCoords);                 // Top-left
	ImVec2 uv_max = ImVec2(0.0f + flipCoords, 1.0f - flipCoords);                 // Lower-right
	ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
	ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
	ImGuiStyle& style = ImGui::GetStyle();
	bool dirty = false;
	int i = 0;
	for (auto& texture : textures)
	{
		if (filter.PassFilter(texture.first.c_str())) {
			ImGui::BeginGroup();
			const bool is_selected = (*selectedTexture == texture.second);
			borderWidth = is_selected ? 2 : 0;
			if (DrawSelectableTextureThumbnail(texture.second, comboImageSize, uv_min, uv_max, borderWidth, border_col, tint_col))
			{
				*selectedTexture = texture.second;
				filter.Clear();
				dirty = true;
			}
			ImGui::Button(std::format("{}##slotbtn{}", texture.first, i).c_str(), ImVec2(comboImageSize.x + 2, 0));

			if (is_selected)
				ImGui::SetItemDefaultFocus();
			float lastItem_x2 = ImGui::GetItemRectMax().x;
			float nextItem_x2 = lastItem_x2 + style.ItemSpacing.x + comboImageSize.x;
			ImGui::EndGroup();
			if (nextItem_x2 < visibleWindowWidth)
				ImGui::SameLine();
			else
				ImGui::AlignTextToFramePadding();
		}
		i++;
	}
	return dirty;
}

template<typename T>
bool Editor::DrawSelectableTextureThumbnail(T* texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
{
	bool dirty = false;
	if (texture != nullptr)
	{
		if (texture->target != GL_TEXTURE_CUBE_MAP)
		{
			dirty = ImGui::ImageButton((void*)texture->handle, size, uv0, uv1, frame_padding, bg_col, tint_col);
		}
		else
		{
			dirty = ImGui::ImageButton((void*)-1, size, uv0, uv1, frame_padding, bg_col, tint_col);
		}
	}
	else
	{
		dirty = ImGui::ImageButton((void*)-1, size, uv0, uv1, frame_padding, bg_col, tint_col);
	}
	return dirty;
}

void Editor::RenderProfilesInspector()
{
	// Left
	static RenderProfile* selectedRenderProfile = nullptr;
	{
		ImGui::BeginChild("left pane", ImVec2(150, 0), true);

		static ImGuiTextFilter filterAvailableRenderProfiles;
		filterAvailableRenderProfiles.Draw("##filterAvailableRenderProfilesFilter", -1.0f);
		std::map<std::string, RenderProfile*> rpsSorted;
		int i = 0;
		for (auto& asset : *GraphicsStorage::assetRegistry.GetPool<RenderProfile>())
		{
			rpsSorted.emplace(std::make_pair(asset.name + "##" + std::to_string(i), &asset));
			i++;
		}
		if (ImGui::BeginListBox("##filterable_renderProfiles", ImVec2(-1, -1)))
		{
			for (auto& profile : rpsSorted)
			{
				if (filterAvailableRenderProfiles.PassFilter(profile.first.c_str()))
				{
					if (!profile.second->path.empty())
					{
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
					}
					else
					{
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
					}
					if (ImGui::Selectable(profile.first.c_str(), selectedRenderProfile == profile.second))
					{
						selectedRenderProfile = profile.second;
					}
					ImGui::PopStyleColor();
				}
			}
			ImGui::EndListBox();
		}
		ImGui::EndChild();
	}
	ImGui::SameLine();

	// Right
	{	
		ImGui::BeginGroup();
		static char name[128];
		if (CreateNewAssetUI(name))
		{
			selectedRenderProfile = CreateNewRenderProfile(name);
		}
		if (selectedRenderProfile != nullptr)
		{
			ImGui::Text(selectedRenderProfile->name.c_str());
			ImGui::Separator();
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
			ScriptSelector(selectedRenderProfile->script, "resources/render_profiles/lua");
			ImGui::EndChild();
			if (ImGui::Button("Revert")) {}
			ImGui::SameLine();

			ImGui::SameLine();
		
			if (!selectedRenderProfile->path.empty())
			{
				if (ImGui::Button("Save"))
				{
					lua_State* L = luaL_newstate();
					luaL_openlibs(L);
					LuaTools::dofile(L, "resources/serialization/render_profile_saver.lua");
					lua_getglobal(L, "SaveRenderProfile");
					if (lua_isfunction(L, -1))
					{
						lua_pushlightuserdata(L, selectedRenderProfile);
						int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
						lua_pop(L, 1);
					}
					lua_close(L);
				}
				ImGui::SameLine();
				if (ImGui::Button("Save As"))
				{
					//we need a dialog to save new files or file with paths for all of the configuration data
					selectedRenderProfile->path = std::format("resources/render_profiles/{}.json", selectedRenderProfile->name);
					lua_State* L = luaL_newstate();
					luaL_openlibs(L);
					LuaTools::dofile(L, "resources/serialization/render_profile_saver.lua");
					lua_getglobal(L, "SaveRenderProfile");
					if (lua_isfunction(L, -1))
					{
						lua_pushlightuserdata(L, selectedRenderProfile);
						int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
						lua_pop(L, 1);
					}
					lua_close(L);
				}
			}
			else
			{
				if (ImGui::Button("Save As"))
				{
					//we need a dialog to save new files or file with paths for all of the configuration data
					selectedRenderProfile->path = std::format("resources/render_profiles/{}.json", selectedRenderProfile->name);
					lua_State* L = luaL_newstate();
					luaL_openlibs(L);
					LuaTools::dofile(L, "resources/serialization/render_profile_saver.lua");
					lua_getglobal(L, "SaveRenderProfile");
					if (lua_isfunction(L, -1))
					{
						lua_pushlightuserdata(L, selectedRenderProfile);
						int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
						lua_pop(L, 1);
					}
					lua_close(L);
				}
			}
		}
		ImGui::EndGroup();
	}
}

void Editor::ObjectProfilesInspector()
{
	// Left
	static ObjectProfile* selectedProfile = nullptr;
	{
		ImGui::BeginChild("left pane", ImVec2(150, 0), true);

		static ImGuiTextFilter filterAvailableObjectProfiles;
		filterAvailableObjectProfiles.Draw("##filterAvailableObjectProfilesFilter", -1.0f);
		std::map<std::string, ObjectProfile*> opsSorted;
		int i = 0;
		for (auto& asset : *GraphicsStorage::assetRegistry.GetPool<ObjectProfile>())
		{
			opsSorted.emplace(std::make_pair(asset.name + "##" + std::to_string(i), &asset));
			i++;
		}
		if (ImGui::BeginListBox("##filterable_objectProfiles", ImVec2(-1, -1)))
		{
			for (auto& [name, profile] : opsSorted)
			{
				if (filterAvailableObjectProfiles.PassFilter(name.c_str()))
				{
					if (ImGui::Selectable(name.c_str(), selectedProfile == profile))
					{
						selectedProfile = profile;
					}
				}
			}
			ImGui::EndListBox();
		}
		ImGui::EndChild();
	}
	ImGui::SameLine();

	// Right
	{
		ImGui::BeginGroup();
		static char name[128];
		if (CreateNewAssetUI(name))
		{
			selectedProfile = CreateNewObjectProfile(name);
		}
		if (selectedProfile != nullptr)
		{
			ImGui::Text(selectedProfile->name.c_str());
			ImGui::Separator();
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
			
			if (selectedProfile->shaderBlockDatas.size() > 0)
			{
				if (ImGui::CollapsingHeader("Uniform Buffers"))
				{
					for (auto& ubd : selectedProfile->shaderBlockDatas)
					{
						ShaderBlocksInspector(ubd.shaderBlock);
						ImGui::Separator();
					}
				}
			}
			ImGui::EndChild();
			if (selectedProfile != nullptr)
			{
				if (ImGui::Button("Revert")) {}
				ImGui::SameLine();
				if (ImGui::Button("Save")) {}
			}
		}
		ImGui::EndGroup();
	}
}

bool Editor::DataRegistryInspector(DataRegistry* registry, const char* pathToLuas, void* ownerPtr)
{
	bool dirty = false;
	if (LuaProperties(registry, "objectProfile", pathToLuas, ownerPtr))
	{
		dirty = true;
	}
	ImGui::Separator();

	if (ObjectProperties(registry, "ObjectProfile"))
	{
		dirty = true;
	}
	ImGui::Separator();

	if (DataRegistryEditor(registry))
	{
		dirty = true;
	}
	return dirty;
}

void Editor::TextureParameterComboIV(Texture* texture, int propertyName, const char* comboLabel, std::unordered_map<unsigned int, const char*> options, int currentValue)
{
	if (ImGui::BeginCombo(comboLabel, options[currentValue]))
	{
		for (auto param : options)
		{
			const bool is_selected = (currentValue == param.first);
			if (ImGui::Selectable(param.second, is_selected))
			{
				glTexParameteri(texture->target, propertyName, param.first);
			}
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
}

void Editor::TextureThumbnails(std::unordered_map<std::string, Texture*>& textures, std::string& selectedName, bool flip)
{
	float flipCoords = 0.0f;
	if (flip)
		flipCoords = 1.0;
	ImVec2 uv_min = ImVec2(1.0f- flipCoords, 0.0f+ flipCoords);                 // Top-left
	ImVec2 uv_max = ImVec2(0.0f+ flipCoords, 1.0f- flipCoords);                 // Lower-right
	ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
	ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
	ImGuiStyle& style = ImGui::GetStyle();

	static ImGuiTextFilter filterAvailableTextures;
	filterAvailableTextures.Draw("##filterAvailableTextures", -1.0f);
	static int thumbnailSize = 128;
	ImGui::SliderInt("Thumbnail Size", &thumbnailSize, 4, 512);
	float windowWidth = ImGui::GetWindowWidth();
	ImGui::Separator();
	ImGui::BeginChild("textures view", ImVec2(windowWidth - 20, 0), true);
	{
		ImVec2 size = ImVec2(thumbnailSize, thumbnailSize);
		float borderWidth = 2;
		int buttons_count = textures.size();
		float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
		std::vector<std::string> names;
		for (auto& asset : textures)
		{
			names.push_back(asset.first);
		}
		std::sort(names.begin(), names.end());
		for (auto& texture : names)
		{
			if (filterAvailableTextures.PassFilter(texture.c_str()))
			{
				ImGui::BeginGroup();
				if (DrawSelectableTextureThumbnail(textures[texture], size, uv_min, uv_max, borderWidth, border_col, tint_col))
				{
					selectedName = texture;
				}
				ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + thumbnailSize);
				ImGui::TextWrapped(texture.c_str());
				ImGui::PopTextWrapPos();
				ImGui::EndGroup();
				float last_button_x2 = ImGui::GetItemRectMax().x;
				float next_button_x2 = last_button_x2 + style.ItemSpacing.x + size.x;
				if (next_button_x2 < window_visible_x2)
					ImGui::SameLine();
				else
					ImGui::AlignTextToFramePadding();
			}
		}
	}
	ImGui::EndChild();
}

void Editor::TexturesInspector(std::unordered_map<std::string, Texture*>& textures, std::string& selectedName, bool flip)
{
	// Left
	float flipCoords = 0.0f;
	if (flip)
		flipCoords = 1.0;
	ImVec2 uv_min = ImVec2(1.0f - flipCoords, 0.0f + flipCoords);                 // Top-left
	ImVec2 uv_max = ImVec2(0.0f + flipCoords, 1.0f - flipCoords);                 // Lower-right
	ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
	ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
	ImGuiStyle& style = ImGui::GetStyle();
	float prevItemSize = ImGui::CalcItemWidth();
	float windowWidth = ImGui::GetWindowWidth();
	
	{
		ImGui::BeginChild("item view", ImVec2(windowWidth * 0.6f, 0), true);
		
		TextureThumbnails(textures, selectedName, flip);
		
		ImGui::EndChild();
	}

	std::vector<Texture*> texturesToUnload;
	ImGui::SameLine();

	// Right
	{
		ImGui::BeginChild("left details");
		
		if (!selectedName.empty())
		{
			ImGui::AlignTextToFramePadding();
			ImGui::Text(selectedName.c_str());
			if (ImGui::Button("Unload Texture"))
			{
				texturesToUnload.push_back(textures[selectedName]);
			}
			ImGui::SameLine();
			if (ImGui::Button("Revert")) {}
			ImGui::SameLine();
			if (ImGui::Button("Save"))
			{
				if (!selectedName.empty())
				{
					//check defaults
					//save texture information
					Texture* texture = textures[selectedName];
					lua_State* L = luaL_newstate();
					luaL_openlibs(L);
					LuaTools::dofile(L, "resources/serialization/texture_settings_saver.lua");
					lua_getglobal(L, "SaveTextureSettings");
					if (lua_isfunction(L, -1))
					{
						lua_pushlightuserdata(L, texture);
						int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
						lua_pop(L, 1);
					}
					lua_close(L);
				}
			}

			ImGui::Separator();
		}
		for (auto texture : texturesToUnload)
		{
			std::string textureName = texture->name;
			for (auto& tp : *GraphicsStorage::assetRegistry.GetPool<TextureProfile>())
			{
				tp.RemoveTexture(texture);
			}
			textures.erase(textureName);

			std::string assetPath = std::format("resources/texture_settings/{}.json", textureName);
			lua_State* L = luaL_newstate();
			luaL_openlibs(L);
			LuaTools::dofile(L, "resources/luaexts/loadAndUnloadAssetFromList.lua");
			lua_getglobal(L, "UnloadFromList");
			if (lua_isfunction(L, -1))
			{
				lua_pushstring(L, assetPath.c_str());
				lua_pushstring(L, "resources/texture_settings_setup/texture_settings.json");
				int result = LuaTools::report(L, LuaTools::docall(L, 2, 1));
				lua_pop(L, 1);
			}
			lua_close(L);

			selectedName = "";
		}

		ImGui::BeginChild("texture details");

		if (!selectedName.empty())
		{
			ImGui::PushItemWidth(-1);
			float imageSize = ImGui::CalcItemWidth() - style.ItemSpacing.x;
			ImVec2 size = ImVec2(imageSize, imageSize);
			int borderWidth = 2;

			Texture* texture = textures[selectedName];

			ImGui::Text(selectedName.c_str());
			ImGui::Text(std::format("Path: {}", texture->path).c_str());
			ImGui::Text(std::format("TexturePath: {}", texture->texturePath).c_str());
			ImGui::Separator();
			size.y = size.y / texture->aspect;
			DrawSelectableTextureThumbnail(texture, size, uv_min, uv_max, 0, border_col, tint_col);
			ImGui::Separator();
			ImGui::PushItemWidth(ImGui::GetFontSize() * -14);
			ImGui::Text("Handle: %d", texture->handle);
			ImGui::Text("Target: %s", targets[texture->target]);
			ImGui::Text("Attachment: %s", attachments[texture->attachment]);
			ImGui::Text("Width: %d", texture->width);
			ImGui::Text("Height: %d", texture->height);
			ImGui::Text("Data Type: %s", types[texture->type]);
			ImGui::Text("Channels(Format): %s", formats[texture->format]);
			ImGui::Text("Channels Format(Internal Format): %s", internalFormats[texture->internalFormat]);
			ImGui::Text("Mip: %d", texture->level);
			texture->Bind();

			static float textureAnisotropy;
			glGetTexParameterfv(texture->target, GL_TEXTURE_MAX_ANISOTROPY_EXT, &textureAnisotropy);
			float maxAnisotropy = 16.0f;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
			if (ImGui::DragFloat("Anisotropy", &textureAnisotropy, 1, 1, maxAnisotropy))
			{
				glTexParameterf(texture->target, GL_TEXTURE_MAX_ANISOTROPY_EXT, textureAnisotropy);
			}

			static int depthStencilMode = GL_DEPTH_COMPONENT;
			glGetTexParameteriv(texture->target, GL_DEPTH_STENCIL_TEXTURE_MODE, &depthStencilMode);
			TextureParameterComboIV(texture, GL_DEPTH_STENCIL_TEXTURE_MODE, "Depth-Stencil Mode", depthStencilModes, depthStencilMode);
			
			static int imageFormatCompatibilityType = GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE;
			glGetTexParameteriv(texture->target, GL_IMAGE_FORMAT_COMPATIBILITY_TYPE, &imageFormatCompatibilityType);
			TextureParameterComboIV(texture, GL_IMAGE_FORMAT_COMPATIBILITY_TYPE, "Image Format Compatibility", imageFormatCompatibilityTypes, imageFormatCompatibilityType);
			
			static int textureBaseLevel = 0;
			glGetTexParameteriv(texture->target, GL_TEXTURE_BASE_LEVEL, &textureBaseLevel);
			ImGui::Text(std::string("Min Mip Level: " + std::to_string(textureBaseLevel)).c_str());
			
			static int textureMaxLevel = 0;
			glGetTexParameteriv(texture->target, GL_TEXTURE_MAX_LEVEL, &textureMaxLevel);
			ImGui::Text(std::string("Max Mip Level: " + std::to_string(textureMaxLevel)).c_str());

			static glm::vec4 borderColor;
			glGetTexParameterfv(texture->target, GL_TEXTURE_BORDER_COLOR, &borderColor.x);
			if (ImGui::ColorEdit4("Texture Border", &borderColor.x))
			{
				glTexParameterfv(texture->target, GL_TEXTURE_BORDER_COLOR, &borderColor.x);
			}
			
			if (std::string(internalFormats[texture->internalFormat]).find("GL_DEPTH") != std::string::npos)
			{
				static int compareMode = GL_COMPARE_REF_TO_TEXTURE;
				glGetTexParameteriv(texture->target, GL_TEXTURE_COMPARE_MODE, &compareMode);
				TextureParameterComboIV(texture, GL_TEXTURE_COMPARE_MODE, "Compare Mode", compareModes, compareMode);

				if (compareMode == GL_COMPARE_REF_TO_TEXTURE)
				{
					static int compareFunc = GL_LEQUAL;
					glGetTexParameteriv(texture->target, GL_TEXTURE_COMPARE_FUNC, &compareFunc);
					TextureParameterComboIV(texture, GL_TEXTURE_COMPARE_FUNC, "Compare Func", compareFuncs, compareFunc);
				}
			}

			static int immutableTextureFormat;
			glGetTexParameteriv(texture->target, GL_TEXTURE_IMMUTABLE_FORMAT, &immutableTextureFormat);
			ImGui::Text(std::string("Is Immutable Format: " + std::string(immutableFormat[immutableTextureFormat])).c_str());

			static int immutableTextureLevels;
			glGetTexParameteriv(texture->target, GL_TEXTURE_IMMUTABLE_LEVELS, &immutableTextureLevels);
			ImGui::Text(std::string("Nr of Immutable Levels: " + std::to_string(immutableTextureLevels)).c_str());

			static int textureMinlod;
			glGetTexParameteriv(texture->target, GL_TEXTURE_MIN_LOD, &textureMinlod);
			if (ImGui::DragInt("Min LOD", &textureMinlod, 1, -1000, 1000))
			{
				glTexParameteri(texture->target, GL_TEXTURE_MIN_LOD, textureMinlod);
			}

			static int textureMaxlod;
			glGetTexParameteriv(texture->target, GL_TEXTURE_MAX_LOD, &textureMaxlod);
			if (ImGui::DragInt("Max LOD", &textureMaxlod, 1, -1000, 1000))
			{
				glTexParameteri(texture->target, GL_TEXTURE_MAX_LOD, textureMaxlod);
			}

			static int previewMip = 0;
			if (ImGui::SliderInt("Mip", &previewMip, 0, 16))
			{
				glTexParameteri(texture->target, GL_TEXTURE_MIN_LOD, previewMip);
			}
			
			static float texturelodBias;
			glGetTexParameterfv(texture->target, GL_TEXTURE_LOD_BIAS, &texturelodBias);
			float maxBias;
			glGetFloatv(GL_MAX_TEXTURE_LOD_BIAS, &maxBias);
			if (ImGui::SliderFloat("LOD Bias", &texturelodBias, -maxBias, maxBias))
			{
				glTexParameterf(texture->target, GL_TEXTURE_LOD_BIAS, texturelodBias);
			}

			static int textureMinFilter = GL_NEAREST;
			glGetTexParameteriv(texture->target, GL_TEXTURE_MIN_FILTER, &textureMinFilter);
			TextureParameterComboIV(texture, GL_TEXTURE_MIN_FILTER, "Min Filter", minFilters, textureMinFilter);

			static int textureMagFilter = GL_NEAREST;
			glGetTexParameteriv(texture->target, GL_TEXTURE_MAG_FILTER, &textureMagFilter);
			TextureParameterComboIV(texture, GL_TEXTURE_MAG_FILTER, "Mag Filter", magFilters, textureMagFilter);

			static int textureSwizzleR = GL_RED;
			glGetTexParameteriv(texture->target, GL_TEXTURE_SWIZZLE_R, &textureSwizzleR);
			TextureParameterComboIV(texture, GL_TEXTURE_SWIZZLE_R, "Swizzle R", swizzles, textureSwizzleR);

			static int textureSwizzleG = GL_GREEN;
			glGetTexParameteriv(texture->target, GL_TEXTURE_SWIZZLE_G, &textureSwizzleG);
			TextureParameterComboIV(texture, GL_TEXTURE_SWIZZLE_G, "Swizzle G", swizzles, textureSwizzleG);

			static int textureSwizzleB = GL_BLUE;
			glGetTexParameteriv(texture->target, GL_TEXTURE_SWIZZLE_B, &textureSwizzleB);
			TextureParameterComboIV(texture, GL_TEXTURE_SWIZZLE_B, "Swizzle B", swizzles, textureSwizzleB);

			static int textureSwizzleA = GL_ALPHA;
			glGetTexParameteriv(texture->target, GL_TEXTURE_SWIZZLE_A, &textureSwizzleA);
			TextureParameterComboIV(texture, GL_TEXTURE_SWIZZLE_A, "Swizzle A", swizzles, textureSwizzleA);

			/*
			//swizzle everything with one function call
			static int Swizzle[] = { GL_RED, GL_GREEN, GL_BLUE , GL_ALPHA };
			bool swizzleChanged = false;
			if (ImGui::BeginCombo("##swizzleRChannel", swizzles[textureSwizzleR]))
			{
				for (auto param : swizzles)
				{
					const bool is_selected = (textureSwizzleR == param.first);
					if (ImGui::Selectable(param.second, is_selected))
					{
						Swizzle[0] = param.first;
						swizzleChanged = true;
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();
			if (ImGui::BeginCombo("##swizzleGChannel", swizzles[textureSwizzleG]))
			{
				for (auto param : swizzles)
				{
					const bool is_selected = (textureSwizzleG == param.first);
					if (ImGui::Selectable(param.second, is_selected))
					{
						Swizzle[1] = param.first;
						swizzleChanged = true;
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();
			if (ImGui::BeginCombo("##swizzleBChannel", swizzles[textureSwizzleB]))
			{
				for (auto param : swizzles)
				{
					const bool is_selected = (textureSwizzleB == param.first);
					if (ImGui::Selectable(param.second, is_selected))
					{
						Swizzle[2] = param.first;
						swizzleChanged = true;
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();
			if (ImGui::BeginCombo("##swizzleAChannel", swizzles[textureSwizzleA]))
			{
				for (auto param : swizzles)
				{
					const bool is_selected = (textureSwizzleA == param.first);
					if (ImGui::Selectable(param.second, is_selected))
					{
						Swizzle[3] = param.first;
						swizzleChanged = true;
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if (swizzleChanged)
			{
				glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, Swizzle);
			}
			*/
			
			static int textureTarget = GL_TEXTURE_2D;
			glGetTexParameteriv(texture->target, GL_TEXTURE_TARGET, &textureTarget);
			TextureParameterComboIV(texture, GL_TEXTURE_TARGET, "Target", targets, textureTarget);
			
			static int textureViewMinLayer;
			glGetTexParameteriv(texture->target, GL_TEXTURE_VIEW_MIN_LAYER, &textureViewMinLayer);
			ImGui::Text(std::string("Min Layer: " + std::to_string(textureViewMinLayer)).c_str());

			static int textureViewNumOfLayers;
			glGetTexParameteriv(texture->target, GL_TEXTURE_VIEW_NUM_LAYERS, &textureViewNumOfLayers);
			ImGui::Text(std::string("Nr of Layers: " + std::to_string(textureViewNumOfLayers)).c_str());

			static int textureViewMinLevel;
			glGetTexParameteriv(texture->target, GL_TEXTURE_VIEW_MIN_LEVEL, &textureViewMinLevel);
			ImGui::Text(std::string("Min Level: " + std::to_string(textureViewMinLevel)).c_str());

			static int textureViewNumOfLevels;
			glGetTexParameteriv(texture->target, GL_TEXTURE_VIEW_NUM_LEVELS, &textureViewNumOfLevels);
			ImGui::Text(std::string("Nr of Levels: " + std::to_string(textureViewNumOfLevels)).c_str());

			static int textureWrapS = GL_REPEAT;
			glGetTexParameteriv(texture->target, GL_TEXTURE_WRAP_S, &textureWrapS);
			TextureParameterComboIV(texture, GL_TEXTURE_WRAP_S, "Wrap S", wrapFunctions, textureWrapS);

			static int textureWrapT = GL_REPEAT;
			glGetTexParameteriv(texture->target, GL_TEXTURE_WRAP_T, &textureWrapT);
			TextureParameterComboIV(texture, GL_TEXTURE_WRAP_T, "Wrap T", wrapFunctions, textureWrapT);

			static int textureWrapR = GL_REPEAT;
			glGetTexParameteriv(texture->target, GL_TEXTURE_WRAP_R, &textureWrapR);
			TextureParameterComboIV(texture, GL_TEXTURE_WRAP_R, "Wrap R", wrapFunctions, textureWrapR);
			
			ImGui::PopItemWidth();
		}
		
		ImGui::EndChild();
		ImGui::EndChild();
	}
}

void Editor::RenderBufferInspector(RenderBuffer* rbuffer)
{
	if (rbuffer != nullptr)
	{
		ImGui::PushItemWidth(-1);

		ImGui::Text(rbuffer->name.c_str());
		ImGui::Text("Path: %s", rbuffer->path.c_str());
		ImGui::Separator();
		ImGui::PushItemWidth(ImGui::GetFontSize() * -14);
		ImGui::Text("Handle: %d", rbuffer->handle);
		ImGui::Text(targets[GL_RENDERBUFFER]);
		ImGui::Text(attachments[rbuffer->attachment]);
		ImGui::Text("Width: %d", rbuffer->width);
		ImGui::Text("Height: %d", rbuffer->height);
		ImGui::Text("Channels Format: %s", internalFormats[rbuffer->internalFormat]);

		ImGui::PopItemWidth();
	}
	ImGui::Separator();
	if (ImGui::Button("Revert")) {}
	ImGui::SameLine();
	if (ImGui::Button("Save"))
	{
		if (rbuffer != nullptr)
		{
			//check defaults
			//save texture information
			lua_State* L = luaL_newstate();
			luaL_openlibs(L);
			LuaTools::dofile(L, "resources/serialization/render_buffer_settings_saver.lua");
			lua_getglobal(L, "SaveRenderBufferSettings");
			if (lua_isfunction(L, -1))
			{
				lua_pushlightuserdata(L, rbuffer);
				int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
				lua_pop(L, 1);
			}
			lua_close(L);
		}
	}
}

void Editor::TextureThumbnailsBasic(std::vector<Texture*>& textures, Texture* selected)
{
	bool flip = true;
	float flipCoords = 0.0f;
	if (flip)
		flipCoords = 1.0;
	ImVec2 uv_min = ImVec2(1.0f - flipCoords, 0.0f + flipCoords);                 // Top-left
	ImVec2 uv_max = ImVec2(0.0f + flipCoords, 1.0f - flipCoords);                 // Lower-right
	ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
	ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
	ImGuiStyle& style = ImGui::GetStyle();

	static ImGuiTextFilter filterAvailableTextures;
	filterAvailableTextures.Draw("##filterAvailableTextures", -1.0f);
	ImGui::Separator();
	{
		static int thumbnailSize = 128;
		ImGui::SliderInt("Thumbnail Size", &thumbnailSize, 4, 512);

		ImVec2 size = ImVec2(thumbnailSize, thumbnailSize);
		float borderWidth = 2;
		float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
		int i = 0;
	
		//std::sort(std::begin(textures), std::end(textures),
		//	[](Texture* a, Texture* b) { return a->name < b->name; });
		for (auto texture : textures)
		{
			if (filterAvailableTextures.PassFilter(texture->name.c_str()))
			{
				ImGui::BeginGroup();
				if (DrawSelectableTextureThumbnail(texture, size, uv_min, uv_max, borderWidth, border_col, tint_col))
				{
					selected = texture;
				}
				ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + thumbnailSize);
				ImGui::TextWrapped(texture->name.c_str());
				ImGui::PopTextWrapPos();
				ImGui::EndGroup();
				float last_button_x2 = ImGui::GetItemRectMax().x;
				float next_button_x2 = last_button_x2 + style.ItemSpacing.x + size.x;
				if (next_button_x2 < window_visible_x2)
					ImGui::SameLine();
				else
					ImGui::AlignTextToFramePadding();
				i++;
			}
		}
	}
}

void Editor::TextureBasicDetails(Texture* texture)
{
	// Left
	bool flip = true;
	float flipCoords = 0.0f;
	if (flip)
		flipCoords = 1.0;
	ImVec2 uv_min = ImVec2(1.0f - flipCoords, 0.0f + flipCoords);                 // Top-left
	ImVec2 uv_max = ImVec2(0.0f + flipCoords, 1.0f - flipCoords);                 // Lower-right
	ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
	ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
	ImGuiStyle& style = ImGui::GetStyle();

	ImGui::PushItemWidth(-1);
	float imageSize = ImGui::CalcItemWidth() - style.ItemSpacing.x;
	ImVec2 size = ImVec2(imageSize, imageSize);
	int borderWidth = 2;

	if (texture != nullptr)
	{
		ImGui::Text(texture->name.c_str());
		ImGui::Separator();
		size.y = size.y / texture->aspect;
		DrawSelectableTextureThumbnail(texture, size, uv_min, uv_max, 0, border_col, tint_col);
		ImGui::Separator();
		ImGui::Text("Handle: %d", texture->handle);
		ImGui::Text("Target: %s", targets[texture->target]);
		ImGui::Text(attachments[texture->attachment]);
		ImGui::Text("Width: %d", texture->width);
		ImGui::Text("Height: %d", texture->height);
		ImGui::Text("Data Type: %s", types[texture->type]);
		ImGui::Text("Channels: %s", formats[texture->format]);
		ImGui::Text("Channels Format: %s", internalFormats[texture->internalFormat]);
		ImGui::Text("Mip: %d", texture->level);
	}
}

void Editor::TexturesInspectorBasic(std::vector<Texture*>& textures, bool reset)
{
	
	static Texture* selectedTexture = nullptr;
	if (reset) selectedTexture = nullptr;

	float prevItemSize = ImGui::CalcItemWidth();
	float windowWidth = ImGui::GetWindowWidth();
	float windowHeight = ImGui::GetWindowHeight();
	{

		ImGui::BeginChild("item view list", ImVec2(windowWidth*0.6f, windowHeight*0.4f), true);
		
		TextureThumbnailsBasic(textures, selectedTexture);

		ImGui::EndChild();
	}
	ImGui::SameLine();

	// Right
	{
		ImGui::BeginGroup();
		ImGui::BeginChild("left pane details", ImVec2(windowWidth*0.4f, windowHeight*0.4f));
		TextureBasicDetails(selectedTexture);
		ImGui::EndChild();
		ImGui::EndGroup();
	}
}

void Editor::TextureThumbnailsBasicRB(std::vector<RenderBuffer*>& textures, RenderBuffer* selected)
{
	ImGuiStyle& style = ImGui::GetStyle();

	static ImGuiTextFilter filterAvailableTextures;
	filterAvailableTextures.Draw("##filterAvailableTextures", -1.0f);
	ImGui::Separator();
	{
		static int thumbnailSize = 128;
		ImGui::SliderInt("Thumbnail Size", &thumbnailSize, 4, 512);

		ImVec2 size = ImVec2(thumbnailSize, thumbnailSize);
		float borderWidth = 2;
		float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
		int i = 0;
		for (auto& texture : textures)
		{
			if (filterAvailableTextures.PassFilter(texture->name.c_str()))
			{
				ImGui::BeginGroup();
				if (ImGui::Button(texture->name.c_str(), size))
				{
					selected = texture;
				}
				ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + thumbnailSize);
				ImGui::TextWrapped(texture->name.c_str());
				ImGui::PopTextWrapPos();
				ImGui::EndGroup();
				float last_button_x2 = ImGui::GetItemRectMax().x;
				float next_button_x2 = last_button_x2 + style.ItemSpacing.x + size.x;
				if (next_button_x2 < window_visible_x2)
					ImGui::SameLine();
				else
					ImGui::AlignTextToFramePadding();
				i++;
			}
		}
	}
}

void Editor::RenderBufferBasicDetails(RenderBuffer* renderBuffer)
{
	ImGuiStyle& style = ImGui::GetStyle();

	ImGui::PushItemWidth(-1);
	float imageSize = ImGui::CalcItemWidth() - style.ItemSpacing.x;
	ImVec2 size = ImVec2(imageSize, imageSize);
	int borderWidth = 2;
	RenderBuffer* rbuffer = nullptr;

	if (rbuffer != nullptr)
	{
		ImGui::Text(renderBuffer->name.c_str());
		ImGui::Separator();
		ImGui::Text("Handle: %d", rbuffer->handle);
		ImGui::Text("Target: %s", targets[GL_RENDERBUFFER]);
		ImGui::Text(attachments[rbuffer->attachment]);
		ImGui::Text("Width: %d", rbuffer->width);
		ImGui::Text("Height: %d", rbuffer->height);
		ImGui::Text("Channels Format: %s", internalFormats[rbuffer->internalFormat]);
	}
}

void Editor::RenderBuffersInspectorBasic(std::vector<RenderBuffer*>& rbuffers, bool reset)
{
	ImGui::PushID("RenderBuffer");
	// Left
	
	static RenderBuffer* selectedRBuffer = nullptr;
	if (reset) selectedRBuffer = nullptr;

	float prevItemSize = ImGui::CalcItemWidth();
	float windowWidth = ImGui::GetWindowWidth();
	float windowHeight = ImGui::GetWindowHeight();
	{
		ImGui::BeginGroup();

		ImGui::BeginChild("item view list", ImVec2(windowWidth*0.6f, windowHeight*0.4f), true);
		
		TextureThumbnailsBasicRB(rbuffers, selectedRBuffer);

		ImGui::EndChild();
		ImGui::EndGroup();
	}
	ImGui::SameLine();

	// Right
	{
		ImGui::BeginGroup();
		ImGui::BeginChild("left pane details", ImVec2(windowWidth*0.4f, windowHeight*0.4f));
		RenderBufferBasicDetails(selectedRBuffer);
		ImGui::EndChild();
		ImGui::EndGroup();
	}
	ImGui::PopID();
}

void Editor::AllTexturesInMemoryInspector()
{
	// Left
	ImVec2 uv_min = ImVec2(1.0f, 0.0f);                 // Top-left
	ImVec2 uv_max = ImVec2(0.0f, 1.0f);                 // Lower-right
	ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
	ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
	ImGuiStyle& style = ImGui::GetStyle();

	static std::string selectedName;
	static GLint selectedHandle;
	float prevItemSize = ImGui::CalcItemWidth();
	float windowWidth = ImGui::GetWindowWidth();
	{
		ImGui::BeginGroup();

		ImGui::BeginChild("item view list", ImVec2(windowWidth*0.6f, 0), true);
		//ImGui::Text(selectedName.c_str());
		static ImGuiTextFilter filterAvailableTextures;
		filterAvailableTextures.Draw("##filterAvailableTextures", -1.0f);
		
		ImGui::Separator();
		{
			float borderWidth = 2;
			static int thumbnailSize = 128;
			ImGui::SliderInt("Thumbnail Size", &thumbnailSize, 4, 512);

			ImVec2 size = ImVec2(thumbnailSize, thumbnailSize);

			float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
			std::vector<int> textureIds;
			for (int i = 0; i < 500000; i++)
			{
				if (glIsTexture(i))
				{
					textureIds.push_back(i);
				}
			}
			for (auto id : textureIds)
			{
				std::string textureName = std::to_string(id);
				for (auto& tex : textures)
				{
					if (tex.second->handle == id)
					{
						textureName = tex.first;
					}
				}
				for (auto& tex : renderTargets)
				{
					if (tex.second->handle == id)
					{
						textureName = tex.first;
					}
				}
				for (auto& tex : cubemaps)
				{
					if (tex.second->handle == id)
					{
						textureName = tex.first;
					}
				}
				for (auto& tex : renderBuffers)
				{
					if (tex.second->handle == id)
					{
						textureName = tex.first;
					}
				}
				ImGui::BeginGroup();
				GLenum target;
				glGetTextureParameteriv(id, GL_TEXTURE_TARGET, (GLint*)&target);
				if (target != GL_TEXTURE_CUBE_MAP)
				{
					if (ImGui::ImageButton((void*)id, size, uv_min, uv_max, borderWidth, border_col, tint_col))
					{
						selectedName = textureName;
						selectedHandle = id;
					}
				}
				else
				{
					if (ImGui::ImageButton((void*)-id, size, uv_min, uv_max, borderWidth, border_col, tint_col))
					{
						selectedName = textureName;
						selectedHandle = id;
					}
				}

				ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + thumbnailSize);
				ImGui::TextWrapped(textureName.c_str());
				ImGui::PopTextWrapPos();
				ImGui::EndGroup();
				float last_button_x2 = ImGui::GetItemRectMax().x;
				float next_button_x2 = last_button_x2 + style.ItemSpacing.x + size.x;
				if (next_button_x2 < window_visible_x2)
					ImGui::SameLine();
				else
					ImGui::AlignTextToFramePadding();
			}
		}
		ImGui::EndChild();
		ImGui::EndGroup();
	}
	ImGui::SameLine();

	// Right
	{
		ImGui::BeginGroup();
		ImGui::BeginChild("left pane details");
		if (!selectedName.empty())
		{
			ImGui::PushItemWidth(-1);
			float imageSize = ImGui::CalcItemWidth() - style.ItemSpacing.x;
			ImVec2 size = ImVec2(imageSize, imageSize);
			int borderWidth = 2;
			GLint textureId = selectedHandle;
			GLenum target;
			glGetTextureParameteriv(textureId, GL_TEXTURE_TARGET, (GLint*)&target);
			ImGui::Text(selectedName.c_str());
			ImGui::Separator();
			if (target != GL_TEXTURE_CUBE_MAP)
			{
				ImGui::Image((void*)textureId, size, uv_min, uv_max, tint_col, border_col);
			}
			else
			{
				ImGui::Image((void*)-textureId, size, uv_min, uv_max, tint_col, border_col);
			}
			ImGui::Separator();
			ImGui::Text("Name: %s", selectedName.c_str());
			ImGui::Text(std::format("Handle: {}", selectedHandle).c_str());
			int w, h;
			int miplevel = 0;
			glBindTexture(target, textureId);
			if (target != GL_TEXTURE_CUBE_MAP)
			{
				glGetTexLevelParameteriv(target, miplevel, GL_TEXTURE_WIDTH, &w);
				glGetTexLevelParameteriv(target, miplevel, GL_TEXTURE_HEIGHT, &h);
			}
			else
			{
				ImGui::Text("Cube Map");
				ImGui::Text("Positive X");
				glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, miplevel, GL_TEXTURE_WIDTH, &w);
				glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, miplevel, GL_TEXTURE_HEIGHT, &h);
			}
			ImGui::Text("Width: %d", w);
			ImGui::Text("Height: %d", h);
		}
		ImGui::EndChild();
		ImGui::EndGroup();
	}
}

void Editor::MiscTools()
{
	ImGui::Begin("UI", &showMiscTools, ImGuiWindowFlags_MenuBar);

	ImGui::Text("FPS %.3f", 1.0 / Times::Instance()->deltaTime);
	ImGui::Text("Average FPS %.3f", 1.0 / Times::Instance()->averageDeltaTime);
	ImGui::Text("Draw calls %d", Render::Instance()->totalNrOfDrawCalls);
	float start = 0;
	float stop = 360;
	if (ImGui::CollapsingHeader("Camera"))
	{
		static float camHA = testCamera->horizontalAngle;
		static float camVA = testCamera->verticalAngle;
		ImGui::DragFloat("cameraNear", &testCamera->near, 0.1f);
		ImGui::DragFloat("cameraFar", &testCamera->far, 0.1f);
		ImGui::DragFloat("cameraFov", &testCamera->fov, 1.0f);
		ImGui::DragFloat("cameraShadowNearOffset", &testCamera->shadowNearOffset, 1.0f);
		ImGui::DragFloat("cameraShadowDistace", &testCamera->shadowDistance, 1.0f);
		if (ImGui::DragFloat("cameraHorizontal", &camHA, 0.05f))
		{
			testCamera->horizontalAngle = camHA;
		}
		if (ImGui::DragFloat("cameraVertical", &camVA, 0.05f))
		{
			testCamera->verticalAngle = camVA;
		}
		static glm::vec3 camPos = testCamera->GetPosition();
		glm::vec3 curPos = testCamera->GetPosition();
		ImGui::DragFloat3("currentCamPos", &curPos.x, 1.0f);
		if (ImGui::DragFloat3("camPos", &camPos.x, 1.0f))
		{
			testCamera->SetPosition(camPos);
		}
	}
	if (ImGui::CollapsingHeader("Tools"))
	{
		if (ImGui::Button("Reload Shaders")) {
			//GraphicsManager::ReloadShader("directionalLightPBR");
			GraphicsManager::ReloadShaders();
		}
		if (ImGui::Button("Select First")) {
			firstObject = lastPickedObject;
		}
		if (ImGui::Button("Select Second")) {
			secondObject = lastPickedObject;
		}
		if (ImGui::Button("Reparent In Place")) {
			firstObject->node->ParentInPlace(secondObject->node);
		}

		if (ImGui::Button("Set Dynamic")) {
			SceneGraph::Instance()->SwitchObjectMovableMode(lastPickedObject, true);
		}
		if (ImGui::Button("Set Static")) {
			SceneGraph::Instance()->SwitchObjectMovableMode(lastPickedObject, false);
		}
		if (firstObject)
		{
			//ImGui::Text("Object1: %s ID: %d", typeid(firstObject).name(), firstObject->ID);
		}
		if (secondObject)
		{
			//ImGui::Text("Object2: %s ID: %d", typeid(secondObject).name(), secondObject->ID);
		}
		if (lastPickedObject)
		{
			//ImGui::Text("LastObject: %s ID: %d", typeid(lastPickedObject).name(), lastPickedObject->ID);
		}
	}

	if (ImGui::CollapsingHeader("Post")) {
		//bool hdrEnabled = Render::Instance()->pb.hdrEnabled;
		//bool bloomEnabled = Render::Instance()->pb.bloomEnabled;
		ImGui::Text("Scene Object Properties");
		Object* scene = nullptr;
		for (auto& obj : *GraphicsStorage::assetRegistry.GetPool<Object>())
		{
			if (strcmp(obj.name.c_str(), "scene") == 0)
			{
				scene = &obj;
				break;
			}
		}
		//ObjectProfile* sceneOp = nullptr;
		//if (scene != nullptr)
		//{
		//	sceneOp = scene->GetComponent<ObjectProfile>();
		//}
		//DataRegistry* sceneDr = sceneOp != nullptr ? &sceneOp->registry : nullptr;
		if (scene != nullptr)
			if (DataRegistryEditor(&scene->registry))
		//{
		//	for (auto& matSq : scene->materials)
		//	{
		//		for (auto mat : matSq)
		//		{
		//			if (mat->op != nullptr)
		//			{
		//				mat->op->UpdateProfileFromDataRegistry(*sceneDr);
		//			}
		//		}
		//	}
		//}
		ImGui::Checkbox("Post Effects:", &post);
		//ImGui::Checkbox("HDR", &hdrEnabled);
		//ImGui::Checkbox("Bloom", &bloomEnabled);
		//ImGui::SliderFloat("Bloom Intensity", &Render::Instance()->pb.bloomIntensity, 0.0f, 5.f);
		//ImGui::SliderFloat("Exposure", &Render::Instance()->pb.exposure, 0.0f, 5.0f);
		//ImGui::SliderFloat("Gamma", &Render::Instance()->pb.gamma, 0.0f, 5.0f);
		//ImGui::SliderFloat("Contrast", &Render::Instance()->pb.contrast, -5.0f, 5.0f);
		//ImGui::SliderFloat("Brightness", &Render::Instance()->pb.brightness, -5.0f, 5.0f);
		ImGui::SliderFloat("Bloom Blur Size", &blurBloomSize, 0.0f, 10.0f);
		ImGui::SliderInt("Bloom Level", &bloomLevel, 0, 3);
		//ImGui::SliderFloat("Fov", &fov, 0.0f, 180.f);
		//ImGui::SliderFloat("Near plane", &near, 0.0f, 5.f);
		//ImGui::SliderFloat("Far plane", &far, 0.0f, 5000.f);
		//Render::Instance()->pb.hdrEnabled = hdrEnabled;
		//Render::Instance()->pb.bloomEnabled = bloomEnabled;
	}
	
	/*
	if (ImGui::CollapsingHeader("Lights")) {
		if (pointLightTest != nullptr)
		{
			if (ImGui::TreeNode("Point Light"))
			{
				//if (ImGui::TreeNode("Child windows"))
				//{
				ImGui::Checkbox("PointCastShadow", &pointLightCompTest->shadowMapActive);
				ImGui::Checkbox("PointBlurShadow", &pointLightCompTest->shadowMapBlurActive);
				ImGui::SliderFloat("PointFov", &pointLightCompTest->fov, start, stop);
				ImGui::SliderFloat("Point Power", &pointLightCompTest->properties.power, start, stop);
				ImGui::SliderFloat("Point Diffuse", &pointLightCompTest->properties.diffuse, start, stop);
				ImGui::SliderFloat("Point Specular", &pointLightCompTest->properties.specular, start, stop);
				ImGui::ColorEdit3("Point Color", (float*)&pointLightCompTest->properties.color);
				ImGui::SliderFloat("PointSize", &pointScale, 1, 1000);
				ImGui::SliderFloat("PointPosX", &pposX, -100, 100);
				ImGui::SliderFloat("PointPosY", &pposY, -100, 100);
				ImGui::SliderFloat("PointPosZ", &pposZ, -100, 100);
				ImGui::TreePop();
			}
		}
		if (spotLightComp != nullptr)
		{
			if (ImGui::TreeNode("Spot Light"))
			{
				ImGui::NewLine();
				ImGui::Checkbox("SpotCastShadow", &spotLightComp->shadowMapActive);
				ImGui::Checkbox("SpotBlurShadow", &spotLightComp->shadowMapBlurActive);
				ImGui::Text("FPS %.3f", 1.0 / Times::Instance()->deltaTime);
				ImGui::SliderFloat("Spot Power", &spotLightComp->properties.power, start, stop);
				ImGui::SliderFloat("Spot Diffuse", &spotLightComp->properties.diffuse, start, stop);
				ImGui::SliderFloat("Spot Specular", &spotLightComp->properties.specular, start, stop);
				ImGui::ColorEdit3("Spot Color", (float*)&spotLightComp->properties.color);
				ImGui::SliderFloat("Spot X angle", &xAngles, start, stop);
				ImGui::SliderFloat("Spot Y angle", &yAngles, start, stop);
				ImGui::SliderFloat("Spot1PosX", &posX, -100, 100);
				ImGui::SliderFloat("Spot1PosY", &posY, -100, 100);
				ImGui::SliderFloat("Spot1PosZ", &posZ, -100, 100);

				ImGui::SliderFloat("Spot1Const", &spotLightComp->attenuation.Constant, 0, 1);
				ImGui::SliderFloat("Spot1Lin", &spotLightComp->attenuation.Linear, 0, 1);
				ImGui::SliderFloat("Spot1Exp", &spotLightComp->attenuation.Exponential, 0, 1);

				ImGui::SliderFloat("Spot CutOff", &spotLightCutOff, 0, 360);
				ImGui::SliderFloat("Spot OuterCutOff", &spotLightOuterCutOff, 0, 180);

				ImGui::SliderFloat("SpotSizeZ", &spotSZ, 1, 100);
				ImGui::TreePop();
			}
		}
		if (directionalLightComp != nullptr)
		{
			if (ImGui::TreeNode("Directional Light 1"))
			{
				ImGui::NewLine();
				ImGui::Text("DIR1:");
				ImGui::Checkbox("Dir1 CastShadow", &directionalLightComp->shadowMapActive);
				ImGui::Checkbox("Dir1 BlurShadow", &directionalLightComp->shadowMapBlurActive);
				ImGui::SliderFloat("Dir1 Power", &directionalLightComp->properties.power, start, stop);
				ImGui::SliderFloat("Dir1 Diffuse", &directionalLightComp->properties.diffuse, start, stop);
				ImGui::SliderFloat("Dir1 Specular", &directionalLightComp->properties.specular, start, stop);
				ImGui::ColorEdit3("Dir1 Color", (float*)&directionalLightComp->properties.color);
				ImGui::SliderFloat("Dir1 X angle", &xAngled, start, stop);
				ImGui::SliderFloat("Dir1 Y angle", &yAngled, start, stop);
				ImGui::SliderFloat("Dir1 Shadow Blur Size", &directionalLightComp->blurIntensity, 0.0f, 10.0f);
				ImGui::SliderInt("Dir1 Shadow Blur Level", &directionalLightComp->activeBlurLevel, 0, 3);
				ImGui::SliderFloat("Dir1 Ortho Size", &directionalLightComp->radius, 0.0f, 2000.f);
				ImGui::SliderFloat("Dir1 Shadow Fade Range", &directionalLightComp->shadowFadeRange, 0.0f, 50.f);
				//directionalLightComp2->radius = directionalLightComp->radius;
				//directionalLightComp2->shadowFadeRange = directionalLightComp->shadowFadeRange;
				ImGui::TreePop();
			}
		}

		if (directionalLightComp2 != nullptr)
		{
			if (ImGui::TreeNode("Directional Light 2"))
			{
				ImGui::NewLine();
				ImGui::Text("DIR2:");
				ImGui::Checkbox("Dir2 CastShadow", &directionalLightComp2->shadowMapActive);
				ImGui::Checkbox("Dir2 BlurShadow", &directionalLightComp2->shadowMapBlurActive);
				ImGui::SliderFloat("Dir2 Power", &directionalLightComp2->properties.power, start, stop);
				ImGui::SliderFloat("Dir2 Diffuse", &directionalLightComp2->properties.diffuse, start, stop);
				ImGui::SliderFloat("Dir2 Specular", &directionalLightComp2->properties.specular, start, stop);
				ImGui::ColorEdit3("Dir2 Color", (float*)&directionalLightComp2->properties.color);
				ImGui::SliderFloat("Dir2 X angle", &xAngled2, start, stop);
				ImGui::SliderFloat("Dir2 Y angle", &yAngled2, start, stop);
				ImGui::SliderFloat("Dir2 Shadow Blur Size", &directionalLightComp2->blurIntensity, 0.0f, 10.0f);
				ImGui::SliderInt("Dir2 Shadow Blur Level", &directionalLightComp2->activeBlurLevel, 0, 3);
				ImGui::SliderFloat("Dir2 2Ortho Size", &directionalLightComp2->radius, 0.0f, 2000.f);
				ImGui::SliderFloat("Dir2 Shadow Fade Range", &directionalLightComp2->shadowFadeRange, 0.0f, 50.f);
				ImGui::TreePop();
			}
		}

		if (directionalLightComp3 != nullptr)
		{
			if (ImGui::TreeNode("Directional Light 3"))
			{
				ImGui::NewLine();
				ImGui::Text("DIR3:");
				ImGui::Checkbox("Dir3 CastShadow", &directionalLightComp3->shadowMapActive);
				ImGui::Checkbox("Dir3 BlurShadow", &directionalLightComp3->shadowMapBlurActive);
				ImGui::SliderFloat("Dir3 X angle", &xAngled3, start, stop);
				ImGui::SliderFloat("Dir3 Y angle", &yAngled3, start, stop);
				ImGui::SliderFloat("Dir3 Shadow Blur Size", &directionalLightComp3->blurIntensity, 0.0f, 10.0f);
				ImGui::SliderInt("Dir3 Shadow Blur Level", &directionalLightComp3->activeBlurLevel, 0, 3);
				ImGui::SliderFloat("Dir3 2Ortho Size", &directionalLightComp3->radius, 0.0f, 2000.f);
				ImGui::SliderFloat("Dir3 Shadow Fade Range", &directionalLightComp3->shadowFadeRange, 0.0f, 50.f);
				ImGui::TreePop();
			}
		}
		if (directionalLightComp4 != nullptr)
		{
			if (ImGui::TreeNode("Directional Light 4"))
			{
				ImGui::NewLine();
				ImGui::Text("DIR4:");
				ImGui::Checkbox("Dir4 CastShadow", &directionalLightComp4->shadowMapActive);
				ImGui::Checkbox("Dir4 BlurShadow", &directionalLightComp4->shadowMapBlurActive);
				ImGui::SliderFloat("Dir4 X angle", &xAngled4, start, stop);
				ImGui::SliderFloat("Dir4 Y angle", &yAngled4, start, stop);
				ImGui::SliderFloat("Dir4 Shadow Blur Size", &directionalLightComp4->blurIntensity, 0.0f, 10.0f);
				ImGui::SliderInt("Dir4 Shadow Blur Level", &directionalLightComp4->activeBlurLevel, 0, 3);
				ImGui::SliderFloat("Dir4 2Ortho Size", &directionalLightComp4->radius, 0.0f, 2000.f);
				ImGui::SliderFloat("Dir4 Shadow Fade Range", &directionalLightComp4->shadowFadeRange, 0.0f, 50.f);
				ImGui::TreePop();
			}
		}
	}
	if (ImGui::CollapsingHeader("Misc")) {
		ImGui::SliderFloat("EnvAngleX", &Render::Instance()->angleX, start, stop);
		ImGui::SliderFloat("EnvAngleY", &Render::Instance()->angleY, start, stop);
		if (currentScene == scene0Loaded || currentScene == scene2Loaded)
		{
			ImGui::NewLine();
			ImGui::SliderFloat("CubeAnglesX", &xCubeAngle, start, stop);
			ImGui::SliderFloat("CubeAnglesY", &yCubeAngle, start, stop);
			ImGui::SliderFloat("CubeAnglesZ", &zCubeAngle, start, stop);

			ImGui::SliderFloat("CubeSpecIntensity", &cubeSpecularIntensity, start, stop);
			ImGui::SliderFloat("CubeSpecShininess", &cubeShininess, start, stop);
		}
	}
	*/
	if (ImGui::CollapsingHeader("Objects")) {
		if (ImGui::TreeNode("Loaded Objects")) {
			for (auto& obj : *GraphicsStorage::assetRegistry.GetPool<Object>())
			{
				ImGui::Text(obj.name.c_str());
				if (ImGui::TreeNode(&obj.name, "Materials")) {
					for (auto& matSq : obj.materials)
					{
						for (auto mat : matSq)
						{
							ImGui::Text(mat->name.c_str());
						}
						ImGui::Separator();
					}
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Objects in Frustum")) {
			for (auto& obj : SceneGraph::Instance()->objectsInFrustum)
			{
				ImGui::Text(obj->name.c_str());
			}
			ImGui::TreePop();
		}
	}

	ImGui::End();
}

void Editor::OverlayStats()
{
	const float DISTANCE = 20.0f;
	static int corner = 0;
	ImGuiIO& io = ImGui::GetIO();
	if (corner != -1)
	{
		ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
		ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	}
	ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	if (corner != -1)
		window_flags |= ImGuiWindowFlags_NoMove;
	if (ImGui::Begin("Example: Simple overlay", &showOverlayStats, window_flags))
	{
		ImGui::Text("Stats");

		static std::pair<std::string, Texture*> texturePair(*renderTargets.begin());

		ImVec2 uv_min = ImVec2(1.0f, 0.0f);                 // Top-left
		ImVec2 uv_max = ImVec2(0.0f, 1.0f);                 // Lower-right
		ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
		ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
		ImVec2 comboImageSize = ImVec2(10, 10);

		if (texturePair.second != nullptr)
		{
			if (ImGui::BeginCombo("Textures", texturePair.first.c_str()))
			{
				for (auto& v_texture : renderTargets)
				{
					DrawSelectableTextureThumbnail(v_texture.second, comboImageSize, uv_min, uv_max, 0, border_col, tint_col);
					ImGui::SameLine();
					const bool is_selected = (texturePair.first == v_texture.first);
					if (ImGui::Selectable(v_texture.first.c_str(), is_selected))
					{
						texturePair = v_texture;
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				for (auto& texture : textures)
				{
					DrawSelectableTextureThumbnail(texture.second, comboImageSize, uv_min, uv_max, 0, border_col, tint_col);
					ImGui::SameLine();
					const bool is_selected = (texturePair.first == texture.first);
					if (ImGui::Selectable(texture.first.c_str(), is_selected))
					{
						texturePair = texture;
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			float imageSize = 30;
			ImVec2 size = ImVec2(imageSize, imageSize);
			DrawSelectableTextureThumbnail(texturePair.second, size, uv_min, uv_max, 0, border_col, tint_col);
		}

		ImGui::Text("FPS %.3f", 1.0 / Times::Instance()->deltaTime);
		ImGui::Text("Average FPS %.3f", 1.0 / Times::Instance()->averageDeltaTime);
		ImGui::Text("Draw calls %d", Render::Instance()->totalNrOfDrawCalls);
		if (ImGui::Checkbox("VSync Enabled", &vsyncEnabled)) glfwSwapInterval(vsyncEnabled);
		ImGui::Checkbox("Toggle UI with Alt", &toggleUIonAlt);
		ImGui::Separator();
		ImGui::Checkbox("Mouse Position in Different Coordinate Systems", &showOverlayMouseInfo);

		float maxValue = 1.0 / Times::Instance()->averageDeltaTime;
		//static float maxValue = 70.f;
		//ImGui::DragFloat("Zoom", &maxValue, 1.f, 0.f, 2000.f);
		static float values[90] = {};
		static int values_offset = 0;

		values[values_offset] = 1.0 / Times::Instance()->deltaTimeF;
		values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);

		// Plots can display overlay texts
		// (in this example, we will display an average value)
		{
			float average = 0.0f;
			for (int n = 0; n < IM_ARRAYSIZE(values); n++)
			{
				average += values[n];
			}
			average /= (float)IM_ARRAYSIZE(values);
			char overlay[32];
			sprintf(overlay, "avg %f", average);
			ImGui::PlotLines("FPS", values, IM_ARRAYSIZE(values), values_offset, overlay, maxValue , -1.0f, ImVec2(0, 40.0f));
		}

		if (showOverlayMouseInfo)
		{
			ImGui::Separator();
			if (ImGui::IsMousePosValid())
			{
				float windowHeight = CameraManager::Instance()->GetCurrentCamera()->windowHeight;
				float windowWidth = CameraManager::Instance()->GetCurrentCamera()->windowWidth;
				
				ImGui::Text("ViewPort Origo(Top-Left) Range(0-width,0-height) : (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
				ImGui::Text("OpenGL,Texture,gl_FragCoord Origo(Bottom-Left) Range(0-width,0-height): (%.1f,%.1f)", io.MousePos.x, windowHeight - io.MousePos.y);
				glm::vec4 pos = glm::vec4(io.MousePos.x, windowHeight - io.MousePos.y, 0, 1);
				glm::vec4 convertedPosToGlPosition = CameraManager::Instance()->ViewProjection*pos;
				ImGui::Text("OpenGL / ScreenSize Origo(Bottom-Left) Range(0,0,1,1): (%.1f,%.1f)", io.MousePos.x / windowWidth, (windowHeight - io.MousePos.y) / windowHeight);
				float x_ScreenSpace = io.MousePos.x / windowWidth;
				float y_ScreenSpace = (windowHeight - io.MousePos.y) / windowHeight;
				ImGui::Text("NDC, gl_Position Origo(Center) Range(-1,-1,1,1): (%.1f,%.1f)", x_ScreenSpace * 2.f - 1.f, y_ScreenSpace * 2.f - 1.f);
			}
			else
				ImGui::Text("Mouse Position: <invalid>");
		}

		ImGui::Text("Particles");
		static std::string selectedShaderName = particlesShader->name;
		std::vector<std::string> particleShaders = { "Particle", "SoftParticle"};
		if (ImGui::BeginCombo("Shader", selectedShaderName.c_str()))
		{
			for (size_t i = 0; i < particleShaders.size(); i++)
			{
				const bool is_selected = (selectedShaderName == particleShaders[i]);
				if (ImGui::Selectable(particleShaders[i].c_str(), is_selected))
				{
					selectedShaderName = particleShaders[i];
					for (auto& shader : *GraphicsStorage::assetRegistry.GetPool<Shader>())
					{
						if (shader.name.compare(selectedShaderName) == 0)
						{
							particlesShader = &shader;
						}
					}
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		if (selectedShaderName == "SoftParticle")
		{
			ImGui::DragFloat("Contrast Power", &contrastPower, 0.1f);
			ImGui::DragFloat("Soft Scale", &softScale, 0.1f);
		}

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Custom", NULL, corner == -1)) corner = -1;
			if (ImGui::MenuItem("Top-left", NULL, corner == 0)) corner = 0;
			if (ImGui::MenuItem("Top-right", NULL, corner == 1)) corner = 1;
			if (ImGui::MenuItem("Bottom-left", NULL, corner == 2)) corner = 2;
			if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
			if (showOverlayStats && ImGui::MenuItem("Close")) showOverlayStats = false;
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}

void Editor::MainMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open", "CTRL+O")) {}
			if (ImGui::MenuItem("Save", "CTRL+S")) {}
			if (ImGui::MenuItem("Save As", "CTRL+SHIFT+S")) {}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Tools"))
		{
			ImGui::MenuItem("Object Editor", NULL, &showObjectEditor);
			ImGui::MenuItem("Scene Graph", NULL, &showSceneGraphInspector);
			ImGui::MenuItem("Database", NULL, &showDatabaseInspector);
			ImGui::MenuItem("Render List", NULL, &Render::Instance()->showRenderList);
			ImGui::MenuItem("Misc", NULL, &showMiscTools);
			ImGui::MenuItem("Stats", NULL, &showStats);
			ImGui::MenuItem("Overlay Stats", NULL, &showOverlayStats);
			ImGui::MenuItem("ImGui Demo", NULL, &showImGuiDemo);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}
template<typename T>
bool Editor::GenerateElementComboBox(Material* mat, ImGuiTextFilter* filter, MaterialElements index, const char* label, T* elementList)
{
	ImGui::PushID(label);
	bool dirty = false;
	RenderElement* selectedElement = mat->elements[(int)index];
	const char* elementLabel = (selectedElement != nullptr) ? selectedElement->name.c_str() : "";
	

	std::string buttonLabel = std::format("{}##Button", elementLabel);
	std::string popUpLabel = "##popup";
	ImGui::PushItemWidth(ImGui::GetFontSize() * -14);
	float calculatedWidth = ImGui::CalcItemWidth();
	if (ImGui::Button(buttonLabel.c_str(), ImVec2(calculatedWidth, 0.f)))
	{
		ImGui::OpenPopup(popUpLabel.c_str());
	}

	ImVec2 button_min = ImGui::GetItemRectMin();
	ImVec2 button_max = ImGui::GetItemRectMax();

	ImGui::SameLine();
	if (ImGui::Button(" X ##Clear"))
	{
		mat->AssignElement(nullptr, index);
		dirty = true;
	}
	ImGui::SameLine(); ImGui::Text(label);

	ImVec2 popupPosition;
	popupPosition.x = button_min.x;
	popupPosition.y = button_min.y;

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_HorizontalScrollbar |
		ImGuiWindowFlags_NoSavedSettings;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::SetNextWindowPos(popupPosition, ImGuiCond_Appearing);
	if (ImGui::BeginPopup(popUpLabel.c_str(), flags))
	{
		ImGui::PushItemWidth(calculatedWidth);
		float offset = 0.f;
		filter->Draw("##searchElements");
		bool isFilterInFocus = ImGui::IsItemActive();

		if (ImGui::BeginListBox("##Combo"))
		{
			int i = 0;
			for (auto& element : *elementList)
			{
				if (filter->PassFilter(element.name.c_str())) {
					std::string selElementName = std::format("{}##{}", element.name, i);
					bool is_selected = (selectedElement != nullptr) ? selectedElement == &element : false;
					if (ImGui::Selectable(selElementName.c_str(), is_selected))
					{
						mat->AssignElement(&element, index);
						filter->Clear();
						ImGui::CloseCurrentPopup();
						dirty = true;
					}
				}
			}
			ImGui::EndListBox();
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);
	ImGui::PopID();
	return dirty;
}

void Editor::GenerateMaterialElementsUI(Material * mat)
{
	ImGui::PushItemWidth(ImGui::GetFontSize() * -14);
	
	static ImGuiTextFilter renderPassFilter;
	static ImGuiTextFilter shaderFilter;
	static ImGuiTextFilter renderProfileFilter;
	static ImGuiTextFilter textureProfileFilter;
	static ImGuiTextFilter materialProfileFilter;
	static ImGuiTextFilter objectProfileFilter;
	static ImGuiTextFilter vaoFilter;
	ImGui::Checkbox("Unbound", &mat->unbound);
	GenerateElementComboBox(mat, &renderPassFilter, MaterialElements::ERenderPass, "RenderPass", GraphicsStorage::assetRegistry.GetPool<RenderPass>());
	GenerateElementComboBox(mat, &shaderFilter, MaterialElements::EShader, "Shader", GraphicsStorage::assetRegistry.GetPool<Shader>());
	GenerateElementComboBox(mat, &renderProfileFilter, MaterialElements::ERenderProfile, "RenderProfile", GraphicsStorage::assetRegistry.GetPool<RenderProfile>());
	GenerateElementComboBox(mat, &textureProfileFilter, MaterialElements::ETextureProfile, "TextureProfile", GraphicsStorage::assetRegistry.GetPool<TextureProfile>());
	GenerateElementComboBox(mat, &materialProfileFilter, MaterialElements::EMaterialProfile, "MaterialProfile", GraphicsStorage::assetRegistry.GetPool<MaterialProfile>());
	GenerateElementComboBox(mat, &vaoFilter, MaterialElements::EVao, "Vao", GraphicsStorage::assetRegistry.GetPool<VertexArray>());
	GenerateElementComboBox(mat, &objectProfileFilter, MaterialElements::EObjectProfile, "ObjectProfile", GraphicsStorage::assetRegistry.GetPool<ObjectProfile>());

	ImGui::Separator();
	ShaderOutputsToFrameBufferRenderTargetsUI(mat);

	ImGui::PopItemWidth();
}

void Editor::ShaderOutputsToFrameBufferRenderTargetsUI(Material* mat)
{
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_HorizontalScrollbar |
		ImGuiWindowFlags_NoSavedSettings;

	float calculatedWidth = ImGui::CalcItemWidth();

	if (mat->shader != nullptr && mat->rps != nullptr && mat->rps->fbo != nullptr)
	{
		if (mat->shader->outputs.size() > 0) ImGui::Text("Shader Outputs To FrameBuffer Attachments(Render Targets)");
		ImGui::Separator();

		ImVec2 uv_min(0.0f, 1.0f);                 // Top-left
		ImVec2 uv_max(1.0f, 0.0f);                 // Lower-right
		ImVec4 tint_col(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
		ImVec4 border_col(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
		ImVec2 size(10, 10);

		int j = 0;
		for (auto& output : mat->shader->outputs)
		{
			ImGui::PushID(j);
			ImGui::Text(std::format("Index: {} Type: {} Name: {}", output.index, ShaderDataType::Str(output.type), output.name).c_str());
			ImGui::Text("Will Output To This Attachment:");
			unsigned int attachment = mat->shader->renderTargets[j];
			Texture* currentTextureAtt = nullptr;
			for (auto& texAttachment : mat->rps->fbo->textures)
			{
				if (texAttachment->attachment == attachment)
				{
					currentTextureAtt = texAttachment;
					break;
				}
			}
			std::string currentTextureName = "";
			for (auto& vtex : renderTargets)
			{
				if (vtex.second == currentTextureAtt)
				{
					currentTextureName = vtex.first;
					break;
				}
			}
			std::string selectedAttachment = "UNKNOWN_ATTACHMENT";
			if (attachments.find(attachment) != attachments.end())
			{
				selectedAttachment = attachments[attachment];
			}

			std::string attachmentLabel = std::format("{} {}", selectedAttachment, currentTextureName);
			std::string popupLabel("setRTPopup");
			std::string clearButtonLabel(" X ");

			if (currentTextureAtt != nullptr && currentTextureAtt->target != GL_TEXTURE_CUBE_MAP)
			{
				ImGui::Image((void*)currentTextureAtt->handle, ImVec2(16, 16), uv_min, uv_max, tint_col, border_col);
			}
			else
			{
				ImGui::Image((void*)-1, ImVec2(16, 16), uv_min, uv_max, tint_col, border_col);
			}
			ImGui::SameLine();
			if (ImGui::Button(attachmentLabel.c_str(), ImVec2(calculatedWidth, 0.f)))
			{
				ImGui::OpenPopup(popupLabel.c_str());
			}

			ImVec2 setRenderTargetMin = ImGui::GetItemRectMin();
			ImVec2 setRenderTargetMax = ImGui::GetItemRectMax();

			ImGui::SameLine();
			if (ImGui::Button(clearButtonLabel.c_str()))
			{
				mat->shader->SetRenderTarget(j, 0);
			}


			ImVec2 renderTargetPopupPosition;
			renderTargetPopupPosition.x = setRenderTargetMin.x;
			renderTargetPopupPosition.y = setRenderTargetMin.y;
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::SetNextWindowPos(renderTargetPopupPosition, ImGuiCond_Appearing);
			if (ImGui::BeginPopup(popupLabel.c_str(), flags))
			{

				ImGui::PushItemWidth(calculatedWidth);
				if (ImGui::BeginListBox("##setRTlist"))
				{
					for (size_t i = 0; i < mat->rps->fbo->attachments.size(); i++)
					{
						bool is_selected = (!selectedAttachment.empty()) ? !strcmp(selectedAttachment.c_str(), attachments[mat->rps->fbo->attachments[i]]) : false;
						std::string selAttachmentName = std::format("{}##Sel{}", attachments[mat->rps->fbo->attachments[i]], i);
						Texture* texture = mat->rps->fbo->textures[i];
						std::string textureName = "";
						for (auto& tex : renderTargets)
						{
							if (tex.second == texture)
							{
								textureName = tex.first;
								break;
							}
						}
						if (texture->target != GL_TEXTURE_CUBE_MAP)
						{
							ImGui::Image((void*)texture->handle, size, uv_min, uv_max, tint_col, border_col);
						}
						else
						{
							ImGui::Image((void*)-1, size, uv_min, uv_max, tint_col, border_col);
						}
						ImGui::SameLine();
						if (ImGui::Selectable(selAttachmentName.c_str(), is_selected))
						{
							mat->shader->SetRenderTarget(j, GL_COLOR_ATTACHMENT0 + i);
						}
						ImGui::SameLine();
						ImGui::Text(textureName.c_str());
					}
					ImGui::EndListBox();
				}
				ImGui::PopItemWidth();
				ImGui::EndPopup();
			}
			ImGui::PopStyleVar(2);
			ImGui::Separator();
			j++;
			ImGui::PopID();
		}
	}
}

void Editor::ScriptSelector(Script* script, const char* directory)
{
	std::string scriptID = GraphicsStorage::assetRegistry.GetAssetIDAsString(script);
	ImGui::PushID(scriptID.c_str());
	std::string buttonLabel = script != nullptr ? script->name : "";
	buttonLabel += "##Button";
	std::string popupLabel = "##popup";

	ImGui::PushItemWidth(ImGui::GetFontSize() * -14);
	float calculatedWidth = ImGui::CalcItemWidth();
	if (ImGui::Button(buttonLabel.c_str(), ImVec2(calculatedWidth, 0.f)))
	{
		ImGui::OpenPopup(popupLabel.c_str());
	}

	ImVec2 button_min = ImGui::GetItemRectMin();
	ImVec2 button_max = ImGui::GetItemRectMax();

	ImGui::SameLine();
	if (ImGui::Button("..."))
	{
		ImGui::OpenPopup("ScriptMenu");
	}
	if (ImGui::BeginPopup("ScriptMenu"))
	{
		if (ImGui::MenuItem("Unload"))
		{
			if (script != nullptr) script->Unload();
		}
		if (ImGui::MenuItem("Reload"))
		{
			if (script != nullptr) script->Reload();
		}
		ImGui::EndPopup();
	}
	ImGui::SameLine(); ImGui::Text("Script");

	ImVec2 popupPosition;
	popupPosition.x = button_min.x;
	popupPosition.y = button_min.y;

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_HorizontalScrollbar |
		ImGuiWindowFlags_NoSavedSettings;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::SetNextWindowPos(popupPosition, ImGuiCond_Appearing);
	if (ImGui::BeginPopup(popupLabel.c_str(), flags))
	{
		ImGui::PushItemWidth(calculatedWidth);
		std::string filterLabel = "##searchScript";
		float offset = 0.f;
		static ImGuiTextFilter scriptFilter;
		scriptFilter.Draw(filterLabel.c_str());
		bool isFilterInFocus = ImGui::IsItemActive();

		std::vector<std::string> scriptNames;
		GraphicsManager::GetFileNames(scriptNames, directory, ".lua");

		int filteredItemCount = 0;
		for (auto& scriptName : scriptNames)
		{
			if (scriptFilter.PassFilter(scriptName.c_str())) {
				filteredItemCount++;
			}
		}

		const char* scriptLabel = script != nullptr ? script->name.c_str() : "";
		if (ImGui::BeginListBox("##Combo"))
		{
			for (auto& scriptName : scriptNames)
			{
				if (scriptFilter.PassFilter(scriptName.c_str())) {
					bool is_selected = (scriptLabel != NULL) ? !strcmp(scriptLabel, scriptName.c_str()) : false;
					std::string selScriptName = std::format("{}##Sel", scriptName);
					if (ImGui::Selectable(selScriptName.c_str(), is_selected))
					{
						script->LoadLuaFile(std::format("{}/{}.lua", directory, scriptName).c_str());
						scriptFilter.Clear();
						ImGui::CloseCurrentPopup();
					}
				}
			}
			ImGui::EndListBox();
		}
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);
	ImGui::PopID();
}

void Editor::RenderPassInspector(RenderPass * pass)
{
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_HorizontalScrollbar |
		ImGuiWindowFlags_NoSavedSettings;

	ImGui::Text("Instances");
	ImGui::PushItemWidth(-1);
	static ImGuiTextFilter filterInstances;
	filterInstances.Draw("##searchbar2");

	ImGui::PushID("FrameBuffer");
	std::string fboName = "";
	if (pass->fbo != nullptr)
	{
		fboName = pass->fbo->name;
	}
	std::string buttonLabel = std::format("{}##fbobtn", fboName);
	std::string popupLabel = "##popup";

	ImGui::PushItemWidth(ImGui::GetFontSize() * -14);
	float calculatedWidth = ImGui::CalcItemWidth();
	if (ImGui::Button(buttonLabel.c_str(), ImVec2(calculatedWidth, 0.f)))
	{
		ImGui::OpenPopup(popupLabel.c_str());
	}

	ImVec2 button_min = ImGui::GetItemRectMin();
	ImVec2 button_max = ImGui::GetItemRectMax();

	ImGui::SameLine();
	if (ImGui::Button(" X ##Clear"))
	{
		pass->fbo = nullptr;
	}
	ImGui::SameLine(); ImGui::Text("FrameBuffer");

	ImVec2 popupPosition;
	popupPosition.x = button_min.x;
	popupPosition.y = button_min.y;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::SetNextWindowPos(popupPosition, ImGuiCond_Appearing);
	if (ImGui::BeginPopup(popupLabel.c_str(), flags))
	{
		ImGui::PushItemWidth(calculatedWidth);
		std::string filterLabel = "##searchFboProperty";
		float offset = 0.f;
		static ImGuiTextFilter fboPropertyFilter;
		fboPropertyFilter.Draw(filterLabel.c_str());
		bool isFilterInFocus = ImGui::IsItemActive();

		int filteredItemCount = 0;
		for (auto& fbo : *GraphicsStorage::assetRegistry.GetPool<FrameBuffer>())
		{
			if (fboPropertyFilter.PassFilter(fbo.name.c_str())) {
				filteredItemCount++;
			}
		}

		const char* fboLabel = fboName.c_str();
		if (ImGui::BeginListBox("##Combo"))
		{
			int i = 0;
			for (auto& fbo : *GraphicsStorage::assetRegistry.GetPool<FrameBuffer>())
			{
				if (fboPropertyFilter.PassFilter(fbo.name.c_str())) {
					const bool is_selected = (&fbo == pass->fbo);
					std::string selectableName = std::format("{}##{}", fbo.name, i);
					if (ImGui::Selectable(selectableName.c_str(), is_selected))
					{
						pass->fbo = &fbo;
						fboPropertyFilter.Clear();
						ImGui::CloseCurrentPopup();
					}
				}
				i++;
			}
			ImGui::EndListBox();
		}
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);
	ImGui::PopID();
	ScriptSelector(pass->script, "resources/passes/lua");

	if (LuaProperties(&pass->registry, "RenderPass", "resources/passes/properties/", pass))
	{
		pass->UpdateProfileFromDataRegistry(pass->registry);
	}
	ImGui::Separator();

	if (ObjectProperties(&pass->registry, "RenderPass"))
	{
		pass->UpdateProfileFromDataRegistry(pass->registry);
	}
	ImGui::Separator();

	if (DataRegistryEditor(&pass->registry))
	{
		pass->UpdateProfileFromDataRegistry(pass->registry);
	}
	ImGui::Separator();

	ShaderBlocks(pass);

	if (ImGui::Button("Revert")) {}
	ImGui::SameLine();
	if (ImGui::Button("Save"))
	{
		pass->path = std::format("resources/passes/{}.json", pass->name);
		lua_State* L = luaL_newstate();
		luaL_openlibs(L);
		LuaTools::dofile(L, "resources/serialization/pass_saver.lua");
		lua_getglobal(L, "SaveRenderPass");
		if (lua_isfunction(L, -1))
		{
			lua_pushlightuserdata(L, pass);
			int result = LuaTools::report(L, LuaTools::docall(L, 1, 1));
			lua_pop(L, 1);
		}
		lua_close(L);
	}
}

inline void Editor::ShaderBlocks(ObjectProfile* profile)
{
	if (ImGui::CollapsingHeader("Uniform Buffers"))
	{
		ImGui::PushItemWidth(-1);
		float calculatedWidth = ImGui::CalcItemWidth();
		std::string popupLabel = "adduniform_popup";
		if (ImGui::Button("Add Uniform Buffer##AddUniformBufferButton", ImVec2(-1, 0)))
		{
			ImGui::OpenPopup(popupLabel.c_str());
		}

		ImVec2 button_min = ImGui::GetItemRectMin();
		ImVec2 button_max = ImGui::GetItemRectMax();

		ImVec2 popupPosition;
		popupPosition.x = button_min.x;
		popupPosition.y = button_min.y;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::SetNextWindowPos(popupPosition, ImGuiCond_Appearing);

		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_HorizontalScrollbar |
			ImGuiWindowFlags_NoSavedSettings;

		if (ImGui::BeginPopup(popupLabel.c_str(), flags))
		{
			ImGui::PushItemWidth(calculatedWidth);
			static ImGuiTextFilter filterAvailableUniformBuffers;
			filterAvailableUniformBuffers.Draw("##filterAvailableUniformBuffers");

			if (ImGui::BeginListBox("##AvailableUniformBuffersList"))
			{
				int i = 0;
				for (auto ub : GraphicsStorage::uniformBuffers)
				{
					if (filterAvailableUniformBuffers.PassFilter(ub->name.c_str())) {
						std::string selectableName = std::format("{}##{}", ub->name, i);
						if (ImGui::Selectable(selectableName.c_str()))
						{
							profile->AddShaderBlock(ub);
							filterAvailableUniformBuffers.Clear();
						}
						i++;
					}
				}
				ImGui::EndListBox();
			}
			ImGui::PopItemWidth();
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar(2);

		ImGui::Text("Uniform Buffers:");
		ImGui::Separator();
		std::vector<ShaderBlock*> uniformBuffersToRemove;
		for (auto& ubd : profile->shaderBlockDatas)
		{
			if (ImGui::Button((std::string("Remove##") + ubd.shaderBlock->name).c_str()))
			{
				uniformBuffersToRemove.push_back(ubd.shaderBlock);
			}
			ShaderBlocksInspector(ubd.shaderBlock);
			ImGui::Separator();
		}
		for (auto ub : uniformBuffersToRemove)
		{
			profile->RemoveShaderBlock(ub);
		}
	}
}

inline void Editor::ShaderBlocksInspector(ShaderBlock * ub)
{
	std::vector<std::string> uniformBufferUpdateFrequencyOptions = { "Pass", "Object", "Material" };

	std::unordered_map<ShaderBlock*, std::string> buffersToMove;
	std::string selectedFrequency = GraphicsStorage::shaderBlockTypes[ub->name];
	std::string updateFrequencyLabel = "Update Frequency##combo" + ub->name;
	ImGui::PushItemWidth(ImGui::GetFontSize() * -10.f);
	if (ImGui::BeginCombo(updateFrequencyLabel.c_str(), selectedFrequency.c_str()))
	{
		for (size_t i = 0; i < uniformBufferUpdateFrequencyOptions.size(); i++)
		{
			const bool is_selected = (selectedFrequency == uniformBufferUpdateFrequencyOptions[i]);
			if (ImGui::Selectable(uniformBufferUpdateFrequencyOptions[i].c_str(), is_selected))
			{
				GraphicsStorage::shaderBlockTypes[ub->name] = uniformBufferUpdateFrequencyOptions[i];
			}
		}
		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();

	ImGui::PushItemWidth(ImGui::GetFontSize() * -14);
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), std::format("Name: {}\nIndex: {}\nSize: {} B", ub->name, ub->index, ub->size).c_str());
	ImGui::Text("Locations:");
	auto& uniforms = GraphicsStorage::shaderBlockUniforms[ub->name];
	std::map<int, std::string> orderedUniforms;
	static int ivalue;
	static float fvalue;
	static char cvalue[128];
	static bool bvalue;
	static glm::vec2 row21;
	static glm::vec2 row22;
	static glm::vec3 row31;
	static glm::vec3 row32;
	static glm::vec3 row33;
	static glm::vec4 row41;
	static glm::vec4 row42;
	static glm::vec4 row43;
	static glm::vec4 row44;
	static glm::vec2 row2;
	static glm::vec3 row3;
	static glm::vec4 row4;
	static glm::mat4 mat4;
	static glm::mat3 mat3;
	static glm::vec4 mat2;
	for (auto& uniformInfo : uniforms)
	{
		orderedUniforms[uniformInfo.second.offset] = uniformInfo.first;
	}
	for (auto& uniform : orderedUniforms)
	{
		int glType = uniforms[uniform.second].type;
		std::string type = glShaderDataTypes[glType];
		std::string valueName = (type + " " + uniform.second);
		switch (glType)
		{
		case GL_INT:
			ImGui::DragInt(valueName.c_str(), (int*)&ub->data.data[ub->offsets[uniform.second]]);
			break;
		case GL_UNSIGNED_INT:
			ImGui::DragInt(valueName.c_str(), (int*)&ub->data.data[ub->offsets[uniform.second]]);
			break;
		case GL_FLOAT:
			ImGui::DragFloat(valueName.c_str(), (float*)&ub->data.data[ub->offsets[uniform.second]]);
			break;
		case GL_BOOL:
			ImGui::Checkbox(valueName.c_str(), (bool*)&ub->data.data[ub->offsets[uniform.second]]);
			break;
		case GL_FLOAT_MAT2:
			ImGui::DragFloat2(valueName.c_str(), (float*)&ub->data.data[ub->offsets[uniform.second]]);
			ImGui::DragFloat2(("##" + valueName + "2").c_str(), (float*)&ub->data.data[ub->offsets[uniform.second] + 2 * 4]);
			break;
		case GL_FLOAT_MAT3:
			ImGui::DragFloat3(valueName.c_str(), (float*)&ub->data.data[ub->offsets[uniform.second]]);
			ImGui::DragFloat3(("##" + valueName + "2").c_str(), (float*)&ub->data.data[ub->offsets[uniform.second] + 3 * 4]);
			ImGui::DragFloat3(("##" + valueName + "3").c_str(), (float*)&ub->data.data[ub->offsets[uniform.second] + 2 * 3 * 4]);
			break;
		case GL_FLOAT_MAT4:
			ImGui::DragFloat4(valueName.c_str(), (float*)&ub->data.data[ub->offsets[uniform.second]]);
			ImGui::DragFloat4(("##" + valueName + "2").c_str(), (float*)&ub->data.data[ub->offsets[uniform.second] + 4 * 4]);
			ImGui::DragFloat4(("##" + valueName + "3").c_str(), (float*)&ub->data.data[ub->offsets[uniform.second] + 2 * 4 * 4]);
			ImGui::DragFloat4(("##" + valueName + "4").c_str(), (float*)&ub->data.data[ub->offsets[uniform.second] + 3 * 4 * 4]);
			break;
		case GL_FLOAT_VEC2:
			ImGui::DragFloat2(valueName.c_str(), (float*)&ub->data.data[ub->offsets[uniform.second]]);
			break;
		case GL_FLOAT_VEC3:
			ImGui::DragFloat3(valueName.c_str(), (float*)&ub->data.data[ub->offsets[uniform.second]]);
			break;
		case GL_FLOAT_VEC4:
			ImGui::DragFloat4(valueName.c_str(), (float*)&ub->data.data[ub->offsets[uniform.second]]);
			break;
		default:
			break;
		}
	}
	ImGui::PopItemWidth();
}

void Editor::InitializeGLC()
{
	targets[GL_TEXTURE_1D] = "GL_TEXTURE_1D";
	targets[GL_TEXTURE_1D_ARRAY] = "GL_TEXTURE_1D_ARRAY";
	targets[GL_TEXTURE_2D] = "GL_TEXTURE_2D";
	targets[GL_TEXTURE_2D_ARRAY] = "GL_TEXTURE_2D_ARRAY";
	targets[GL_TEXTURE_2D_MULTISAMPLE] = "GL_TEXTURE_2D_MULTISAMPLE";
	targets[GL_TEXTURE_2D_MULTISAMPLE_ARRAY] = "GL_TEXTURE_2D_MULTISAMPLE_ARRAY";
	targets[GL_TEXTURE_3D] = "GL_TEXTURE_3D";
	targets[GL_TEXTURE_CUBE_MAP] = "GL_TEXTURE_CUBE_MAP";
	targets[GL_TEXTURE_RECTANGLE] = "GL_TEXTURE_CUBE_MAP_ARRAY";
	targets[GL_TEXTURE_CUBE_MAP_ARRAY] = "GL_TEXTURE_RECTANGLE";
	targets[GL_RENDERBUFFER] = "GL_RENDERBUFFER";

	//base
	internalFormats[GL_DEPTH_COMPONENT] = "GL_DEPTH_COMPONENT";
	internalFormats[GL_DEPTH_STENCIL] = "GL_DEPTH_STENCIL";
	internalFormats[GL_RED] = "GL_RED";
	internalFormats[GL_RG] = "GL_RG";
	internalFormats[GL_RGB] = "GL_RGB";
	internalFormats[GL_RGBA] = "GL_RGBA";
	//depth stencil
	internalFormats[GL_DEPTH_COMPONENT16] = "GL_DEPTH_COMPONENT16";
	internalFormats[GL_DEPTH_COMPONENT24] = "GL_DEPTH_COMPONENT24";
	internalFormats[GL_DEPTH_COMPONENT32] = "GL_DEPTH_COMPONENT32";
	internalFormats[GL_DEPTH24_STENCIL8] = "GL_DEPTH24_STENCIL8";
	//sized
	internalFormats[GL_R8] = "GL_R8";
	internalFormats[GL_R8_SNORM] = "GL_R8_SNORM";
	internalFormats[GL_R16] = "GL_R16";
	internalFormats[GL_R16_SNORM] = "GL_R16_SNORM";
	internalFormats[GL_RG8] = "GL_RG8";
	internalFormats[GL_RG8_SNORM] = "GL_RG8_SNORM";
	internalFormats[GL_RG16] = "GL_RG16";
	internalFormats[GL_RG16_SNORM] = "GL_RG16_SNORM";
	internalFormats[GL_R3_G3_B2] = "GL_R3_G3_B2";
	internalFormats[GL_RGB4] = "GL_RGB4";
	internalFormats[GL_RGB5] = "GL_RGB5";
	internalFormats[GL_RGB8] = "GL_RGB8";
	internalFormats[GL_RGB8_SNORM] = "GL_RGB8_SNORM";
	internalFormats[GL_RGB10] = "GL_RGB10";
	internalFormats[GL_RGB12] = "GL_RGB12";
	internalFormats[GL_RGB16_SNORM] = "GL_RGB16_SNORM";
	internalFormats[GL_RGBA2] = "GL_RGBA2";
	internalFormats[GL_RGBA4] = "GL_RGBA4";
	internalFormats[GL_RGB5_A1] = "GL_RGB5_A1";
	internalFormats[GL_RGBA8] = "GL_RGBA8";
	internalFormats[GL_RGBA8_SNORM] = "GL_RGBA8_SNORM";
	internalFormats[GL_RGB10_A2] = "GL_RGB10_A2";
	internalFormats[GL_RGB10_A2UI] = "GL_RGB10_A2UI";
	internalFormats[GL_RGBA12] = "GL_RGBA12";
	internalFormats[GL_RGBA16] = "GL_RGBA16";
	internalFormats[GL_SRGB8] = "GL_SRGB8";
	internalFormats[GL_SRGB8_ALPHA8] = "GL_SRGB8_ALPHA8";
	internalFormats[GL_R16F] = "GL_R16F";
	internalFormats[GL_RG16F] = "GL_RG16F";
	internalFormats[GL_RGB16F] = "GL_RGB16F";
	internalFormats[GL_RGBA16F] = "GL_RGBA16F";
	internalFormats[GL_R32F] = "GL_R32F";
	internalFormats[GL_RG32F] = "GL_RG32F";
	internalFormats[GL_RGB32F] = "GL_RGB32F";
	internalFormats[GL_RGBA32F] = "GL_RGBA32F";
	internalFormats[GL_R11F_G11F_B10F] = "GL_R11F_G11F_B10F";
	internalFormats[GL_RGB9_E5] = "GL_RGB9_E5";
	internalFormats[GL_R8I] = "GL_R8I";
	internalFormats[GL_R8UI] = "GL_R8UI";
	internalFormats[GL_R16I] = "GL_R16I";
	internalFormats[GL_R16UI] = "GL_R16UI";
	internalFormats[GL_R32I] = "GL_R32I";
	internalFormats[GL_R32UI] = "GL_R32UI";
	internalFormats[GL_RG8I] = "GL_RG8I";
	internalFormats[GL_RG8UI] = "GL_RG8UI";
	internalFormats[GL_RG16I] = "GL_RG16I";
	internalFormats[GL_RG16UI] = "GL_RG16UI";
	internalFormats[GL_RG32I] = "GL_RG32I";
	internalFormats[GL_RG32UI] = "GL_RG32UI";
	internalFormats[GL_RGB8I] = "GL_RGB8I";
	internalFormats[GL_RGB8UI] = "GL_RGB8UI";
	internalFormats[GL_RGB16I] = "GL_RGB16I";
	internalFormats[GL_RGB16UI] = "GL_RGB16UI";
	internalFormats[GL_RGB32I] = "GL_RGB32I";
	internalFormats[GL_RGB32UI] = "GL_RGB32UI";
	internalFormats[GL_RGBA8I] = "GL_RGBA8I";
	internalFormats[GL_RGBA8UI] = "GL_RGBA8UI";
	internalFormats[GL_RGBA16I] = "GL_RGBA16I";
	internalFormats[GL_RGBA16UI] = "GL_RGBA16UI";
	internalFormats[GL_RGBA32I] = "GL_RGBA32I";
	internalFormats[GL_RGBA32UI] = "GL_RGBA32UI";
		
	//compressed
	internalFormats[GL_COMPRESSED_RED] = "GL_COMPRESSED_RED";
	internalFormats[GL_COMPRESSED_RG] = "GL_COMPRESSED_RG";
	internalFormats[GL_COMPRESSED_RGB] = "GL_COMPRESSED_RGB";
	internalFormats[GL_COMPRESSED_RGBA] = "GL_COMPRESSED_RGBA";
	internalFormats[GL_COMPRESSED_SRGB] = "GL_COMPRESSED_SRGB";
	internalFormats[GL_COMPRESSED_SRGB_ALPHA] = "GL_COMPRESSED_SRGB_ALPHA";
	internalFormats[GL_COMPRESSED_RED_RGTC1] = "GL_COMPRESSED_RED_RGTC1";
	internalFormats[GL_COMPRESSED_SIGNED_RED_RGTC1] = "GL_COMPRESSED_SIGNED_RED_RGTC1";
	internalFormats[GL_COMPRESSED_RG_RGTC2] = "GL_COMPRESSED_RG_RGTC2";
	internalFormats[GL_COMPRESSED_SIGNED_RG_RGTC2] = "GL_COMPRESSED_SIGNED_RG_RGTC2";
	internalFormats[GL_COMPRESSED_RGBA_BPTC_UNORM] = "GL_COMPRESSED_RGBA_BPTC_UNORM";
	internalFormats[GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM] = "GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM";
	internalFormats[GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT] = "GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT";
	internalFormats[GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT] = "GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT";
	//compressed directx
	internalFormats[GL_COMPRESSED_RGB_S3TC_DXT1_EXT] = "GL_COMPRESSED_RGB_S3TC_DXT1_EXT";
	internalFormats[GL_COMPRESSED_RGBA_S3TC_DXT1_EXT] = "GL_COMPRESSED_RGBA_S3TC_DXT1_EXT";
	internalFormats[GL_COMPRESSED_RGBA_S3TC_DXT3_EXT] = "GL_COMPRESSED_RGBA_S3TC_DXT3_EXT";
	internalFormats[GL_COMPRESSED_RGBA_S3TC_DXT5_EXT] = "GL_COMPRESSED_RGBA_S3TC_DXT5_EXT";

	formats[GL_RED] = "GL_RED";
	formats[GL_RG] = "GL_RG";
	formats[GL_RGB] = "GL_RGB";
	formats[GL_BGR] = "GL_BGR";
	formats[GL_RGBA] = "GL_RGBA";
	formats[GL_BGRA] = "GL_BGRA";
	formats[GL_RED_INTEGER] = "GL_RED_INTEGER";
	formats[GL_RG_INTEGER] = "GL_RG_INTEGER";
	formats[GL_RGB_INTEGER] = "GL_RGB_INTEGER";
	formats[GL_BGR_INTEGER] = "GL_BGR_INTEGER";
	formats[GL_RGBA_INTEGER] = "GL_RGBA_INTEGER";
	formats[GL_BGRA_INTEGER] = "GL_BGRA_INTEGER";
	formats[GL_STENCIL_INDEX] = "GL_STENCIL_INDEX";
	formats[GL_DEPTH_COMPONENT] = "GL_DEPTH_COMPONENT";
	formats[GL_DEPTH_STENCIL] = "GL_DEPTH_STENCIL";

	types[GL_UNSIGNED_BYTE] = "GL_UNSIGNED_BYTE";
	types[GL_BYTE] = "GL_BYTE";
	types[GL_UNSIGNED_SHORT] = "GL_UNSIGNED_SHORT";
	types[GL_SHORT] = "GL_SHORT";
	types[GL_UNSIGNED_INT] = "GL_UNSIGNED_INT";
	types[GL_INT] = "GL_INT";
	types[GL_HALF_FLOAT] = "GL_HALF_FLOAT";
	types[GL_FLOAT] = "GL_FLOAT";
	types[GL_UNSIGNED_BYTE_3_3_2] = "GL_UNSIGNED_BYTE_3_3_2";
	types[GL_UNSIGNED_BYTE_2_3_3_REV] = "GL_UNSIGNED_BYTE_2_3_3_REV";
	types[GL_UNSIGNED_SHORT_5_6_5] = "GL_UNSIGNED_SHORT_5_6_5";
	types[GL_UNSIGNED_SHORT_5_6_5_REV] = "GL_UNSIGNED_SHORT_5_6_5_REV";
	types[GL_UNSIGNED_SHORT_4_4_4_4] = "GL_UNSIGNED_SHORT_4_4_4_4";
	types[GL_UNSIGNED_SHORT_4_4_4_4_REV] = "GL_UNSIGNED_SHORT_4_4_4_4_REV";
	types[GL_UNSIGNED_SHORT_5_5_5_1] = "GL_UNSIGNED_SHORT_5_5_5_1";
	types[GL_UNSIGNED_SHORT_1_5_5_5_REV] = "GL_UNSIGNED_SHORT_1_5_5_5_REV";
	types[GL_UNSIGNED_INT_8_8_8_8] = "GL_UNSIGNED_INT_8_8_8_8";
	types[GL_UNSIGNED_INT_8_8_8_8_REV] = "GL_UNSIGNED_INT_8_8_8_8_REV";
	types[GL_UNSIGNED_INT_10_10_10_2] = "GL_UNSIGNED_INT_10_10_10_2";
	types[GL_UNSIGNED_INT_2_10_10_10_REV] = "GL_UNSIGNED_INT_2_10_10_10_REV";

	attachments[GL_COLOR_ATTACHMENT0] = "GL_COLOR_ATTACHMENT0";
	attachments[GL_COLOR_ATTACHMENT1] = "GL_COLOR_ATTACHMENT1";
	attachments[GL_COLOR_ATTACHMENT2] = "GL_COLOR_ATTACHMENT2";
	attachments[GL_COLOR_ATTACHMENT3] = "GL_COLOR_ATTACHMENT3";
	attachments[GL_COLOR_ATTACHMENT4] = "GL_COLOR_ATTACHMENT4";
	attachments[GL_COLOR_ATTACHMENT5] = "GL_COLOR_ATTACHMENT5";
	attachments[GL_COLOR_ATTACHMENT6] = "GL_COLOR_ATTACHMENT6";
	attachments[GL_COLOR_ATTACHMENT7] = "GL_COLOR_ATTACHMENT7";
	attachments[GL_COLOR_ATTACHMENT8] = "GL_COLOR_ATTACHMENT8";
	attachments[GL_COLOR_ATTACHMENT9] = "GL_COLOR_ATTACHMENT9";
	attachments[GL_COLOR_ATTACHMENT10] = "GL_COLOR_ATTACHMENT10";
	attachments[GL_COLOR_ATTACHMENT11] = "GL_COLOR_ATTACHMENT11";
	attachments[GL_COLOR_ATTACHMENT12] = "GL_COLOR_ATTACHMENT12";
	attachments[GL_COLOR_ATTACHMENT13] = "GL_COLOR_ATTACHMENT13";
	attachments[GL_COLOR_ATTACHMENT14] = "GL_COLOR_ATTACHMENT14";
	attachments[GL_COLOR_ATTACHMENT15] = "GL_COLOR_ATTACHMENT15";
	attachments[GL_DEPTH_ATTACHMENT] = "GL_DEPTH_ATTACHMENT";
	attachments[GL_STENCIL_ATTACHMENT] = "GL_STENCIL_ATTACHMENT";
	attachments[GL_DEPTH_STENCIL_ATTACHMENT] = "GL_DEPTH_STENCIL_ATTACHMENT";

	parameterNames[GL_DEPTH_STENCIL_TEXTURE_MODE] = "GL_DEPTH_STENCIL_TEXTURE_MODE";
	parameterNames[GL_IMAGE_FORMAT_COMPATIBILITY_TYPE] = "GL_IMAGE_FORMAT_COMPATIBILITY_TYPE";
	parameterNames[GL_TEXTURE_BASE_LEVEL] = "GL_TEXTURE_BASE_LEVEL";
	parameterNames[GL_TEXTURE_BORDER_COLOR] = "GL_TEXTURE_BORDER_COLOR";
	parameterNames[GL_TEXTURE_COMPARE_MODE] = "GL_TEXTURE_COMPARE_MODE";
	parameterNames[GL_TEXTURE_COMPARE_FUNC] = "GL_TEXTURE_COMPARE_FUNC";
	parameterNames[GL_TEXTURE_IMMUTABLE_FORMAT] = "GL_TEXTURE_IMMUTABLE_FORMAT";
	parameterNames[GL_TEXTURE_IMMUTABLE_LEVELS] = "GL_TEXTURE_IMMUTABLE_LEVELS";
	parameterNames[GL_TEXTURE_LOD_BIAS] = "GL_TEXTURE_LOD_BIAS";
	parameterNames[GL_TEXTURE_MAG_FILTER] = "GL_TEXTURE_MAG_FILTER";
	parameterNames[GL_TEXTURE_MAX_LEVEL] = "GL_TEXTURE_MAX_LEVEL";
	parameterNames[GL_TEXTURE_MAX_LOD] = "GL_TEXTURE_MAX_LOD";
	parameterNames[GL_TEXTURE_MIN_FILTER] = "GL_TEXTURE_MIN_FILTER";
	parameterNames[GL_TEXTURE_MIN_LOD] = "GL_TEXTURE_MIN_LOD";
	parameterNames[GL_TEXTURE_SWIZZLE_R] = "GL_TEXTURE_SWIZZLE_R";
	parameterNames[GL_TEXTURE_SWIZZLE_G] = "GL_TEXTURE_SWIZZLE_G";
	parameterNames[GL_TEXTURE_SWIZZLE_B] = "GL_TEXTURE_SWIZZLE_B";
	parameterNames[GL_TEXTURE_SWIZZLE_A] = "GL_TEXTURE_SWIZZLE_A";
	parameterNames[GL_TEXTURE_SWIZZLE_RGBA] = "GL_TEXTURE_SWIZZLE_RGBA";
	parameterNames[GL_TEXTURE_TARGET] = "GL_TEXTURE_TARGET";
	parameterNames[GL_TEXTURE_VIEW_MIN_LAYER] = "GL_TEXTURE_VIEW_MIN_LAYER";
	parameterNames[GL_TEXTURE_VIEW_MIN_LEVEL] = "GL_TEXTURE_VIEW_MIN_LEVEL";
	parameterNames[GL_TEXTURE_VIEW_NUM_LAYERS] = "GL_TEXTURE_VIEW_NUM_LAYERS";
	parameterNames[GL_TEXTURE_VIEW_NUM_LEVELS] = "GL_TEXTURE_VIEW_NUM_LEVELS";
	parameterNames[GL_TEXTURE_WRAP_S] = "GL_TEXTURE_WRAP_S";
	parameterNames[GL_TEXTURE_WRAP_T] = "GL_TEXTURE_WRAP_T";
	parameterNames[GL_TEXTURE_WRAP_R] = "GL_TEXTURE_WRAP_R";

	depthStencilModes[GL_DEPTH_COMPONENT] = "GL_DEPTH_COMPONENT";
	depthStencilModes[GL_STENCIL_INDEX] = "GL_STENCIL_INDEX";

	imageFormatCompatibilityTypes[GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE] = "GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE";
	imageFormatCompatibilityTypes[GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS] = "GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS";
	imageFormatCompatibilityTypes[GL_NONE] = "GL_NONE";

	compareModes[GL_COMPARE_REF_TO_TEXTURE] = "GL_COMPARE_REF_TO_TEXTURE";
	compareModes[GL_NONE] = "GL_NONE";

	compareFuncs[GL_LEQUAL] = "GL_LEQUAL";
	compareFuncs[GL_GEQUAL] = "GL_GEQUAL";
	compareFuncs[GL_LESS] = "GL_LESS";
	compareFuncs[GL_GREATER] = "GL_GREATER";
	compareFuncs[GL_EQUAL] = "GL_EQUAL";
	compareFuncs[GL_NOTEQUAL] = "GL_NOTEQUAL";
	compareFuncs[GL_ALWAYS] = "GL_ALWAYS";
	compareFuncs[GL_NEVER] = "GL_NEVER";
	compareFuncs[GL_NONE] = "GL_NONE";

	immutableFormat[GL_TRUE] = "GL_TRUE";
	immutableFormat[GL_FALSE] = "GL_FALSE";

	minFilters[GL_NEAREST] = "GL_NEAREST";
	minFilters[GL_LINEAR] = "GL_LINEAR";
	minFilters[GL_NEAREST_MIPMAP_NEAREST] = "GL_NEAREST_MIPMAP_NEAREST";
	minFilters[GL_LINEAR_MIPMAP_NEAREST] = "GL_LINEAR_MIPMAP_NEAREST";
	minFilters[GL_NEAREST_MIPMAP_LINEAR] = "GL_NEAREST_MIPMAP_LINEAR";
	minFilters[GL_LINEAR_MIPMAP_LINEAR] = "GL_LINEAR_MIPMAP_LINEAR";

	magFilters[GL_NEAREST] = "GL_NEAREST";
	magFilters[GL_LINEAR] = "GL_LINEAR";

	swizzles[GL_RED] = "GL_RED";
	swizzles[GL_GREEN] = "GL_GREEN";
	swizzles[GL_BLUE] = "GL_BLUE";
	swizzles[GL_ALPHA] = "GL_ALPHA";
	swizzles[GL_ZERO] = "GL_ZERO";
	swizzles[GL_ONE] = "GL_ONE";

	wrapFunctions[GL_CLAMP_TO_EDGE] = "GL_CLAMP_TO_EDGE";
	wrapFunctions[GL_CLAMP_TO_BORDER] = "GL_CLAMP_TO_BORDER";
	wrapFunctions[GL_MIRRORED_REPEAT] = "GL_MIRRORED_REPEAT";
	wrapFunctions[GL_REPEAT] = "GL_REPEAT";
	wrapFunctions[GL_MIRROR_CLAMP_TO_EDGE] = "GL_MIRROR_CLAMP_TO_EDGE";

	indicesTypes[GL_UNSIGNED_BYTE] = "GL_UNSIGNED_BYTE";
	indicesTypes[GL_UNSIGNED_SHORT] = "GL_UNSIGNED_SHORT";
	indicesTypes[GL_UNSIGNED_INT] = "GL_UNSIGNED_INT";
	
	glShaderDataTypes[GL_FLOAT] = "float";
	glShaderDataTypes[GL_FLOAT_VEC2] = "vec2";
	glShaderDataTypes[GL_FLOAT_VEC3] = "vec3";
	glShaderDataTypes[GL_FLOAT_VEC4] = "vec4";
	glShaderDataTypes[GL_DOUBLE] = "double";
	glShaderDataTypes[GL_DOUBLE_VEC2] = "dvec2";
	glShaderDataTypes[GL_DOUBLE_VEC3] = "dvec3";
	glShaderDataTypes[GL_DOUBLE_VEC4] = "dvec4";
	glShaderDataTypes[GL_INT] = "int";
	glShaderDataTypes[GL_INT_VEC2] = "ivec2";
	glShaderDataTypes[GL_INT_VEC3] = "ivec3";
	glShaderDataTypes[GL_INT_VEC4] = "ivec4";
	glShaderDataTypes[GL_UNSIGNED_INT] = "uint";
	glShaderDataTypes[GL_UNSIGNED_INT_VEC2] = "uvec2";
	glShaderDataTypes[GL_UNSIGNED_INT_VEC3] = "uvec3";
	glShaderDataTypes[GL_UNSIGNED_INT_VEC4] = "uvec4";
	glShaderDataTypes[GL_BOOL] = "bool";
	glShaderDataTypes[GL_BOOL_VEC2] = "bvec2";
	glShaderDataTypes[GL_BOOL_VEC3] = "bvec3";
	glShaderDataTypes[GL_BOOL_VEC4] = "bvec4";
	glShaderDataTypes[GL_FLOAT_MAT2] = "mat2";
	glShaderDataTypes[GL_FLOAT_MAT3] = "mat3";
	glShaderDataTypes[GL_FLOAT_MAT4] = "mat4";
	glShaderDataTypes[GL_FLOAT_MAT2x3] = "mat2x3";
	glShaderDataTypes[GL_FLOAT_MAT2x4] = "mat2x4";
	glShaderDataTypes[GL_FLOAT_MAT3x2] = "mat3x2";
	glShaderDataTypes[GL_FLOAT_MAT3x4] = "mat3x4";
	glShaderDataTypes[GL_FLOAT_MAT4x2] = "mat4x2";
	glShaderDataTypes[GL_FLOAT_MAT4x3] = "mat4x3";
	glShaderDataTypes[GL_DOUBLE_MAT2] = "dmat2";
	glShaderDataTypes[GL_DOUBLE_MAT3] = "dmat3";
	glShaderDataTypes[GL_DOUBLE_MAT4] = "dmat4";
	glShaderDataTypes[GL_DOUBLE_MAT2x3] = "dmat2x3";
	glShaderDataTypes[GL_DOUBLE_MAT2x4] = "dmat2x4";
	glShaderDataTypes[GL_DOUBLE_MAT3x2] = "dmat3x2";
	glShaderDataTypes[GL_DOUBLE_MAT3x4] = "dmat3x4";
	glShaderDataTypes[GL_DOUBLE_MAT4x2] = "dmat4x2";
	glShaderDataTypes[GL_DOUBLE_MAT4x3] = "dmat4x3";

	currentTarget = GL_TEXTURE_2D;
	currentLevel = 0;
	currentInternalFormat = GL_RGB;
	currentWidthHeight[0] = 1024;
	currentWidthHeight[1] = 1024;
	currentFormat = GL_RGB;
	currentType = GL_FLOAT;
	currentAttachment = GL_COLOR_ATTACHMENT0;
}

/*
* TODO add drawcalls slider for geometry or total?
* total is good for debugging step by step what is happening
*/


void Editor::GenerateGUI()
{
	ImGuiIO& io = ImGui::GetIO();
	keyToggles[ImGuiKey_LeftAlt].Update(ImGui::IsKeyDown(ImGuiKey_LeftAlt));
	toggleUI = keyToggles[ImGuiKey_LeftAlt].IsToggled();
	//if (ImGui::IsKeyPressed(GLFW_KEY_P))
	//{
	//	ParentSceneGraphSelection();
	//}
	//if (ImGui::IsKeyPressed(GLFW_KEY_Z))
	//{
	//	std::string templatePath = "resources/objects/cube.json";
	//	for (size_t i = 0; i < 30; i++)
	//	{
	//		CreateObjectFromTemplate(templatePath.data());
	//	}
	//}
	if (toggleUI || !toggleUIonAlt)
	{
		MainMenuBar();
		if (showOverlayStats) OverlayStats();
		if (showObjectEditor) ObjectEditor(lastPickedObject);
		if (showSceneGraphInspector) SceneGraphInspector();
		if (showDatabaseInspector) DatabaseInspector();
		if (showStyleEditor) ImGui::ShowStyleEditor();
		if (showImGuiDemo) ImGui::ShowDemoWindow(&showImGuiDemo);
		if (showMiscTools) MiscTools();
		if (showStats) Stats();
		if (showImportWindow) ImportUI();
	}
	LoadAndSaveMeshes();
}

void Editor::ParentSceneGraphSelection()
{
	if (selectionList.size() > 1)
	{
		Object* parent = selectionList.back();
		for (size_t i = 0; i < selectionList.size() - 1; i++)
		{
			SceneGraph::Instance()->ParentInPlace(selectionList[i]->node, parent->node);
		}
		selectionList.clear();
		selectionList.push_back(parent);
	}
}

void Editor::UnParentSceneGraphSelection(Node* newParent)
{
	if (selectionList.size() > 0)
	{
		for (auto obj : selectionList)
		{
			SceneGraph::Instance()->UnparentInPlace(obj->node, newParent);
		}
		selectionList.clear();
	}
}

void Editor::GLCCombo(std::unordered_map<unsigned int, const char*>& glcs, const char* label, unsigned int& currentOption)
{
	ImGui::PushItemWidth(ImGui::GetFontSize() * -20.f);
	std::string comboLabel = std::string("##Combo") + std::string(label);
	if (ImGui::BeginCombo(comboLabel.c_str(), glcs[currentOption], ImGuiComboFlags_NoArrowButton))
	{
		for (auto& glc : glcs)
		{
			const bool is_selected = glc.first == currentOption;
			if (ImGui::Selectable(glc.second, is_selected))
			{
				currentOption = glc.first;
			}
			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();
	ImGuiStyle& style = ImGui::GetStyle();
	float w = ImGui::CalcItemWidth();
	float spacing = style.ItemInnerSpacing.x;
	float button_sz = ImGui::GetFrameHeight();
	ImGui::PushItemWidth(w - spacing * 2.0f - button_sz * 2.0f);
	ImGui::SameLine(0, spacing);
	int currentOptionIndex = 0;
	for (auto& glc : glcs)
	{
		if (glc.first == currentOption)
		{
			break;
		}
		currentOptionIndex++;
	}
	if (ImGui::ArrowButton("##leftArrow", ImGuiDir_Left))
	{
		auto glcIt = glcs.begin();
		if (currentOptionIndex <= 0)
		{
			std::advance(glcIt, glcs.size() - 1);
		}
		else
		{
			std::advance(glcIt, currentOptionIndex - 1);
		}
		currentOption = (*glcIt).first;
	}
	ImGui::SameLine(0, spacing);
	if (ImGui::ArrowButton("##rightArrow", ImGuiDir_Right))
	{
		auto glcIt = glcs.begin();
		if (currentOptionIndex < glcs.size() - 1)
		{
			std::advance(glcIt, currentOptionIndex + 1);
		}
		currentOption = (*glcIt).first;
	}
	ImGui::SameLine();
	ImGui::Text(label);
	ImGui::PopItemWidth();
}

void Editor::LoadStyle()
{
	ImGuiIO& io = ImGui::GetIO();
	//io.Fonts->AddFontFromFileTTF("path/to/font/file.ttf", 10.f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
	ImGui::PushStyleColor(ImGuiCol_Tab, IM_COL32(61, 61, 61, 255));
	ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(61, 61, 61, 255));
	ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(40, 40, 40, 255));
	ImGui::PushStyleColor(ImGuiCol_CheckMark, IM_COL32(0, 255, 255, 255));

	//ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 190, 190, 255));
	//ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(0, 225, 225, 255));
	//ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 200, 200, 255));
	//ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0, 225, 225, 255));
	//ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(0, 200, 200, 255));
	//ImGui::PushStyleColor(ImGuiCol_TabHovered, IM_COL32(0, 225, 225, 255));
	//ImGui::PushStyleColor(ImGuiCol_TabActive, IM_COL32(0, 200, 200, 255));
	//ImGui::PushStyleColor(ImGuiCol_TitleBgActive, IM_COL32(0, 180, 180, 255));

	ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(200, 68, 0, 255));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(200, 118, 0, 255));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(200, 76, 0, 255));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(200, 118, 0, 255));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(75, 75, 75, 255));
	ImGui::PushStyleColor(ImGuiCol_TabHovered, IM_COL32(200, 118, 0, 255));
	ImGui::PushStyleColor(ImGuiCol_TabActive, IM_COL32(200, 76, 0, 255));
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, IM_COL32(200, 60, 0, 255));
	ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(52, 52, 52, 255));
}

struct Funcs
{
	static int MyResizeCallback(ImGuiInputTextCallbackData* data)
	{
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			std::string* my_str = (std::string*)data->UserData;
			IM_ASSERT(my_str->c_str() == data->Buf);
			my_str->resize(data->BufSize);
			data->Buf = (char*)my_str->c_str();
		}
		return 0;
	}

	static bool MyInputTextMultiline(const char* label, std::string* my_str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0)
	{
		return ImGui::InputTextMultiline(label, my_str, size, flags, Funcs::MyResizeCallback, (void*)my_str);
	}
};

void Editor::ShaderCodeEditor(Shader* shader, bool newSelection)
{
	static std::string vs_str;
	static std::string fs_str;
	static std::string gs_str;
	static bool vs_dirty = false;
	static bool fs_dirty = false;
	static bool gs_dirty = false;
	static Shader* previousShader = nullptr;

	if (shader == nullptr)
	{
		vs_str = "";
		fs_str = "";
		gs_str = "";
	}

	// Always center this window when appearing
	ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Save File?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Shader %s has been modified.\nWould you like to save the changes?\n\n", previousShader->name.c_str());
		ImGui::Separator();
		if (ImGui::Button("Yes", ImVec2(120, 0)))
		{
			GraphicsManager::WriteStringToTextFile(vs_str, previousShader->shaderPaths.vs);
			GraphicsManager::WriteStringToTextFile(fs_str, previousShader->shaderPaths.fs);
			GraphicsManager::WriteStringToTextFile(gs_str, previousShader->shaderPaths.gs);
			vs_dirty = false;
			fs_dirty = false;
			gs_dirty = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("No", ImVec2(120, 0)))
		{
			vs_str = GraphicsManager::ReadTextFileIntoString(previousShader->shaderPaths.vs);
			fs_str = GraphicsManager::ReadTextFileIntoString(previousShader->shaderPaths.fs);
			gs_str = GraphicsManager::ReadTextFileIntoString(previousShader->shaderPaths.gs);
			vs_dirty = false;
			fs_dirty = false;
			gs_dirty = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (previousShader != nullptr && (vs_dirty || fs_dirty || gs_dirty) && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S))
	{
		GraphicsManager::WriteStringToTextFile(vs_str, previousShader->shaderPaths.vs);
		GraphicsManager::WriteStringToTextFile(fs_str, previousShader->shaderPaths.fs);
		GraphicsManager::WriteStringToTextFile(gs_str, previousShader->shaderPaths.gs);
		vs_dirty = false;
		fs_dirty = false;
		gs_dirty = false;
	}

	if (shader != nullptr && newSelection)
	{
		if (vs_dirty || fs_dirty || gs_dirty)
		{
			ImGui::OpenPopup("Save File?");
			shader = previousShader;
		}
		else
		{
			vs_str = GraphicsManager::ReadTextFileIntoString(shader->shaderPaths.vs);
			fs_str = GraphicsManager::ReadTextFileIntoString(shader->shaderPaths.fs);
			gs_str = GraphicsManager::ReadTextFileIntoString(shader->shaderPaths.gs);
			vs_dirty = false;
			fs_dirty = false;
			gs_dirty = false;
			previousShader = shader;
		}
	}
	ImGui::Text(vs_dirty ? "Vertex Shader*" : "Vertex Shader");
	if (Funcs::MyInputTextMultiline("Vertex Shader##VSShader", &vs_str, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16)))
	{
		vs_dirty = true;
	}
	if (ImGui::Button("Save##savevertexshader", ImVec2(120, 0)))
	{
		GraphicsManager::WriteStringToTextFile(vs_str, previousShader->shaderPaths.vs);
		vs_dirty = false;
	}
	ImGui::Text("Data: %p\nSize: %d\nCapacity: %d", (void*)vs_str.c_str(), vs_str.size(), vs_str.capacity());
	ImGui::Text(fs_dirty ? "Fragment Shader*" : "Fragment Shader");
	if (Funcs::MyInputTextMultiline("Fragment Shader##FSShader", &fs_str, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16)))
	{
		fs_dirty = true;
	}
	if (ImGui::Button("Save##savefragmentshader", ImVec2(120, 0)))
	{
		if (!fs_str.empty())
		{
			if (previousShader->shaderPaths.fs.empty())
			{
				std::string fsPath = std::format("resources/shaders/{}.fs", previousShader->name);
				previousShader->shaderPaths.fs = fsPath;
			}
			GraphicsManager::WriteStringToTextFile(fs_str, previousShader->shaderPaths.fs);
		}
		fs_dirty = false;
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete##deletefragmentshader", ImVec2(120, 0)))
	{
		ImGui::OpenPopup("Delete Fragment Shader?");
	}

	if (ImGui::BeginPopupModal("Delete Fragment Shader?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Do you want to remove fragment shader of %s shader?\n\n", previousShader->name.c_str());
		ImGui::Separator();
		if (ImGui::Button("Yes", ImVec2(120, 0)))
		{
			std::remove(previousShader->shaderPaths.fs.c_str());
			previousShader->shaderPaths.fs = "";
			fs_str = "";
			fs_dirty = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("No", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::Text("Data: %p\nSize: %d\nCapacity: %d", (void*)fs_str.c_str(), fs_str.size(), fs_str.capacity());
	ImGui::Text(gs_dirty ? "Geometry Shader*" : "Geometry Shader");
	if (Funcs::MyInputTextMultiline("Geometry Shader##GSShader", &gs_str, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16)))
	{
		gs_dirty = true;
	}
	if (ImGui::Button("Save##savegeometryshader", ImVec2(120, 0)))
	{
		if (!gs_str.empty())
		{
			if (previousShader->shaderPaths.gs.empty())
			{
				std::string gsPath = std::format("resources/shaders/{}.gs", previousShader->name);
				previousShader->shaderPaths.gs = gsPath;
			}
			GraphicsManager::WriteStringToTextFile(gs_str, previousShader->shaderPaths.gs);
		}
		gs_dirty = false;
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete##deletegeometryshader", ImVec2(120, 0)))
	{
		ImGui::OpenPopup("Delete Geometry Shader?");
	}

	if (ImGui::BeginPopupModal("Delete Geometry Shader?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Do you want to remove geometry shader of %s shader?\n\n", previousShader->name.c_str());
		ImGui::Separator();
		if (ImGui::Button("Yes", ImVec2(120, 0)))
		{
			std::remove(previousShader->shaderPaths.gs.c_str());
			previousShader->shaderPaths.gs = "";
			gs_str = "";
			gs_dirty = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("No", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::Text("Data: %p\nSize: %d\nCapacity: %d", (void*)gs_str.c_str(), gs_str.size(), gs_str.capacity());
	if (ImGui::Button("Save Shader##saveshader", ImVec2(120, 0)))
	{
		GraphicsManager::WriteStringToTextFile(vs_str, previousShader->shaderPaths.vs);
		if (!fs_str.empty())
		{
			if (previousShader->shaderPaths.fs.empty())
			{
				std::string fsPath = std::format("resources/shaders/{}.fs", previousShader->name);
				previousShader->shaderPaths.fs = fsPath;
			}
			GraphicsManager::WriteStringToTextFile(fs_str, previousShader->shaderPaths.fs);
		}
		if (!gs_str.empty())
		{
			if (previousShader->shaderPaths.gs.empty())
			{
				std::string gsPath = std::format("resources/shaders/{}.gs", previousShader->name);
				previousShader->shaderPaths.gs = gsPath;
			}
			GraphicsManager::WriteStringToTextFile(gs_str, previousShader->shaderPaths.gs);
		}
		vs_dirty = false;
		fs_dirty = false;
		gs_dirty = false;
	}
}

void Editor::ScriptEditor(Script* script, bool newSelection)
{
	ImGui::PushID(script->path.c_str());
	static std::string lua_str;
	static bool lua_dirty = false;
	if (script != nullptr && newSelection)
	{
		if (lua_dirty)
		{
			ImGui::OpenPopup("Save File?");
		}
		else
		{
			lua_str = GraphicsManager::ReadTextFileIntoString(script->path);
		}
	}
	if (script != nullptr)
	{
		if (Funcs::MyInputTextMultiline("Script##scriptInput", &lua_str, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16)))
		{
			lua_dirty = true;
		}
		if (ImGui::Button("Save##saveScript", ImVec2(120, 0)))
		{
		
			GraphicsManager::WriteStringToTextFile(lua_str, script->path);
			lua_dirty = false;
		}
		ImGui::Text("Data: %p\nSize: %d\nCapacity: %d", (void*)lua_str.c_str(), lua_str.size(), lua_str.capacity());
	}
}

void Editor::Stats()
{
	ImGui::Begin("Stats", &showStats);
	
	if (ImGui::TreeNode("Modes"))
	{
		if (Times::Instance()->paused) ImGui::Text("PAUSED");
		if (pausedPhysics) ImGui::Text("PAUSED PHYSICS");
		if (drawBB) ImGui::Text("DRAW BB");
		if (drawLines) ImGui::Text("LINES ON");
		if (drawPoints) ImGui::Text("POINTS ON");
		if (drawParticles) ImGui::Text("PARTICLES ON");
		if (drawMaps) ImGui::Text("MAPS ON");
		ImGui::NewLine();
		ImGui::TreePop();
	}
	ImGui::Text("Draw calls %d", Render::Instance()->totalNrOfDrawCalls);
	ImGui::Text("TimeStep %.6f", Times::Instance()->paused ? 0.0 : 1.0 / Times::Instance()->timeStep);
	ImGui::Text("Total FPS %.3f", 1.0 / Times::Instance()->deltaTime);
	ImGui::Text("Total MS %.3f", Times::Instance()->deltaTime * 1000.0);
	ImGui::DragInt("Update Average After N Frames", &Times::Instance()->averageFPSafterNFrames, 1, 1, 1000);
	ImGui::Text("Average Total FPS %.3f", 1.0 / Times::Instance()->averageDeltaTime);
	ImGui::Text("Average Total MS %.3f", Times::Instance()->averageDeltaTime * 1000.0);
	ImGui::Text("Total - PhysicsSceneFrustumGraph MS %.6f", (Times::Instance()->deltaTime - updateTime)*1000.0);
	ImGui::Text("PhysicsSceneFrustumGraph MS %.6f", updateTime * 1000.0);
	ImGui::Text("Frustum Culling MS %.6f", frustumCullingTime * 1000.0);
	ImGui::Text("CPU Graph ElementCount MS %.8f", Render::Instance()->countingElementsTime * 1000);
	ImGui::Text("CPU Graph TreeGeneration MS %.8f", Render::Instance()->treeGenerationTime * 1000);
	ImGui::Text("CPU Graph TotalGeneration MS %.8f", Render::Instance()->totalGenerationTime * 1000);
	ImGui::Text("Executing graph MS %.8f", Render::Instance()->executingGraphTime * 1000);
	ImGui::Text("Render Time MS %.6f", renderTime * 1000.0);
	ImGui::Text("Swap Buffers Time MS %.6f", swapBuffersTime * 1000.0);
	ImGui::Text("Objects rendered %d", objectsRendered);
	ImGui::Text("IObjects rendered %d", instancedGeometryDrawn);
	ImGui::Text("BBs rendered %d", DebugDraw::Instance()->boundingBoxesDrawn);
	ImGui::Text("Lights rendered %d", lightsRendered);
	ImGui::Text("Particles rendered %d", particlesRendered);
	ImGui::Text("Update Dynamic Array Time %.6f", SceneGraph::Instance()->updateDynamicArrayTime);
	ImGui::Text("Update Transforms Time %.6f", SceneGraph::Instance()->updateTransformsTime);
	ImGui::Text("Update Bounds Time %.6f", Bounds::updateBoundsTime);
	ImGui::Text("Update MinMax Time %.6f", Bounds::updateMinMaxTime);
	ImGui::Text("Update Components Time %.6f", SceneGraph::Instance()->updateComponentsTime);
	ImGui::Text("PickedID %d", pickedID);
	ImGui::Text("PRUNE %.8f", PhysicsManager::Instance()->pruneAndSweepTime);
	ImGui::Text("SAT %.8f", PhysicsManager::Instance()->satTime);
	ImGui::Text("Intersection Test %.8f", PhysicsManager::Instance()->intersectionTestTime);
	ImGui::Text("Generate Contacts %.8f", PhysicsManager::Instance()->generateContactsTime);
	ImGui::Text("Process Contacts %.8f", PhysicsManager::Instance()->processContactTime);
	ImGui::Text("Positional Correction %.8f", PhysicsManager::Instance()->positionalCorrectionTime);
	ImGui::Text("Iterations Count %d", PhysicsManager::Instance()->iterCount);
	
	ImGui::End();
}