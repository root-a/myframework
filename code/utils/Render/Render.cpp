#include "Render.h"
#include "MyMathLib.h"
#include <GL/glew.h>
#include "Object.h"
#include "Material.h"
#include "Mesh.h"
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "PointLight.h"
#include "Frustum.h"
#include "FBOManager.h"
#include "FrameBuffer.h"
#include "ShaderManager.h"
#include "Attenuation.h"
#include <algorithm>
#include "CameraManager.h"
#include "Scene.h"
#include "Camera.h"
#include "Texture.h"

using namespace mwm;

Render::Render()
{
}

Render::~Render()
{
}

Render*
Render::Instance()
{
	static Render instance;

	return &instance;
}

int 
Render::drawGeometry(const std::vector<Object*>& objects, const FrameBuffer * geometryBuffer, const GLenum * lightAttachmentsToDraw, const int countOfAttachments)
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, geometryBuffer->handle);
	glDrawBuffers(countOfAttachments, lightAttachmentsToDraw);
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	GLuint geometryShader = ShaderManager::Instance()->shaderIDs["geometry"];
	ShaderManager::Instance()->SetCurrentShader(geometryShader);

	
	GLuint MatrixHandle = glGetUniformLocation(geometryShader, "MVP");
	GLuint ModelMatrixHandle = glGetUniformLocation(geometryShader, "M");
	GLuint MaterialPropertiesHandle = glGetUniformLocation(geometryShader, "MaterialProperties");
	GLuint MaterialColorHandle = glGetUniformLocation(geometryShader, "MaterialColor");
	GLuint PickingObjectIndexHandle = glGetUniformLocation(geometryShader, "objectID");
	GLuint Tiling = glGetUniformLocation(geometryShader, "tiling");
	GLuint TextureSamplerHandle = glGetUniformLocation(geometryShader, "myTextureSampler");
	glUniform1i(TextureSamplerHandle, 0);

	glActiveTexture(GL_TEXTURE0);

	int objectsRendered = 0;
	for (auto& object : objects)
	{
		if (FrustumManager::Instance()->isBoundingSphereInView(object->node.centeredPosition, object->radius))
		{
			Matrix4F ModelMatrix = object->node.TopDownTransform.toFloat();
			Matrix4F MVP = (object->node.TopDownTransform*CameraManager::Instance()->ViewProjection).toFloat();
			
			glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixHandle, 1, GL_FALSE, &ModelMatrix[0][0]);
			Vector4F matProperties = Vector4F(object->mat->metallic, object->mat->diffuseIntensity, object->mat->specularIntensity, object->mat->shininess);
			glUniform4fv(MaterialPropertiesHandle, 1, &matProperties.x);
			glUniform3fv(MaterialColorHandle, 1, &object->mat->color.x);			
			glUniform1ui(PickingObjectIndexHandle, object->ID);
			glUniform2f(Tiling, object->mat->tileX, object->mat->tileY);
			
			object->mat->texture->Bind();

			glBindVertexArray(object->mesh->vaoHandle);

			glDrawElements(GL_TRIANGLES, object->mesh->indicesSize, GL_UNSIGNED_INT, (void*)0);

			objectsRendered++;
		}
	}

	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	return objectsRendered;
}

int 
Render::draw(const std::vector<Object*>& objects, const Matrix4& ViewProjection, const GLuint currentShaderID)
{
	int objectsRendered = 0;

	GLuint MatrixHandle = glGetUniformLocation(currentShaderID, "MVP");
	GLuint ModelMatrixHandle = glGetUniformLocation(currentShaderID, "M");
	GLuint MaterialPropertiesHandle = glGetUniformLocation(currentShaderID, "MaterialProperties");
	GLuint MaterialColorHandle = glGetUniformLocation(currentShaderID, "MaterialColor");
	GLuint PickingObjectIndexHandle = glGetUniformLocation(currentShaderID, "objectID");
	GLuint tiling = glGetUniformLocation(currentShaderID, "tiling");
	GLuint TextureSamplerHandle = glGetUniformLocation(currentShaderID, "myTextureSampler");
	glUniform1i(TextureSamplerHandle, 0);

	glActiveTexture(GL_TEXTURE0);

	for (auto& object : objects)
	{
		if (FrustumManager::Instance()->isBoundingSphereInView(object->node.centeredPosition, object->radius))
		{
			Matrix4F ModelMatrix = object->node.TopDownTransform.toFloat();
			Matrix4F MVP = (object->node.TopDownTransform*ViewProjection).toFloat();

			
			glUniform2f(tiling, object->mat->tileX, object->mat->tileY);

			glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixHandle, 1, GL_FALSE, &ModelMatrix[0][0]);

			Vector4F matProperties = Vector4F(object->mat->metallic, object->mat->diffuseIntensity, object->mat->specularIntensity, object->mat->shininess);
			glUniform4fv(MaterialPropertiesHandle, 1, &matProperties.x);
			glUniform3fv(MaterialColorHandle, 1, &object->mat->color.x);

			glUniform1ui(PickingObjectIndexHandle, object->ID);

			object->mat->texture->Bind();
			
			glBindVertexArray(object->mesh->vaoHandle);

			glDrawElements(GL_TRIANGLES, object->mesh->indicesSize, GL_UNSIGNED_INT, (void*)0);
			
			objectsRendered++;
		}
	}
	return objectsRendered;
}

int
Render::drawLight(const FrameBuffer * lightFrameBuffer, const FrameBuffer * geometryBuffer, const GLenum * lightAttachmentsToDraw, const int countOfAttachments)
{
	int lightsRendered = 0;
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lightFrameBuffer->handle);
	
	glDrawBuffers(countOfAttachments, lightAttachmentsToDraw);

	glClear(GL_COLOR_BUFFER_BIT);

	lightsRendered += drawPointLights(Scene::Instance()->pointLightComponents, Scene::Instance()->renderList, CameraManager::Instance()->ViewProjection, lightAttachmentsToDraw, countOfAttachments, lightFrameBuffer->handle, geometryBuffer->textures);
	lightsRendered += drawSpotLights(Scene::Instance()->spotLightComponents, Scene::Instance()->renderList, CameraManager::Instance()->ViewProjection, lightAttachmentsToDraw, countOfAttachments, lightFrameBuffer->handle, geometryBuffer->textures);
	lightsRendered += drawDirectionalLights(Scene::Instance()->directionalLightComponents, Scene::Instance()->renderList, lightAttachmentsToDraw, countOfAttachments, lightFrameBuffer->handle, geometryBuffer->textures);	

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	return lightsRendered;
}

