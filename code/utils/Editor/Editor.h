#pragma once
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <typeindex>
#include <string>
#include <gl/glew.h>
class DataRegistry;
class Object;
class Texture;
class RenderBuffer;
enum class MaterialElements;
struct ImGuiTextFilter;
class Material;
class Script;
class RenderPass;
class ShaderBlock;
class RenderElement;
class ObjectProfile;
class Shader;
class Node;
class Component;
struct DataInfo;
struct ImVec2;
struct ImVec4;
class Camera;
class Vector3;
class Matrix4F;
class OBJ;
class RenderProfile;
class TextureProfile;
class MaterialProfile;
class FrameBuffer;
enum ImGuiKey;

class Editor
{
private:
	struct CubemapPreview {
		GLuint cubemapTexture;
		GLuint fbo;
		GLuint previewTexture;
		GLuint depthBuffer;
		int faceSize;
		int previewWidth;
		int previewHeight;
		bool needsUpdate;
	};
public:
	Editor();
	~Editor();
	bool CreateNewAssetUI(char* nameBuffer, bool copy = false);
	Object* CreateNewObject(char* nameBuffer);
	Object* CreateDefaultObject(char* name);
	Object* CreateObjectFromTemplate(char* path);
	Object*DuplicateObject(Object* object);
	Material* CreateNewMaterial(char* nameBuffer);
	void ImportNewMesh(char* nameBuffer);
	void ImportNewTexture(char* nameBuffer);
	bool CreateNewRenderTargetUI(char* nameBuffer);
	void CreateNewRenderTarget(char* nameBuffer);
	bool CreateNewRenderBufferUI(char* nameBuffer);
	RenderBuffer* CreateNewRenderBuffer(char* nameBuffer);
	FrameBuffer* CreateNewFrameBuffer(char* nameBuffer);
	RenderPass* CreateNewRenderPass(char* nameBuffer);
	MaterialProfile* CreateNewMaterialProfile(char* nameBuffer);
	TextureProfile* CreateNewTextureProfile(char* nameBuffer);
	RenderProfile* CreateNewRenderProfile(char* nameBuffer);
	ObjectProfile* CreateNewObjectProfile(char* nameBuffer);
	Shader* CreateNewShader(char* nameBuffer);
	void CreateNewScript(char* nameBuffer);
	void ObjectEditor(Object* object);
	void ObjectInspector(Object* object, bool newSelection);
	void DrawComponentUI(ImGuiTextFilter& filter, Object* object, Component* component, const std::string& name, bool isDynamic, bool newSelection);
	bool DataRegistryEditor(DataRegistry* registry);
	void DrawPropertyUI(DataInfo& info, const std::string& name);
	bool LuaProperties(DataRegistry* registry, const char* label, const char* pathToLua, void* registerArgument);
	bool ObjectProperties(DataRegistry* registry, const char* label);
	void SceneGraphInspector();
	void DatabaseInspector();
	void AssetImport(const char** paths, int count);
	void LoadAndSaveMeshes();
	void SaveMeshData(OBJ* obj);
	void CreateAndSaveVao(OBJ* obj, std::string configPath, std::string meshPath);
	void LoadOBJSToVaos(std::unordered_map<std::string, OBJ*>& objs);
	void ImportUI();
	void EditTransform(float* matrix, Object* object);
	std::string GetComponentName(const std::type_index& type);
	void ObjectsInspector();
	void MaterialsInspector();
	void OBJsInspector();
	void VAOsInspector();
	void VBOsInspector();
	void TexturesInspector();
	void RenderTargetsInspector();
	void CubeMapsInspector();
	void RenderBuffersInspector();
	void FrameBuffersInspector();
	void FrameBufferInspector(FrameBuffer* fbo, bool resetInspectors = false);
	void RenderPassesInspector();
	void RenderQueueInspector();
	void ShadersInspector();
	void ShaderBlockInspector();
	void BuffersDefinitionsInspector();
	void MaterialProfilesInspector();
	void TextureProfilesInspector();
	void ClearThumbnailResources();
	GLuint CreateCubemapPreview(GLuint cubemapTexture, const ImVec2& size);
	std::unordered_map<unsigned int, CubemapPreview> cubemapPreviews;
	template<typename T>
	bool DrawTexturesThumbnailsWithLabel(ImGuiTextFilter& filterAddSlots, T** selectedTexture, std::vector<T*>& textures, float visibleWindowWidth);
	template<typename T>
	bool DrawSelectableTextureThumbnail(T* texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col);
	void RenderProfilesInspector();
	void ObjectProfilesInspector();
	bool DataRegistryInspector(DataRegistry* registry, const char* pathToLuas, void* ownerPtr);
	void TextureParameterComboIV(Texture* texture, int propertyName, const char* comboLabel, std::unordered_map<unsigned int, const char*> options, int currentValue);
	void TextureThumbnails(std::vector<Texture*>& textures, Texture** selectedTexture, bool flip = false);
	void TexturesInspector(std::vector<Texture*>& textures, Texture** selectedTexture, bool flip = false);
	void RenderBufferInspector(RenderBuffer* rb);
	void TextureThumbnailsBasic(std::vector<Texture*>& textures, Texture* selected);
	void TextureThumbnailsBasicRB(std::vector<RenderBuffer*>& textures, RenderBuffer* selected);
	void TextureBasicDetails(Texture* texture);
	void TexturesInspectorBasic(std::vector<Texture*>& textures, bool reset = false);
	void RenderBufferBasicDetails(RenderBuffer* renderBuffer);
	void RenderBuffersInspectorBasic(std::vector<RenderBuffer*>& rbuffers, bool reset = false);
	void AllTexturesInMemoryInspector();
	void MiscTools();
	void OverlayStats();
	void MainMenuBar();
	template<typename T>
	bool GenerateElementComboBox(Material* mat, ImGuiTextFilter* filter, MaterialElements index, const char* label, T* elementList);
	void GenerateMaterialElementsUI(Material* mat);
	void ShaderOutputsToFrameBufferRenderTargetsUI(Material* mat);
	void ScriptSelector(Script* script, const char* directory);
	void RenderPassInspector(RenderPass* pass);
	void ShaderBlocks(ObjectProfile* profile);
	void ShaderBlocksInspector(ShaderBlock* ub);
	void InitializeGLC();
	void GenerateGUI();
	void AddObjectToSceneGrapghSelection(Object* obj, bool isSelected);
	void DrawSceneGraphNode(Node* child);
	void ParentSceneGraphSelection();
	void UnParentSceneGraphSelection(Node* newParent);
	bool UnParentAction = false;
	Node* NewUnparentParent = nullptr;
	void GLCCombo(std::unordered_map<unsigned int, const char*>& glcs, const char* label, unsigned int& currentOption);
	void LoadStyle();
	void ShaderCodeEditor(Shader* shader, bool newSelection);
	void ScriptEditor(Script* script, bool newSelection);
	void GenerateSceneGraphChildren(Node* node);
	void Stats();
	std::vector<Object*> selectionList;
	bool showObjectEditor = false;
	bool showSceneGraphInspector = false;
	bool showDatabaseInspector = false;
	bool showStyleEditor = false;
	bool showImGuiDemo = false;
	bool showMiscTools = false;
	bool showOverlayStats = true;
	bool vsyncEnabled = true;
	bool showOverlayMouseInfo = false;
	bool showStats = false;
	bool toggleUIonAlt = false;
	bool toggleUI = true;
	bool showImportWindow = false;

