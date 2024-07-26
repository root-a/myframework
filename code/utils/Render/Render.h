#pragma once
#include <vector>
#include <unordered_map>
#include "MyMathLib.h"
#include "LightProperties.h"
//#include "UniformBuffer.h"
//#include "CPUBufferData.h"
#include "DataRegistry.h"
#include "MaterialProfile.h"
#include <array>
#include "GraphicsStorage.h"

class Matrix4;
class Object;
class DirectionalLight;
class SpotLight;
class PointLight;
class FrameBuffer;
class Texture;
class ParticleSystem;
class InstanceSystem;
class FastInstanceSystem;
class RenderPass;
struct RenderNode;

class Render
{
	typedef unsigned int GLuint;
	typedef unsigned int GLenum;

public:

	static Render* Instance();
	void captureToTexture2D(const GLuint shaderID, FrameBuffer * captureFBO, Texture* textureToDrawTo);
	void captureTextureToCubeMapWithMips(const GLuint shaderID, FrameBuffer* captureFBO, Texture* textureToCapture, Texture* textureToDrawTo);
	void captureTextureToCubeMap(const GLuint shaderID, FrameBuffer* captureFBO, Texture* textureToCapture, Texture* textureToDrawTo);
	int drawGeometry(const GLuint shaderID, const std::vector<Object*>& objects, FrameBuffer * geometryBuffer, const GLenum * attachmentsToDraw = nullptr, const int countOfAttachments = 0);
	int drawInstancedGeometry(const GLuint shaderID, PoolParty<InstanceSystem, 1000>& iSystems, FrameBuffer * geometryBuffer);
	int drawFastInstancedGeometry(const GLuint shaderID, PoolParty<FastInstanceSystem, 1000>& iSystems, FrameBuffer * geometryBuffer);
	int draw(const GLuint shaderID, const std::vector<Object*>& objects, const glm::mat4& ViewProjection);
	int drawLight(const GLuint pointLightShader, const GLuint pointLightShadowShader, const GLuint spotLightShader, const GLuint spotLightShadowShader, const GLuint directionalLightShader, const GLuint directionalLightShadowShader, FrameBuffer* lightFrameBuffer, FrameBuffer* geometryBuffer, const GLenum * attachmentsToDraw = nullptr, const int countOfAttachments = 0);
	void drawSingle(const GLuint shaderID, const Object* object, const glm::mat4& ViewProjection, const GLuint currentShaderID);
	int drawPicking(const GLuint shaderID, std::unordered_map<unsigned int, Object*>& pickingList, FrameBuffer* pickingBuffer, const GLenum * attachmentsToDraw = nullptr, const int countOfAttachments = 0);
	int drawDepth(const GLuint shaderID, const std::vector<Object*>& objects, const glm::mat4& ViewProjection);
	int drawCubeDepth(const GLuint shaderID, const std::vector<Object*>& objects, const std::vector<glm::mat4>& ViewProjection, const Object* light);
	void drawSkyboxWithClipPlane(const GLuint shaderID, FrameBuffer * lightFrameBuffer, Texture* texture, const glm::vec4& plane, const glm::mat4& ViewMatrix);
	void drawSkybox(const GLuint shaderID, FrameBuffer * lightFrameBuffer, Texture* texture);
	void drawGSkybox(const GLuint shaderID, FrameBuffer * lightFrameBuffer, Texture* texture);
	int drawAmbientLight(const GLuint shaderID, FrameBuffer* bufferToDrawTheLightTO, const std::vector<Texture*>& geometryTextures, const std::vector<Texture*> &pbrEnvTextures);
	int drawDirectionalLights(const GLuint shaderID, const GLuint shadowShaderID, PoolParty<DirectionalLight>& lights, const std::vector<Object*>& objects, FrameBuffer* bufferToDrawTheLightTO, const std::vector<Texture*>& geometryTextures);
	int drawPointLights(const GLuint shaderID, const GLuint shadowShaderID, PoolParty<PointLight>& lights, const std::vector<Object*>& objects, const glm::mat4& ViewProjection, FrameBuffer* fboToDrawTheLightTO, const std::vector<Texture*>& geometryTextures);
	int drawSpotLights(const GLuint shaderID, const GLuint shadowShaderID, PoolParty<SpotLight>& lights, const std::vector<Object*>& objects, const glm::mat4& ViewProjection, FrameBuffer* fboToDrawTheLightTO, const std::vector<Texture*>& geometryTextures);
	void drawHDR(const GLuint shaderID, Texture* colorTexture, Texture* bloomTexture = nullptr);
	void drawHDRequirectangular(const GLuint shaderID, Texture* colorTexture);
	void drawRegion(const GLuint shaderID, int posX, int posY, int width, int height, const Texture* texture);
	void drawParticles(const GLuint shaderID, std::vector<ParticleSystem*>& particleSystems, FrameBuffer* targetFrameBuffer);
	void AddPingPongBuffer(int width, int height);
	void AddMultiBlurBuffer(int width, int height, int levels = 4, double scaleX = 0.5, double scaleY = 0.5);
	void InitializeShderBlockDatas();
	void UpdateShaderBlockDatas();
	Texture* PingPongBlur(Texture* sourceTexture, int outputLevel, float blurSize, GLuint blurShader);
	Texture* MultiBlur(Texture* sourceTexture, int outputLevel, float blurSize, GLuint blurShader);
	FrameBuffer* AddDirectionalShadowMapBuffer(int width, int height);
	FrameBuffer* dirShadowMapBuffer;
	Texture* dirShadowMapTexture;