void
Render::drawSingle(const Object * object, const mwm::Matrix4 & ViewProjection, const GLuint currentShaderID)
{
	Matrix4F ModelMatrix = object->node.TopDownTransform.toFloat();
	Matrix4F MVP = (object->node.TopDownTransform*ViewProjection).toFloat();

	GLuint MatrixHandle = glGetUniformLocation(currentShaderID, "MVP");
	GLuint ModelMatrixHandle = glGetUniformLocation(currentShaderID, "M");

	GLuint MaterialPropertiesHandle = glGetUniformLocation(currentShaderID, "MaterialProperties");
	GLuint MaterialColorHandle = glGetUniformLocation(currentShaderID, "MaterialColor");
	GLuint PickingObjectIndexHandle = glGetUniformLocation(currentShaderID, "objectID");
	GLuint tiling = glGetUniformLocation(currentShaderID, "tiling");

	GLuint TextureSamplerHandle = glGetUniformLocation(currentShaderID, "myTextureSampler");
	glUniform1i(TextureSamplerHandle, 0);

	
	glUniform2f(tiling, object->mat->tileX, object->mat->tileY);
	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(ModelMatrixHandle, 1, GL_FALSE, &ModelMatrix[0][0]);

	Vector4F matProperties = Vector4F(object->mat->metallic, object->mat->diffuseIntensity, object->mat->specularIntensity, object->mat->shininess);
	glUniform4fv(MaterialPropertiesHandle, 1, &matProperties.x);
	glUniform3fv(MaterialColorHandle, 1, &object->mat->color.x);

	glUniform1ui(PickingObjectIndexHandle, object->ID);

	glActiveTexture(GL_TEXTURE0);
	object->mat->texture->Bind();
	

	glBindVertexArray(object->mesh->vaoHandle);

	glDrawElements(GL_TRIANGLES, object->mesh->indicesSize, GL_UNSIGNED_INT, (void*)0);
}

int
Render::drawPicking(std::unordered_map<unsigned int, Object*>& pickingList, const mwm::Matrix4 & ViewProjection, const GLuint currentShaderID)
{
	int objectsRendered = 0;
	GLuint MatrixHandle = glGetUniformLocation(currentShaderID, "MVP");
	GLuint PickingObjectIndexHandle = glGetUniformLocation(currentShaderID, "objectID");
	for (auto& objectPair : pickingList)
	{
		Object* object = objectPair.second;
		if (FrustumManager::Instance()->isBoundingSphereInView(object->node.centeredPosition, object->radius))
		{
			Matrix4F ModelMatrix = object->node.TopDownTransform.toFloat();
			Matrix4F MVP = (object->node.TopDownTransform*ViewProjection).toFloat();

			glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
			glUniform1ui(PickingObjectIndexHandle, object->ID);

			//bind vao before drawing
			glBindVertexArray(object->mesh->vaoHandle);

			// Draw the triangles !
			glDrawElements(GL_TRIANGLES, object->mesh->indicesSize, GL_UNSIGNED_INT, (void*)0); // mode, count, type, element array buffer offset

			objectsRendered++;
		}
	}
	return objectsRendered;
}

int 
Render::drawDepth(const std::vector<Object*>& objects, const Matrix4& ViewProjection, const GLuint currentShaderID)
{
	int objectsRendered = 0;
	GLuint depthMatrixHandle = glGetUniformLocation(currentShaderID, "depthMVP");
	//might want to do frustum culling, must extract planes
	for (auto object : objects)
	{
		Matrix4F MVP = (object->node.TopDownTransform * ViewProjection).toFloat();

		glUniformMatrix4fv(depthMatrixHandle, 1, GL_FALSE, &MVP[0][0]);

		glBindVertexArray(object->mesh->vaoHandle);

		glDrawElements(GL_TRIANGLES, object->mesh->indicesSize, GL_UNSIGNED_INT, (void*)0);
		objectsRendered++;
	}
	return objectsRendered;
}

int
Render::drawCubeDepth(const std::vector<Object*>& objects, const std::vector<mwm::Matrix4>& ViewProjection, const GLuint currentShaderID, const Object* light)
{
	for (unsigned int i = 0; i < 6; ++i)
	{
		GLuint VPMatrixHandle = glGetUniformLocation(currentShaderID, ("shadowMatrices[" + std::to_string(i) + "]").c_str());
		Matrix4F VP = ViewProjection[i].toFloat();
		glUniformMatrix4fv(VPMatrixHandle, 1, GL_FALSE, &VP[0][0]);
	}

	GLuint lightPosForDepth = glGetUniformLocation(currentShaderID, "lightPos");
	Vector3F lightPosDepth = light->GetWorldPosition().toFloat();
	glUniform3fv(lightPosForDepth, 1, &lightPosDepth.x);

	GLuint farPlaneForDepth = glGetUniformLocation(currentShaderID, "far_plane");
	glUniform1f(farPlaneForDepth, (float)light->radius);

	GLuint ModelHandle = glGetUniformLocation(currentShaderID, "model");
	int objectsRendered = 0;

	double radiusDistance = 0.0;
	double centerDistance = 0.0;
	for (auto object : objects)
	{
		radiusDistance = light->radius + object->radius;
		centerDistance = (light->node.centeredPosition - object->node.centeredPosition).vectLengt();
		if (centerDistance < radiusDistance)
		{
			Matrix4F model = object->node.TopDownTransform.toFloat();
			glUniformMatrix4fv(ModelHandle, 1, GL_FALSE, &model[0][0]);

			glBindVertexArray(object->mesh->vaoHandle);
			glDrawElements(GL_TRIANGLES, object->mesh->indicesSize, GL_UNSIGNED_INT, (void*)0);
			objectsRendered++;
		}
	}
	return objectsRendered;
}