	unsigned int pickedID = 0;
	Camera* testCamera;
	Object* firstObject = nullptr;
	Object* secondObject = nullptr;

	bool post = true;
	float blurBloomSize = 0.3f;
	int bloomLevel = 1;
	bool pausedPhysics = false;

	float softScale = 0.5f;
	float contrastPower = 0.5f;
	Shader* particlesShader;

	bool drawBB = false;
	bool drawLines = false;
	bool drawPoints = false;
	bool drawParticles = true;
	bool drawMaps = true;

	double updateTime = 0.0;
	double renderTime = 0.0;
	double frustumCullingTime = 0.0;
	double swapBuffersTime = 0.0;

	int objectsRendered = 0;
	int lightsRendered = 0;
	int particlesRendered = 0;
	int instancedGeometryDrawn = 0;
	Object* lastPickedObject = nullptr;

	unsigned int currentTarget;
	int currentLevel;
	unsigned int currentInternalFormat;
	int currentWidthHeight[2];
	unsigned int currentFormat;
	unsigned int currentType;
	unsigned int currentAttachment;

	std::unordered_map<unsigned int, const char*> targets;
	std::unordered_map<unsigned int, const char*> internalFormats;
	std::unordered_map<unsigned int, const char*> formats;
	std::unordered_map<unsigned int, const char*> types;
	std::unordered_map<unsigned int, const char*> attachments;
	std::unordered_map<unsigned int, const char*> parameterNames;
	std::unordered_map<unsigned int, const char*> depthStencilModes;
	std::unordered_map<unsigned int, const char*> imageFormatCompatibilityTypes;
	std::unordered_map<unsigned int, const char*> compareModes;
	std::unordered_map<unsigned int, const char*> compareFuncs;
	std::unordered_map<unsigned int, const char*> immutableFormat;
	std::unordered_map<unsigned int, const char*> minFilters;
	std::unordered_map<unsigned int, const char*> magFilters;
	std::unordered_map<unsigned int, const char*> swizzles;
	std::unordered_map<unsigned int, const char*> wrapFunctions;
	std::unordered_map<unsigned int, const char*> indicesTypes;
	std::unordered_map<unsigned int, const char*> glShaderDataTypes;
	std::unordered_map<std::string, std::string> pathsAndConfigurations;

	template<typename UIPropertiesFunction, typename UIMenuItemsFunction>
	inline void DrawComponentBasic(Component* component, const std::string& name, bool dynamic, Object* object, const UIPropertiesFunction& uiFunction, const UIMenuItemsFunction& menuItems);
	std::vector<OBJ*> importedObjs;
private:
	

	class KeyToggle {
	public:
		KeyToggle() : isToggled(false), wasPressed(false), key((ImGuiKey)0) {}
		KeyToggle(ImGuiKey key) : isToggled(false), wasPressed(false), key(key) {}

		void Update(bool isPressed) {
			if (isPressed && !wasPressed) {
				isToggled = !isToggled;
			}

			wasPressed = isPressed;
		}

		bool IsToggled() const {
			return isToggled;
		}

	private:
		bool isToggled;
		bool wasPressed;
		ImGuiKey key;
	};
	std::unordered_map<ImGuiKey, KeyToggle> keyToggles;
};