	float gamma;
	float exposure;
	float brightness;
	float contrast;
	float bloomIntensity;
	bool hdrEnabled;
	bool bloomEnabled;

	float angleX = 0.f;
	float angleY = 0.f;
	std::vector<glm::mat4> captureVPs; //used when drawing depth
	void GenerateGraph();
	void RenderGraph();
	std::vector<RenderElement*> finalRenderList;
	double treeGenerationTime;
	double countingElementsTime;
	double executingGraphTime;
	double totalGenerationTime;
	unsigned int totalNrOfDrawCalls;
	bool showRenderList = false;
private:
	void BlurOnOneAxis(Texture* sourceTexture, FrameBuffer* destinationFbo, float offsetxVal, float offsetyVal, GLuint offsetHandle);
	Texture* BlurTexture(Texture* sourceTexture, std::vector<FrameBuffer*>& startFrameBuffer, std::vector<FrameBuffer*>& targetFrameBuffer, int outputLevel, float blurSize, GLuint shader, int windowWidth, int windowHeight);
	Texture* BlurTextureAtSameSize(Texture* sourceTexture, FrameBuffer* startFrameBuffer, FrameBuffer* targetFrameBuffer, int outputLevel, float blurSize, GLuint shader, int windowWidth, int windowHeight);
    Render();
    ~Render();
	VertexArray* previousVao;
	VertexArray* currentVao;
	std::unordered_map<RenderElement*, std::vector<std::vector<Material*>*>> uniqueMaterialSequencesPerPass;
	FrameBuffer* pingPongBuffers[2];
	std::vector<FrameBuffer*> multiBlurBufferStart;
	std::vector<FrameBuffer*> multiBlurBufferTarget;
	//geometry
	ShaderBlockData* o_gbd;
	ShaderBlockData* m_gbd;
	//light material buffer
	ShaderBlockData* m_lbd;
	//directional
	ShaderBlockData* o_ldbd;
	ShaderBlockData* o_ldsbd;
	//spot
	ShaderBlockData* o_lsbd;
	ShaderBlockData* o_lssbd;
	//point
	ShaderBlockData* o_lpbd;
	//camera
	ShaderBlockData* g_csbd;
	//post
	ShaderBlockData* g_psbd;
	//time
	ShaderBlockData* g_tsbd;
	//depth
	ShaderBlockData* m_lvpbd;
	inline void FindLeastDifferentMaterial(std::vector<RenderElement*>& currentMaterial, std::vector<std::vector<Material*>*>& listOfMaterialSequences, int startFrom, int& outDifferencesCount, Material* outLeastDifferentMaterial, int& outLeastDifferentMaterialIndex);
	inline void UpdateCurrentMaterialAndRenderList(std::vector<RenderElement*>& currentMaterial, std::vector<RenderElement*>& renderList, std::vector<Material*>& materialSequence);
};