void Render::drawSkyboxWithClipPlane(const FrameBuffer * lightFrameBuffer, const GLenum * lightAttachmentsToDraw, const int countOfAttachments, Texture* texture, const mwm::Vector4F& plane, const mwm::Matrix4& ViewMatrix)
{
	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();
	GLuint currentShader = ShaderManager::Instance()->shaderIDs["skyboxWithClipPlane"];
	ShaderManager::Instance()->SetCurrentShader(currentShader);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lightFrameBuffer->handle);
	glDrawBuffers(countOfAttachments, lightAttachmentsToDraw);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDepthMask(GL_FALSE);

	GLuint planeHandle = glGetUniformLocation(currentShader, "plane");
	glUniform4fv(planeHandle, 1, &plane.x);
	Matrix4 View = ViewMatrix;
	View[3][0] = 0;
	View[3][1] = 0;
	View[3][2] = 0;
	Matrix4 ViewProjection = View * currentCamera->ProjectionMatrix;

	box.mat->AssignTexture(texture);
	box.Draw(ViewProjection, currentShader);

	glDepthMask(GL_TRUE);
}

void
Render::drawSkybox(const FrameBuffer * lightFrameBuffer, const GLenum * lightAttachmentsToDraw, const int countOfAttachments, Texture* texture)
{
	//glDepthRange(0.999999, 1.0);
	//glDisable(GL_CULL_FACE);
	GLuint currentShader = ShaderManager::Instance()->shaderIDs["skybox"];
	ShaderManager::Instance()->SetCurrentShader(currentShader);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lightFrameBuffer->handle);
	glDrawBuffers(countOfAttachments, lightAttachmentsToDraw);

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	//glDepthMask(GL_FALSE);
	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();
	Matrix4 View = currentCamera->ViewMatrix;
	View[3][0] = 0;
	View[3][1] = 0;
	View[3][2] = 0;
	Matrix4 ViewProjection = View * currentCamera->ProjectionMatrix;

	box.mat->AssignTexture(texture);
	GLuint MaterialColorHandle = glGetUniformLocation(currentShader, "MaterialColor");
	glUniform3fv(MaterialColorHandle, 1, &box.mat->color.x);
	box.Draw(ViewProjection, currentShader);

	glDepthFunc(GL_LESS);
	glDepthMask(GL_FALSE);
	//glEnable(GL_CULL_FACE);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	//glDepthRange(0.0, 1.0);
	//glDepthMask(GL_TRUE);
}

void Render::drawGSkybox(const FrameBuffer * lightFrameBuffer, const GLenum * lightAttachmentsToDraw, const int countOfAttachments, Texture * texture)
{
	//glDepthRange(0.999999, 1.0);
	//glDisable(GL_CULL_FACE);
	GLuint currentShader = ShaderManager::Instance()->shaderIDs["gskybox"];
	ShaderManager::Instance()->SetCurrentShader(currentShader);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lightFrameBuffer->handle);
	glDrawBuffers(countOfAttachments, lightAttachmentsToDraw);

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	//glDepthMask(GL_FALSE);
	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();
	Matrix4 View = currentCamera->ViewMatrix;
	View[3][0] = 0;
	View[3][1] = 0;
	View[3][2] = 0;
	Matrix4 ViewProjection = View * currentCamera->ProjectionMatrix;

	box.mat->AssignTexture(texture);
	GLuint MaterialColorHandle = glGetUniformLocation(currentShader, "MaterialColor");
	glUniform3fv(MaterialColorHandle, 1, &box.mat->color.x);
	box.Draw(ViewProjection, currentShader);

	glDepthFunc(GL_LESS);
	glDepthMask(GL_FALSE);
	//glEnable(GL_CULL_FACE);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	//glDepthRange(0.0, 1.0);
	//glDepthMask(GL_TRUE);
}

