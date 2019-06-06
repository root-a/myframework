#pragma once
#include <vector>
#include <unordered_map>
#include "MyMathLib.h"
#include "LightProperties.h"

namespace mwm
{
	class Matrix4;
}

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

class Render
{
	typedef unsigned int GLuint;
	typedef unsigned int GLenum;

	struct PostHDRBloom
	{
		float gamma;
		float exposure;
		float brightness;
		float contrast;
		float bloomIntensity;
		bool hdrEnabled;
		bool alignmentOffset;
		bool alignmentOffset2;
		bool alignmentOffset3;
		bool bloomEnabled;
		bool alignmentOffset4;
		bool alignmentOffset5;
		bool alignmentOffset6;
	};

public:

	static Render* Instance();
	void captureToTexture2D(const GLuint shaderID, FrameBuffer * captureFBO, GLuint captureRBO, Texture* textureToDrawTo);
	void captureTextureToCubeMapWithMips(const GLuint shaderID, FrameBuffer* captureFBO, GLuint captureRBO, Texture* textureToCapture, Texture* textureToDrawTo);
	void captureTextureToCubeMap(const GLuint shaderID, FrameBuffer* captureFBO, GLuint captureRBO, Texture* textureToCapture, Texture* textureToDrawTo);
	int drawGeometry(const GLuint shaderID, const std::vector<Object*>& objects, FrameBuffer * geometryBuffer, const GLenum * attachmentsToDraw = nullptr, const int countOfAttachments = 0);
	int drawInstancedGeometry(const GLuint shaderID, const std::vector<InstanceSystem*>& iSystems, FrameBuffer * geometryBuffer);
	int drawFastInstancedGeometry(const GLuint shaderID, const std::vector<FastInstanceSystem*>& iSystems, FrameBuffer * geometryBuffer);
	int draw(const GLuint shaderID, const std::vector<Object*>& objects, const mwm::Matrix4& ViewProjection);
	int drawLight(const GLuint pointLightShader, const GLuint pointLightShadowShader, const GLuint spotLightShader, const GLuint spotLightShadowShader, const GLuint directionalLightShader, const GLuint directionalLightShadowShader, FrameBuffer* lightFrameBuffer, FrameBuffer* geometryBuffer, const GLenum * attachmentsToDraw = nullptr, const int countOfAttachments = 0);
	void drawSingle(const GLuint shaderID, const Object* object, const mwm::Matrix4& ViewProjection, const GLuint currentShaderID);
	int drawPicking(const GLuint shaderID, std::unordered_map<unsigned int, Object*>& pickingList, FrameBuffer* pickingBuffer, const GLenum * attachmentsToDraw = nullptr, const int countOfAttachments = 0);
	int drawDepth(const GLuint shaderID, const std::vector<Object*>& objects, const mwm::Matrix4& ViewProjection);
	int drawCubeDepth(const GLuint shaderID, const std::vector<Object*>& objects, const std::vector<mwm::Matrix4>& ViewProjection, const Object* light);
	void drawSkyboxWithClipPlane(const GLuint shaderID, FrameBuffer * lightFrameBuffer, Texture* texture, const mwm::Vector4F& plane, const mwm::Matrix4& ViewMatrix);
	void drawSkybox(const GLuint shaderID, FrameBuffer * lightFrameBuffer, Texture* texture);
	void drawGSkybox(const GLuint shaderID, FrameBuffer * lightFrameBuffer, Texture* texture);
	int drawAmbientLight(const GLuint shaderID, FrameBuffer* bufferToDrawTheLightTO, const std::vector<Texture*>& geometryTextures, const std::vector<Texture*> &pbrEnvTextures);
	int drawDirectionalLights(const GLuint shaderID, const GLuint shadowShaderID, const std::vector<DirectionalLight*>& lights, const std::vector<Object*>& objects, FrameBuffer* bufferToDrawTheLightTO, const std::vector<Texture*>& geometryTextures);
	int drawPointLights(const GLuint shaderID, const GLuint shadowShaderID, const std::vector<PointLight*>& lights, const std::vector<Object*>& objects, const mwm::Matrix4& ViewProjection, FrameBuffer* fboToDrawTheLightTO, const std::vector<Texture*>& geometryTextures);
	int drawSpotLights(const GLuint shaderID, const GLuint shadowShaderID, const std::vector<SpotLight*>& lights, const std::vector<Object*>& objects, const mwm::Matrix4& ViewProjection, FrameBuffer* fboToDrawTheLightTO, const std::vector<Texture*>& geometryTextures);
	void drawHDR(const GLuint shaderID, Texture* colorTexture, Texture* bloomTexture);
	void drawHDRequirectangular(const GLuint shaderID, Texture* colorTexture);
	void drawRegion(const GLuint shaderID, int posX, int posY, int width, int height, const Texture* texture);
	void AddPingPongBuffer(int width, int height);
	void AddMultiBlurBuffer(int width, int height, int levels = 4, double scaleX = 0.5, double scaleY = 0.5);
	void GenerateEBOs();
	void UpdateEBOs();
	Texture* PingPongBlur(Texture* sourceTexture, int outputLevel, float blurSize, GLuint blurShader);
	Texture* MultiBlur(Texture* sourceTexture, int outputLevel, float blurSize, GLuint blurShader);
	FrameBuffer* AddDirectionalShadowMapBuffer(int width, int height);
	FrameBuffer* dirShadowMapBuffer;
	Texture* dirShadowMapTexture;
	PostHDRBloom pb;
	float angleX = 0.f;
	float angleY = 0.f;
	std::vector<mwm::Matrix4F> captureVPs; //used when drawing depth
private:
	void BlurOnOneAxis(Texture* sourceTexture, FrameBuffer* destinationFbo, float offsetxVal, float offsetyVal, GLuint offsetHandle);
	Texture* BlurTexture(Texture* sourceTexture, std::vector<FrameBuffer*> startFrameBuffer, std::vector<FrameBuffer*> targetFrameBuffer, int outputLevel, float blurSize, GLuint shader, int windowWidth, int windowHeight);
	Texture* BlurTextureAtSameSize(Texture* sourceTexture, FrameBuffer* startFrameBuffer, FrameBuffer* targetFrameBuffer, int outputLevel, float blurSize, GLuint shader, int windowWidth, int windowHeight);
    Render();
    ~Render();
	
	FrameBuffer* pingPongBuffers[2];
	std::vector<FrameBuffer*> multiBlurBufferStart;
	std::vector<FrameBuffer*> multiBlurBufferTarget;
	
	
	struct GBVars
	{
		mwm::Matrix4F MVP;
		mwm::Matrix4F M;
		mwm::Vector4F MaterialColorShininess;
		mwm::Vector2F tiling;
		unsigned int objectID;
	};

	GBVars gb;

	struct LightVars
	{
		mwm::Matrix4F depthBiasMVP;
		mwm::Vector3F lightInvDir;
		float shadowTransitionSize;
		float outerCutOff;
		float innerCutOff;
		float lightRadius;
		float lightPower;
		mwm::Vector3F lightColor;
		float ambient;
		float diffuse;
		float specular;
		float alignmentOffset;
		float alignmentOffset2;
		mwm::Matrix4F MVP;
		mwm::Vector3F lightPosition;
		Attenuation attenuation;
	};

	LightVars lb;

	struct CamVars
	{
		float width;
		float height;
		float near;
		float far;
		mwm::Vector3F cameraPos;
	};

	CamVars cb;

	GLuint uboGBVars;
	GLuint uboLBVars;
	GLuint uboCBVars;
	GLuint uboPBVars;
};