int
Render::drawDirectionalLights(const std::vector<DirectionalLight*>& lights, const std::vector<Object*>& objects, const GLenum* lightBuffers, const int lightBuffersCount, const GLuint fboToDrawTheLightTO, const std::vector<Texture*>& geometryTextures)
{
	glDepthMask(GL_FALSE);
	int lightsRendered = 0;

	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();
	float windowWidth = (float)currentCamera->windowWidth;
	float windowHeight = (float)currentCamera->windowHeight;
	
	Vector3F camPos = currentCamera->GetPosition2().toFloat();

	GLuint lightShaderNoShadows = ShaderManager::Instance()->shaderIDs["directionalLight"];
	GLuint lightShaderWithShadows = ShaderManager::Instance()->shaderIDs["directionalLightShadow"];
	GLuint depthShader = ShaderManager::Instance()->shaderIDs["depth"];
	GLuint blurShader = ShaderManager::Instance()->shaderIDs["fastBlurShadow"];

	glActiveTexture(GL_TEXTURE0);
	geometryTextures[0]->Bind();
	
	glActiveTexture(GL_TEXTURE1);
	geometryTextures[1]->Bind();
	
	glActiveTexture(GL_TEXTURE2);
	geometryTextures[2]->Bind();
	
	glActiveTexture(GL_TEXTURE3);
	geometryTextures[3]->Bind();

	glUseProgram(lightShaderNoShadows);

	GLuint screenSize = glGetUniformLocation(lightShaderNoShadows, "screenSize");
	glUniform2f(screenSize, windowWidth, windowHeight);
	GLuint CameraPos = glGetUniformLocation(lightShaderNoShadows, "CameraPos");
	glUniform3fv(CameraPos, 1, &camPos.x);

	if (onceD)
	{
		GLuint positionSampler = glGetUniformLocation(lightShaderNoShadows, "positionSampler");
		glUniform1i(positionSampler, 0);
		GLuint diffuseSampler = glGetUniformLocation(lightShaderNoShadows, "diffuseSampler");
		glUniform1i(diffuseSampler, 1);
		GLuint normalsSampler = glGetUniformLocation(lightShaderNoShadows, "normalsSampler");
		glUniform1i(normalsSampler, 2);
		GLuint metDiffIntShinSampler = glGetUniformLocation(lightShaderNoShadows, "metDiffIntShinSpecIntSampler");
		glUniform1i(metDiffIntShinSampler, 3);
	}

	glUseProgram(lightShaderWithShadows);

	GLuint shadowFadeSize = glGetUniformLocation(lightShaderWithShadows, "transitionDistance");
	GLuint shadowDistance = glGetUniformLocation(lightShaderWithShadows, "shadowDistance");
	GLuint DepthBiasMatrixHandle = glGetUniformLocation(lightShaderWithShadows, "DepthBiasMVP");

	screenSize = glGetUniformLocation(lightShaderWithShadows, "screenSize");
	glUniform2f(screenSize, windowWidth, windowHeight);
	CameraPos = glGetUniformLocation(lightShaderWithShadows, "CameraPos");
	glUniform3fv(CameraPos, 1, &camPos.x);

	if (onceD)
	{
		GLuint positionSampler = glGetUniformLocation(lightShaderWithShadows, "positionSampler");
		glUniform1i(positionSampler, 0);
		GLuint diffuseSampler = glGetUniformLocation(lightShaderWithShadows, "diffuseSampler");
		glUniform1i(diffuseSampler, 1);
		GLuint normalsSampler = glGetUniformLocation(lightShaderWithShadows, "normalsSampler");
		glUniform1i(normalsSampler, 2);
		GLuint metDiffIntShinSampler = glGetUniformLocation(lightShaderWithShadows, "metDiffIntShinSpecIntSampler");
		glUniform1i(metDiffIntShinSampler, 3);
		GLuint ShadowMapHandle = glGetUniformLocation(lightShaderWithShadows, "shadowMapSampler");
		glUniform1i(ShadowMapHandle, 4);
		onceD = false;
	}


	GLuint lightShader = lightShaderNoShadows;
	for (auto& directionalLight : lights)
	{
		if (directionalLight->hasShadowMap)
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dirShadowMapBuffer->handle);

			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glDepthMask(GL_TRUE);
			glClearColor(1,1,0,1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glCullFace(GL_FRONT);
			glUseProgram(depthShader);
			glViewport(0, 0, dirShadowMapTexture->width, dirShadowMapTexture->height);
			drawDepth(objects, directionalLight->LightMatrixVP, depthShader);
			glViewport(0, 0, windowWidth, windowHeight);
			glCullFace(GL_BACK);
			glDepthMask(GL_FALSE);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);			
			glClearColor(0, 0, 0, 1);
			if (directionalLight->blurShadowMap)
			{
				Texture* blurredShadowMap = nullptr;
				if(directionalLight->oneSizeBlur)
					blurredShadowMap = BlurTextureAtSameSize(dirShadowMapTexture, pingPongBuffers[0], pingPongBuffers[1], directionalLight->activeBlurLevel, directionalLight->blurIntensity, blurShader, windowWidth, windowHeight);
				else
					blurredShadowMap = BlurTexture(dirShadowMapTexture, multiBlurBufferStart, multiBlurBufferTarget, directionalLight->activeBlurLevel, directionalLight->blurIntensity, blurShader, windowWidth, windowHeight);

				glActiveTexture(GL_TEXTURE4);
				blurredShadowMap->Bind();
			}
			else
			{
				glActiveTexture(GL_TEXTURE4);
				dirShadowMapTexture->Bind();
			}

			lightShader = lightShaderWithShadows;
			glUseProgram(lightShader);
			
			glUniform1f(shadowFadeSize, directionalLight->shadowFadeRange);
			glUniform1f(shadowDistance, directionalLight->radius);
			glUniformMatrix4fv(DepthBiasMatrixHandle, 1, GL_FALSE, &directionalLight->BiasedLightMatrixVP[0][0]);
		}
		else
		{
			lightShader = lightShaderNoShadows;
			glUseProgram(lightShader);
		}

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboToDrawTheLightTO);

		glDrawBuffers(lightBuffersCount, lightBuffers);

		glDisable(GL_DEPTH_TEST);

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		GLuint LightDir = glGetUniformLocation(lightShader, "LightInvDirection_worldspace");
		glUniform3fv(LightDir, 1, &directionalLight->LightInvDir.x);

		GLuint LightPower = glGetUniformLocation(lightShader, "lightPower");
		glUniform1f(LightPower, directionalLight->object->mat->diffuseIntensity);

		GLuint LightColor = glGetUniformLocation(lightShader, "lightColor");
		glUniform3fv(LightColor, 1, &directionalLight->object->mat->color.x);

		GLuint Ambient = glGetUniformLocation(lightShader, "ambient");
		glUniform1f(Ambient, directionalLight->object->mat->specularIntensity);

		glBindVertexArray(plane.mesh->vaoHandle);

		glDrawElements(GL_TRIANGLES, plane.mesh->indicesSize, GL_UNSIGNED_SHORT, (void*)0);

		glDisable(GL_BLEND);

		lightsRendered++;
	}	

	return lightsRendered;
}

int
Render::drawPointLights(const std::vector<PointLight*>& lights, const std::vector<Object*>& objects, const mwm::Matrix4& ViewProjection, const GLenum* lightBuffers, const int lightBuffersCount, const GLuint fboToDrawTheLightTO, const std::vector<Texture*>& geometryTextures)
{
	glDepthMask(GL_FALSE);
	int lightsRendered = 0;

	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();
	float windowWidth = (float)currentCamera->windowWidth;
	float windowHeight = (float)currentCamera->windowHeight;
	Vector3F camPos = currentCamera->GetPosition2().toFloat();
	
	GLuint lightShaderNoShadows = ShaderManager::Instance()->shaderIDs["pointLight"];
	GLuint lightShaderWithShadows = ShaderManager::Instance()->shaderIDs["pointLightShadow"];
	GLuint depthShader = ShaderManager::Instance()->shaderIDs["depthCube"];
	GLuint blurShader = ShaderManager::Instance()->shaderIDs["fastBlurShadow"];
	GLuint stencilShader = ShaderManager::Instance()->shaderIDs["stencil"];

	glActiveTexture(GL_TEXTURE0);
	geometryTextures[0]->Bind();

	glActiveTexture(GL_TEXTURE1);
	geometryTextures[1]->Bind();

	glActiveTexture(GL_TEXTURE2);
	geometryTextures[2]->Bind();

	glActiveTexture(GL_TEXTURE3);
	geometryTextures[3]->Bind();

	glUseProgram(lightShaderNoShadows);

	GLuint screenSize = glGetUniformLocation(lightShaderNoShadows, "screenSize");
	glUniform2f(screenSize, windowWidth, windowHeight);
	GLuint CameraPos = glGetUniformLocation(lightShaderNoShadows, "CameraPos");
	glUniform3fv(CameraPos, 1, &camPos.x);

	if (onceP)
	{
		GLuint positionSampler = glGetUniformLocation(lightShaderNoShadows, "positionSampler");
		glUniform1i(positionSampler, 0);
		GLuint diffuseSampler = glGetUniformLocation(lightShaderNoShadows, "diffuseSampler");
		glUniform1i(diffuseSampler, 1);
		GLuint normalsSampler = glGetUniformLocation(lightShaderNoShadows, "normalsSampler");
		glUniform1i(normalsSampler, 2);
		GLuint metDiffIntShinSampler = glGetUniformLocation(lightShaderNoShadows, "metDiffIntShinSpecIntSampler");
		glUniform1i(metDiffIntShinSampler, 3);
	}
	
	glUseProgram(lightShaderWithShadows);

	screenSize = glGetUniformLocation(lightShaderWithShadows, "screenSize");
	glUniform2f(screenSize, windowWidth, windowHeight);
	CameraPos = glGetUniformLocation(lightShaderWithShadows, "CameraPos");
	glUniform3fv(CameraPos, 1, &camPos.x);

	if (onceP)
	{
		GLuint positionSampler = glGetUniformLocation(lightShaderWithShadows, "positionSampler");
		glUniform1i(positionSampler, 0);
		GLuint diffuseSampler = glGetUniformLocation(lightShaderWithShadows, "diffuseSampler");
		glUniform1i(diffuseSampler, 1);
		GLuint normalsSampler = glGetUniformLocation(lightShaderWithShadows, "normalsSampler");
		glUniform1i(normalsSampler, 2);
		GLuint metDiffIntShinSampler = glGetUniformLocation(lightShaderWithShadows, "metDiffIntShinSpecIntSampler");
		glUniform1i(metDiffIntShinSampler, 3);
		GLuint ShadowMapHandle = glGetUniformLocation(lightShaderWithShadows, "shadowMapSampler");
		glUniform1i(ShadowMapHandle, 4);
		onceP = false;
	}

	GLuint lightShader = lightShaderNoShadows;

	glEnable(GL_STENCIL_TEST);
	for (auto& light : lights)
	{
		if (FrustumManager::Instance()->isBoundingSphereInView(light->object->node.centeredPosition, light->object->radius))
		{
			if (light->shadowMapBuffer != nullptr)
			{

				lightShader = lightShaderWithShadows;
				
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, light->shadowMapBuffer->handle);

				glDepthMask(GL_TRUE);
				glClearColor(1, 1, 0, 1);
				glClear(GL_DEPTH_BUFFER_BIT);
				glEnable(GL_DEPTH_TEST);
				glCullFace(GL_FRONT);
				glUseProgram(depthShader);

				glViewport(0, 0, light->shadowMapTexture->width, light->shadowMapTexture->height);
				drawCubeDepth(objects, light->LightMatrixesVP, depthShader, light->object);
				glViewport(0, 0, windowWidth, windowHeight);
				glCullFace(GL_BACK);
				glDepthMask(GL_FALSE);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glClearColor(0, 0, 0, 1);

				//glUseProgram(lightShader);
				glActiveTexture(GL_TEXTURE4);
				light->shadowMapTexture->Bind();
			}
			
			else
			{
				lightShader = lightShaderNoShadows;
			}

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboToDrawTheLightTO);
			
			//-----------phase 1 stencil-----------
			//enable stencil shader 
			glUseProgram(stencilShader);

			glDrawBuffer(GL_NONE);

			glEnable(GL_DEPTH_TEST);

			glDisable(GL_CULL_FACE);

			glClear(GL_STENCIL_BUFFER_BIT);

			// We need the stencil test to be enabled but we want it
			// to succeed always. Only the depth test matters.
			glStencilFunc(GL_ALWAYS, 0, 0);

			glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
			glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

			//draw
			Matrix4F MVP = (light->object->node.TopDownTransform*ViewProjection).toFloat();

			GLuint MatrixHandle = glGetUniformLocation(stencilShader, "MVP");
			glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);

			glBindVertexArray(light->object->mesh->vaoHandle);
			glDrawElements(GL_TRIANGLES, light->object->mesh->indicesSize, GL_UNSIGNED_INT, (void*)0);

			//-----------phase 2 light-----------
			//enable light shader
			glUseProgram(lightShader);
			
			//enable drawing of final color in light buffer 
			glDrawBuffers(lightBuffersCount, lightBuffers);

			glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

			glDisable(GL_DEPTH_TEST);

			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);

			MatrixHandle = glGetUniformLocation(lightShader, "MVP");
			glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);

			GLuint LightRadius = glGetUniformLocation(lightShader, "lightRadius");
			glUniform1f(LightRadius, (float)light->object->radius);

			GLuint attenauationConstantp = glGetUniformLocation(lightShader, "pointAttenuation.constant");
			glUniform1f(attenauationConstantp, light->attenuation.Constant);

			GLuint attenauationLinearp = glGetUniformLocation(lightShader, "pointAttenuation.linear");
			glUniform1f(attenauationLinearp, light->attenuation.Linear);

			GLuint attenauationExponentialp = glGetUniformLocation(lightShader, "pointAttenuation.exponential");
			glUniform1f(attenauationExponentialp, light->attenuation.Exponential);

			GLuint LightPosHandle = glGetUniformLocation(lightShader, "LightPosition_worldspace");
			Vector3F worldPos = light->object->GetWorldPosition().toFloat();
			glUniform3fv(LightPosHandle, 1, &worldPos.x);

			GLuint LightPower = glGetUniformLocation(lightShader, "lightPower");
			glUniform1f(LightPower, light->object->mat->diffuseIntensity);

			GLuint LightColor = glGetUniformLocation(lightShader, "lightColor");
			glUniform3fv(LightColor, 1, &light->object->mat->color.x);

			GLuint Ambient = glGetUniformLocation(lightShader, "ambient");
			glUniform1f(Ambient, light->object->mat->specularIntensity);

			GLuint farPlaneForDepth = glGetUniformLocation(lightShader, "far_plane");
			glUniform1f(farPlaneForDepth, (float)light->object->radius);

			glDrawElements(GL_TRIANGLES, light->object->mesh->indicesSize, GL_UNSIGNED_INT, (void*)0);

			glCullFace(GL_BACK);
			glDisable(GL_BLEND);
			
			lightsRendered++;
		}
	}
	glDisable(GL_STENCIL_TEST);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return lightsRendered;
}

int
Render::drawSpotLights(const std::vector<SpotLight*>& lights, const std::vector<Object*>& objects, const mwm::Matrix4 & ViewProjection, const GLenum * lightBuffers, const int lightBuffersCount, const GLuint fboToDrawTheLightTO, const std::vector<Texture*>& geometryTextures)
{
	glDepthMask(GL_FALSE);
	int lightsRendered = 0;

	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();
	float windowWidth = (float)currentCamera->windowWidth;
	float windowHeight = (float)currentCamera->windowHeight;
	Vector3F camPos = currentCamera->GetPosition2().toFloat();

	glActiveTexture(GL_TEXTURE0); //we should activate the textures once, should not need any shader bound for that
	geometryTextures[0]->Bind();

	glActiveTexture(GL_TEXTURE1);
	geometryTextures[1]->Bind();

	glActiveTexture(GL_TEXTURE2);
	geometryTextures[2]->Bind();

	glActiveTexture(GL_TEXTURE3);
	geometryTextures[3]->Bind();

	GLuint lightShaderNoShadows = ShaderManager::Instance()->shaderIDs["spotLight"];
	GLuint lightShaderWithShadows = ShaderManager::Instance()->shaderIDs["spotLightShadow"];
	GLuint depthShader = ShaderManager::Instance()->shaderIDs["depth"];
	GLuint blurShader = ShaderManager::Instance()->shaderIDs["fastBlurShadow"];
	GLuint stencilShader = ShaderManager::Instance()->shaderIDs["stencil"];

	glUseProgram(lightShaderNoShadows); //we bind shader so we can set variables on it, not to get locations

	GLuint screenSize = glGetUniformLocation(lightShaderNoShadows, "screenSize");
	glUniform2f(screenSize, windowWidth, windowHeight);
	GLuint CameraPos = glGetUniformLocation(lightShaderNoShadows, "CameraPos");
	glUniform3fv(CameraPos, 1, &camPos.x);

	if (onceS)
	{
		GLuint positionSampler = glGetUniformLocation(lightShaderNoShadows, "positionSampler");
		glUniform1i(positionSampler, 0);
		GLuint diffuseSampler = glGetUniformLocation(lightShaderNoShadows, "diffuseSampler");
		glUniform1i(diffuseSampler, 1);
		GLuint normalsSampler = glGetUniformLocation(lightShaderNoShadows, "normalsSampler");
		glUniform1i(normalsSampler, 2);
		GLuint metDiffIntShinSampler = glGetUniformLocation(lightShaderNoShadows, "metDiffIntShinSpecIntSampler");
		glUniform1i(metDiffIntShinSampler, 3);
	}

	glUseProgram(lightShaderWithShadows); //we bind shader so we can set variables on it, not to get locations

	screenSize = glGetUniformLocation(lightShaderWithShadows, "screenSize");
	glUniform2f(screenSize, windowWidth, windowHeight);
	CameraPos = glGetUniformLocation(lightShaderWithShadows, "CameraPos");
	glUniform3fv(CameraPos, 1, &camPos.x);

	if (onceS)
	{
		GLuint positionSampler = glGetUniformLocation(lightShaderWithShadows, "positionSampler");
		glUniform1i(positionSampler, 0);
		GLuint diffuseSampler = glGetUniformLocation(lightShaderWithShadows, "diffuseSampler");
		glUniform1i(diffuseSampler, 1);
		GLuint normalsSampler = glGetUniformLocation(lightShaderWithShadows, "normalsSampler");
		glUniform1i(normalsSampler, 2);
		GLuint metDiffIntShinSampler = glGetUniformLocation(lightShaderWithShadows, "metDiffIntShinSpecIntSampler");
		glUniform1i(metDiffIntShinSampler, 3);
		GLuint ShadowMapHandle = glGetUniformLocation(lightShaderWithShadows, "shadowMapSampler");
		glUniform1i(ShadowMapHandle, 4);
		onceS = false;
	}

	GLuint lightShader = lightShaderNoShadows;

	glEnable(GL_STENCIL_TEST);
	for (auto& light : lights)
	{
		if (FrustumManager::Instance()->isBoundingSphereInView(light->object->node.centeredPosition, light->radius))
		{
			if (light->CanCastShadow())
			{
				
				lightShader = lightShaderWithShadows;
				
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, light->shadowMapBuffer->handle);
				
				glDrawBuffer(GL_COLOR_ATTACHMENT0);
				glDepthMask(GL_TRUE);
				glClearColor(1, 1, 0, 1);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glEnable(GL_DEPTH_TEST);
				glCullFace(GL_FRONT);
				glUseProgram(depthShader);
				glViewport(0, 0, light->shadowMapTexture->width, light->shadowMapTexture->height);
				drawDepth(objects, light->LightMatrixVP, depthShader);
				glViewport(0, 0, windowWidth, windowHeight);
				glCullFace(GL_BACK);
				glDepthMask(GL_FALSE);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glClearColor(0, 0, 0, 1);

				if (light->CanBlurShadowMap())
				{
					Texture* blurredShadowMap = nullptr;
					if (light->oneSizeBlur)
						blurredShadowMap = BlurTextureAtSameSize(light->shadowMapTexture, light->pingPongBuffers[0], light->pingPongBuffers[1], light->activeBlurLevel, light->blurIntensity, blurShader, windowWidth, windowHeight);
					else
						blurredShadowMap = BlurTexture(light->shadowMapTexture, light->multiBlurBufferStart, light->multiBlurBufferTarget, light->activeBlurLevel, light->blurIntensity, blurShader, windowWidth, windowHeight);

					//glUseProgram(lightShader);
					glActiveTexture(GL_TEXTURE4);
					blurredShadowMap->Bind();
				}
				else
				{
					//glUseProgram(lightShader);
					glActiveTexture(GL_TEXTURE4);
					light->shadowMapTexture->Bind();
				}

				//GLuint shadowFadeSize = glGetUniformLocation(lightShader, "transitionDistance");
				//glUniform1f(shadowFadeSize, light->shadowFadeRange);

				//GLuint shadowDistance = glGetUniformLocation(lightShader, "shadowDistance");
				//glUniform1f(shadowDistance, light->object->node.TopDownTransform.getScale().z); //i think instead of projection size we have to use the radius, maybve that's what it is and what i should send
			}
			else
			{
				lightShader = lightShaderNoShadows;
			}

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboToDrawTheLightTO);

			//-----------phase 1 stencil-----------
			//enable stencil shader 
			glUseProgram(stencilShader);

			glDrawBuffer(GL_NONE);

			glEnable(GL_DEPTH_TEST);

			glDisable(GL_CULL_FACE);

			glClear(GL_STENCIL_BUFFER_BIT);

			// We need the stencil test to be enabled but we want it
			// to succeed always. Only the depth test matters.
			glStencilFunc(GL_ALWAYS, 0, 0);

			glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
			glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

			//draw
			Matrix4F MVP = (light->object->node.TopDownTransform*ViewProjection).toFloat();

			GLuint MatrixHandle = glGetUniformLocation(stencilShader, "MVP");
			glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);

			//bind vao before drawing
			glBindVertexArray(light->object->mesh->vaoHandle);

			// Draw the triangles !
			glDrawElements(GL_TRIANGLES, light->object->mesh->indicesSize, GL_UNSIGNED_INT, (void*)0);

			//-----------phase 2 light-----------
			
			glUseProgram(lightShader);

			glDrawBuffers(lightBuffersCount, lightBuffers);

			glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

			glDisable(GL_DEPTH_TEST);

			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);

			MatrixHandle = glGetUniformLocation(lightShader, "MVP");
			glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);

			GLuint LightRadius = glGetUniformLocation(lightShader, "lightRadius");
			float radiuz = (float)light->object->getScale().z;
			glUniform1f(LightRadius, radiuz);

			GLuint attenauationConstants = glGetUniformLocation(lightShader, "spotAttenuation.constant");
			glUniform1f(attenauationConstants, light->attenuation.Constant);

			GLuint attenauationLinears = glGetUniformLocation(lightShader, "spotAttenuation.linear");
			glUniform1f(attenauationLinears, light->attenuation.Linear);

			GLuint attenauationExponentials = glGetUniformLocation(lightShader, "spotAttenuation.exponential");
			glUniform1f(attenauationExponentials, light->attenuation.Exponential);

			GLuint SpotLightDirection_worldspace = glGetUniformLocation(lightShader, "SpotLightDirection_worldspace");
			glUniform3fv(SpotLightDirection_worldspace, 1, &light->LightInvDir.x);

			GLuint SpotLightOuterCutOff = glGetUniformLocation(lightShader, "SpotLightOuterCutOff");
			glUniform1f(SpotLightOuterCutOff, light->cosOuterCutOff);

			GLuint SpotLightCutOff = glGetUniformLocation(lightShader, "SpotLightInnerCutOff");
			glUniform1f(SpotLightCutOff, light->cosInnerCutOff);

			GLuint LightPosHandle = glGetUniformLocation(lightShader, "LightPosition_worldspace");
			Vector3F worldPos = light->object->GetWorldPosition().toFloat();
			glUniform3fv(LightPosHandle, 1, &worldPos.x);

			GLuint LightPower = glGetUniformLocation(lightShader, "lightPower");
			glUniform1f(LightPower, light->object->mat->diffuseIntensity);

			GLuint LightColor = glGetUniformLocation(lightShader, "lightColor");
			glUniform3fv(LightColor, 1, &light->object->mat->color.x);

			GLuint Ambient = glGetUniformLocation(lightShader, "ambient");
			glUniform1f(Ambient, light->object->mat->specularIntensity);

			GLuint DepthBiasMatrixHandle = glGetUniformLocation(lightShader, "DepthBiasMVP");
			glUniformMatrix4fv(DepthBiasMatrixHandle, 1, GL_FALSE, &light->BiasedLightMatrixVP[0][0]);

			glDrawElements(GL_TRIANGLES, light->object->mesh->indicesSize, GL_UNSIGNED_INT, (void*)0);

			glCullFace(GL_BACK);
			glDisable(GL_BLEND);

			lightsRendered++;
		}
	}
	glDisable(GL_STENCIL_TEST);
	return lightsRendered;
}

void
Render::AddPingPongBuffer(int width, int height)
{
	for (int i = 0; i < 2; i++)
	{
		FrameBuffer* pingPongBuffer = FBOManager::Instance()->GenerateFBO();
		pingPongBuffer->dynamicSize = false;
		Texture* blurTexture = pingPongBuffer->RegisterTexture(new Texture(GL_TEXTURE_2D, 0, GL_RG32F, width, height, GL_RG, GL_FLOAT, NULL, GL_COLOR_ATTACHMENT0));
		blurTexture->SetLinear();
		blurTexture->AddClampingToBorder(Vector4F(1.f, 1.f, 1.f, 1.f));
		pingPongBuffer->GenerateAndAddTextures();
		pingPongBuffer->CheckAndCleanup();
		pingPongBuffers[i] = pingPongBuffer;
	}
}

void
Render::AddMultiBlurBuffer(int width, int height, int levels, double scaleX, double scaleY)
{
	std::vector<FrameBuffer*>* bufferStorage = &multiBlurBufferStart;
	for (int i = 0; i < 2; i++)
	{
		FrameBuffer* multiBlurBuffer = FBOManager::Instance()->GenerateFBO();
		//multiBlurBuffer->dynamicSize = false;
		Texture* blurTexture = multiBlurBuffer->RegisterTexture(new Texture(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, GL_RGB, GL_FLOAT, NULL, GL_COLOR_ATTACHMENT0));
		multiBlurBuffer->AddDefaultTextureParameters();
		multiBlurBuffer->GenerateAndAddTextures();

		multiBlurBuffer->CheckAndCleanup();

		bufferStorage->push_back(multiBlurBuffer);
		FrameBuffer* parentBuffer = multiBlurBuffer;
		for (int j = 1; j < levels; j++)
		{
			FrameBuffer* childBlurBuffer = FBOManager::Instance()->GenerateFBO();
			childBlurBuffer->dynamicSize = false;
			Texture* blurTexture = childBlurBuffer->RegisterTexture(new Texture(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, GL_RGB, GL_FLOAT, NULL, GL_COLOR_ATTACHMENT0));
			childBlurBuffer->scaleXFactor = scaleX;
			childBlurBuffer->scaleYFactor = scaleY;
			childBlurBuffer->AddDefaultTextureParameters();
			childBlurBuffer->GenerateAndAddTextures();

			childBlurBuffer->CheckAndCleanup();
			bufferStorage->push_back(childBlurBuffer);

			parentBuffer->RegisterChildBuffer(childBlurBuffer);
			parentBuffer = childBlurBuffer;
		}
		multiBlurBuffer->UpdateTextures(width, height);
		bufferStorage = &multiBlurBufferTarget;
	}
}

Texture*
Render::PingPongBlur(Texture* sourceTexture, int outputLevel, float blurSize, GLuint blurShader)
{
	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();
	return BlurTextureAtSameSize(sourceTexture, pingPongBuffers[0], pingPongBuffers[1], outputLevel, blurSize, blurShader, currentCamera->windowWidth, currentCamera->windowHeight);
}

Texture*
Render::MultiBlur(Texture* sourceTexture, int outputLevel, float blurSize, GLuint blurShader)
{
	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();
	return BlurTexture(sourceTexture, multiBlurBufferStart, multiBlurBufferTarget, outputLevel, blurSize, blurShader, currentCamera->windowWidth, currentCamera->windowHeight);
}

FrameBuffer*
Render::AddDirectionalShadowMapBuffer(int width, int height)
{
	dirShadowMapBuffer = FBOManager::Instance()->GenerateFBO();
	dirShadowMapBuffer->dynamicSize = false;
	dirShadowMapTexture = dirShadowMapBuffer->RegisterTexture(new Texture(GL_TEXTURE_2D, 0, GL_RG32F, width, height, GL_RG, GL_FLOAT, NULL, GL_COLOR_ATTACHMENT0));
	dirShadowMapTexture->SetLinear();
	dirShadowMapTexture->AddClampingToBorder(mwm::Vector4F(1.f, 1.f, 1.f, 1.f));
	Texture* shadowDepthTexture = dirShadowMapBuffer->RegisterTexture(new Texture(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, NULL, GL_DEPTH_ATTACHMENT));
	shadowDepthTexture->AddDefaultTextureParameters();
	dirShadowMapBuffer->GenerateAndAddTextures();
	dirShadowMapBuffer->CheckAndCleanup();
	return dirShadowMapBuffer;
}

void
Render::Blur(Texture* sourceTexture, GLuint destinationFbo, float offsetxVal, float offsetyVal, GLuint offset)
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destinationFbo);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUniform2f(offset, offsetxVal, offsetyVal);

	sourceTexture->Bind();

	glDrawElements(GL_TRIANGLES, plane.mesh->indicesSize, GL_UNSIGNED_SHORT, (void*)0);
}

Texture*
Render::BlurTexture(Texture* sourceTexture, std::vector<FrameBuffer*> startFrameBuffer, std::vector<FrameBuffer*> targetFrameBuffer, int outputLevel, float blurSize, GLuint shader, int windowWidth, int windowHeight)
{
	glUseProgram(shader);

	GLuint offset = glGetUniformLocation(shader, "offset");
	GLuint blurMapSampler = glGetUniformLocation(shader, "blurMapSampler");
	glUniform1i(blurMapSampler, 5); //we tell the sampler to use the texture stored in texture bank GL_TEXTURE5, should be done only once

	glActiveTexture(GL_TEXTURE5); //we activate texture bank 5, next time we call bind on texture it will get attached to the active texture bank

	glBindVertexArray(plane.mesh->vaoHandle);

	int textureWidth = 0;
	int textureHeight = 0;
	Texture* HorizontalSourceTexture = sourceTexture;

	for (size_t i = 0; i < outputLevel + 1; i++)
	{
		textureWidth = startFrameBuffer[i]->textures[0]->width;
		textureHeight = startFrameBuffer[i]->textures[0]->height;

		glViewport(0, 0, textureWidth, textureHeight);

		Blur(HorizontalSourceTexture, startFrameBuffer[i]->handle, blurSize / ((float)textureWidth), 0.f, offset); //horizontally
		Blur(startFrameBuffer[i]->textures[0], targetFrameBuffer[i]->handle, 0.f, blurSize / ((float)textureHeight), offset); //vertically
		HorizontalSourceTexture = targetFrameBuffer[i]->textures[0];
	}

	glBindTexture(GL_TEXTURE_2D, 0); //we unbind the texture from the active texture bank (GL_TEXTURE5)

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glViewport(0, 0, windowWidth, windowHeight);

	return targetFrameBuffer[outputLevel]->textures[0];
}

Texture*
Render::BlurTextureAtSameSize(Texture* sourceTexture, FrameBuffer* startFrameBuffer, FrameBuffer* targetFrameBuffer, int outputLevel, float blurSize, GLuint shader, int windowWidth, int windowHeight)
{
	glUseProgram(shader);

	GLuint offset = glGetUniformLocation(shader, "offset");
	GLuint blurMapSampler = glGetUniformLocation(shader, "blurMapSampler");
	glUniform1i(blurMapSampler, 5); //we tell the sampler to use the texture stored in texture bank GL_TEXTURE5

	glActiveTexture(GL_TEXTURE5); //we activate texture bank 5, next time we call bind on texture it will get attached to the active texture bank

	glBindVertexArray(plane.mesh->vaoHandle);

	int textureWidth = 0;
	int textureHeight = 0;
	Texture* HorizontalSourceTexture = sourceTexture;

	textureWidth = startFrameBuffer->textures[0]->width;
	textureHeight = startFrameBuffer->textures[0]->height;

	glViewport(0, 0, textureWidth, textureHeight);

	float offsetWidth = blurSize / ((float)textureWidth);
	float offsetHeight = blurSize / ((float)textureHeight);

	for (size_t i = 0; i < outputLevel + 1; i++)
	{
		Blur(HorizontalSourceTexture, startFrameBuffer->handle, offsetWidth, 0.f, offset); //horizontally
		Blur(startFrameBuffer->textures[0], targetFrameBuffer->handle, 0.f, offsetHeight, offset); //vertically
		HorizontalSourceTexture = targetFrameBuffer->textures[0];
	}

	glBindTexture(GL_TEXTURE_2D, 0); //we unbind the texture from the active texture bank (GL_TEXTURE5)

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glViewport(0, 0, windowWidth, windowHeight);

	return targetFrameBuffer->textures[0];
}
