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
#include "GraphicsStorage.h"

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

	GLuint geometryShader = GraphicsStorage::shaderIDs["geometry"];
	ShaderManager::Instance()->SetCurrentShader(geometryShader);
	
	GLuint TextureSamplerHandle = glGetUniformLocation(geometryShader, "myTextureSampler");
	glUniform1i(TextureSamplerHandle, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindBuffer(GL_UNIFORM_BUFFER, uboGBVars);

	int objectsRendered = 0;
	for (auto& object : objects)
	{
		if (FrustumManager::Instance()->isBoundingSphereInView(object->node.centeredPosition, object->radius))
		{
			gb.M = object->node.TopDownTransform.toFloat();
			gb.MVP = (object->node.TopDownTransform*CameraManager::Instance()->ViewProjection).toFloat();
			gb.MaterialProperties.x = object->mat->metallic;
			gb.MaterialProperties.y = object->mat->diffuseIntensity;
			gb.MaterialProperties.z = object->mat->specularIntensity;
			gb.MaterialProperties.w = object->mat->shininess;
			gb.MaterialColor.x = object->mat->color.x;
			gb.MaterialColor.y = object->mat->color.y;
			gb.MaterialColor.z = object->mat->color.z;
			gb.objectID = object->ID;
			gb.tiling.x = object->mat->tileX;
			gb.tiling.y = object->mat->tileY;

			glBufferSubData(GL_UNIFORM_BUFFER, 0, 172, &gb);

			object->mat->texture->Bind();

			glBindVertexArray(object->mesh->vaoHandle);

			glDrawElements(GL_TRIANGLES, object->mesh->indicesSize, GL_UNSIGNED_INT, (void*)0);

			objectsRendered++;
		}
	}

	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
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
	glBindBuffer(GL_UNIFORM_BUFFER, uboGBVars);
	Matrix4F ModelMatrix = object->node.TopDownTransform.toFloat();
	Matrix4F MVP = (object->node.TopDownTransform*ViewProjection).toFloat();

	GLuint TextureSamplerHandle = glGetUniformLocation(currentShaderID, "myTextureSampler");
	glUniform1i(TextureSamplerHandle, 0);

	gb.M = object->node.TopDownTransform.toFloat();
	gb.MVP = (object->node.TopDownTransform*CameraManager::Instance()->ViewProjection).toFloat();
	gb.MaterialProperties.x = object->mat->metallic;
	gb.MaterialProperties.y = object->mat->diffuseIntensity;
	gb.MaterialProperties.z = object->mat->specularIntensity;
	gb.MaterialProperties.w = object->mat->shininess;
	gb.MaterialColor.x = object->mat->color.x;
	gb.MaterialColor.y = object->mat->color.y;
	gb.MaterialColor.z = object->mat->color.z;
	gb.objectID = object->ID;
	gb.tiling.x = object->mat->tileX;
	gb.tiling.y = object->mat->tileY;

	glBufferSubData(GL_UNIFORM_BUFFER, 0, 172, &gb);

	glActiveTexture(GL_TEXTURE0);
	object->mat->texture->Bind();

	glBindVertexArray(object->mesh->vaoHandle);

	glDrawElements(GL_TRIANGLES, object->mesh->indicesSize, GL_UNSIGNED_INT, (void*)0);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

int
Render::drawPicking(std::unordered_map<unsigned int, Object*>& pickingList, const mwm::Matrix4 & ViewProjection, const GLuint currentShaderID)
{
	glBindBuffer(GL_UNIFORM_BUFFER, uboGBVars);
	int objectsRendered = 0;
	Object* object = nullptr;
	for (auto& objectPair : pickingList)
	{
		object = objectPair.second;
		if (FrustumManager::Instance()->isBoundingSphereInView(object->node.centeredPosition, object->radius))
		{
			gb.MVP = (object->node.TopDownTransform*CameraManager::Instance()->ViewProjection).toFloat();
			gb.M = object->node.TopDownTransform.toFloat();
			gb.objectID = object->ID;

			glBufferSubData(GL_UNIFORM_BUFFER, 0, 172, &gb);

			//bind vao before drawing
			glBindVertexArray(object->mesh->vaoHandle);

			// Draw the triangles !
			glDrawElements(GL_TRIANGLES, object->mesh->indicesSize, GL_UNSIGNED_INT, (void*)0); // mode, count, type, element array buffer offset

			objectsRendered++;
		}
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
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
	GLuint currentShader = GraphicsStorage::shaderIDs["skyboxWithClipPlane"];
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
	GLuint currentShader = GraphicsStorage::shaderIDs["skybox"];
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
	GLuint currentShader = GraphicsStorage::shaderIDs["gskybox"];
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
	glBindBuffer(GL_UNIFORM_BUFFER, uboLBVars);
	glDepthMask(GL_FALSE);
	int lightsRendered = 0;

	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();

	GLuint lightShaderNoShadows = GraphicsStorage::shaderIDs["directionalLight"];
	GLuint lightShaderWithShadows = GraphicsStorage::shaderIDs["directionalLightShadow"];
	GLuint depthShader = GraphicsStorage::shaderIDs["depth"];
	GLuint blurShader = GraphicsStorage::shaderIDs["fastBlurShadow"];

	glActiveTexture(GL_TEXTURE0);
	geometryTextures[0]->Bind();
	
	glActiveTexture(GL_TEXTURE1);
	geometryTextures[1]->Bind();
	
	glActiveTexture(GL_TEXTURE2);
	geometryTextures[2]->Bind();
	
	glActiveTexture(GL_TEXTURE3);
	geometryTextures[3]->Bind();

	glUseProgram(lightShaderNoShadows);	

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
		if (directionalLight->CanCastShadow() && dirShadowMapBuffer != nullptr)
		{
			lightShader = lightShaderWithShadows;

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
			glViewport(0, 0, currentCamera->windowWidth, currentCamera->windowHeight);
			glCullFace(GL_BACK);
			glDepthMask(GL_FALSE);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glClearColor(0, 0, 0, 1);
			if (directionalLight->CanBlurShadowMap())
			{
				Texture* blurredShadowMap = nullptr;
				switch (directionalLight->blurMode)
				{
				case None:
					break;
				case OneSize:
					if (pingPongBuffers[0] != nullptr)
					blurredShadowMap = BlurTextureAtSameSize(dirShadowMapTexture, pingPongBuffers[0], pingPongBuffers[1], directionalLight->activeBlurLevel, directionalLight->blurIntensity, blurShader, currentCamera->windowWidth, currentCamera->windowHeight);
					glActiveTexture(GL_TEXTURE4);
					blurredShadowMap->Bind();
					break;
				case MultiSize:
					if (multiBlurBufferStart[0] != nullptr)
					blurredShadowMap = BlurTexture(dirShadowMapTexture, multiBlurBufferStart, multiBlurBufferTarget, directionalLight->activeBlurLevel, directionalLight->blurIntensity, blurShader, currentCamera->windowWidth, currentCamera->windowHeight);
					glActiveTexture(GL_TEXTURE4);
					blurredShadowMap->Bind();
					break;
				default:
					break;
				}
			}
			else
			{
				glActiveTexture(GL_TEXTURE4);
				dirShadowMapTexture->Bind();
			}
			
			lb.shadowTransitionSize = directionalLight->shadowFadeRange;
			lb.lightRadius = directionalLight->radius;
			lb.depthBiasMVP = directionalLight->BiasedLightMatrixVP;
		}
		else
		{
			lightShader = lightShaderNoShadows;
		}

		glUseProgram(lightShader);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboToDrawTheLightTO);

		glDrawBuffers(lightBuffersCount, lightBuffers);

		glDisable(GL_DEPTH_TEST);

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		lb.lightInvDir = directionalLight->LightInvDir;
		lb.lightColor = directionalLight->object->mat->color;
		lb.lightPower = directionalLight->object->mat->diffuseIntensity;
		lb.ambient = directionalLight->object->mat->specularIntensity;

		glBufferSubData(GL_UNIFORM_BUFFER, 0, 112, &lb);

		glBindVertexArray(plane.mesh->vaoHandle);

		glDrawElements(GL_TRIANGLES, plane.mesh->indicesSize, GL_UNSIGNED_SHORT, (void*)0);

		glDisable(GL_BLEND);

		lightsRendered++;
	}

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return lightsRendered;
}

int
Render::drawPointLights(const std::vector<PointLight*>& lights, const std::vector<Object*>& objects, const mwm::Matrix4& ViewProjection, const GLenum* lightBuffers, const int lightBuffersCount, const GLuint fboToDrawTheLightTO, const std::vector<Texture*>& geometryTextures)
{
	glBindBuffer(GL_UNIFORM_BUFFER, uboLBVars);

	glDepthMask(GL_FALSE);
	int lightsRendered = 0;

	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();
	
	GLuint lightShaderNoShadows = GraphicsStorage::shaderIDs["pointLight"];
	GLuint lightShaderWithShadows = GraphicsStorage::shaderIDs["pointLightShadow"];
	GLuint depthShader = GraphicsStorage::shaderIDs["depthCube"];
	GLuint blurShader = GraphicsStorage::shaderIDs["fastBlurShadow"];
	GLuint stencilShader = GraphicsStorage::shaderIDs["stencil"];

	glActiveTexture(GL_TEXTURE0);
	geometryTextures[0]->Bind();

	glActiveTexture(GL_TEXTURE1);
	geometryTextures[1]->Bind();

	glActiveTexture(GL_TEXTURE2);
	geometryTextures[2]->Bind();

	glActiveTexture(GL_TEXTURE3);
	geometryTextures[3]->Bind();

	glUseProgram(lightShaderNoShadows);

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
			if (light->CanCastShadow())
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
				glViewport(0, 0, currentCamera->windowWidth, currentCamera->windowHeight);
				glCullFace(GL_BACK);
				glDepthMask(GL_FALSE);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glClearColor(0, 0, 0, 1);

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

			lb.lightRadius = (float)light->object->radius;
			lb.lightPower = light->object->mat->diffuseIntensity;
			lb.ambient = light->object->mat->specularIntensity;
			lb.lightColor = light->object->mat->color;
			lb.MVP = (light->object->node.TopDownTransform*ViewProjection).toFloat();
			lb.lightPosition = light->object->GetWorldPosition().toFloat();
			lb.attenuation = light->attenuation;
			
			glBufferSubData(GL_UNIFORM_BUFFER, 24, 176, &lb.lightRadius);


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

			glDrawElements(GL_TRIANGLES, light->object->mesh->indicesSize, GL_UNSIGNED_INT, (void*)0);

			glCullFace(GL_BACK);
			glDisable(GL_BLEND);
			
			lightsRendered++;
		}
	}
	glDisable(GL_STENCIL_TEST);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return lightsRendered;
}

int
Render::drawSpotLights(const std::vector<SpotLight*>& lights, const std::vector<Object*>& objects, const mwm::Matrix4 & ViewProjection, const GLenum * lightBuffers, const int lightBuffersCount, const GLuint fboToDrawTheLightTO, const std::vector<Texture*>& geometryTextures)
{
	glBindBuffer(GL_UNIFORM_BUFFER, uboLBVars);
	glDepthMask(GL_FALSE);
	int lightsRendered = 0;

	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();

	glActiveTexture(GL_TEXTURE0); //we should activate the textures once, should not need any shader bound for that
	geometryTextures[0]->Bind();

	glActiveTexture(GL_TEXTURE1);
	geometryTextures[1]->Bind();

	glActiveTexture(GL_TEXTURE2);
	geometryTextures[2]->Bind();

	glActiveTexture(GL_TEXTURE3);
	geometryTextures[3]->Bind();

	GLuint lightShaderNoShadows = GraphicsStorage::shaderIDs["spotLight"];
	GLuint lightShaderWithShadows = GraphicsStorage::shaderIDs["spotLightShadow"];
	GLuint depthShader = GraphicsStorage::shaderIDs["depth"];
	GLuint blurShader = GraphicsStorage::shaderIDs["fastBlurShadow"];
	GLuint stencilShader = GraphicsStorage::shaderIDs["stencil"];

	glUseProgram(lightShaderNoShadows); //we bind shader so we can set variables on it, not to get locations

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
				glViewport(0, 0, currentCamera->windowWidth, currentCamera->windowHeight);
				glCullFace(GL_BACK);
				glDepthMask(GL_FALSE);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glClearColor(0, 0, 0, 1);

				if (light->CanBlurShadowMap())
				{
					Texture* blurredShadowMap = nullptr;
					switch (light->blurMode)
					{
					case None:
						break;
					case OneSize:
						blurredShadowMap = BlurTextureAtSameSize(light->shadowMapTexture, light->pingPongBuffers[0], light->pingPongBuffers[1], light->activeBlurLevel, light->blurIntensity, blurShader, currentCamera->windowWidth, currentCamera->windowHeight);
						break;
					case MultiSize:
						blurredShadowMap = BlurTexture(light->shadowMapTexture, light->multiBlurBufferStart, light->multiBlurBufferTarget, light->activeBlurLevel, light->blurIntensity, blurShader, currentCamera->windowWidth, currentCamera->windowHeight);
						break;
					default:
						break;
					}

					glActiveTexture(GL_TEXTURE4);
					blurredShadowMap->Bind();
				}
				else
				{
					glActiveTexture(GL_TEXTURE4);
					light->shadowMapTexture->Bind();
				}

				lb.depthBiasMVP = light->BiasedLightMatrixVP;
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
			
			lb.outerCutOff = light->cosOuterCutOff;
			lb.innerCutOff = light->cosInnerCutOff;
			lb.lightInvDir = light->LightInvDir;
			lb.lightPower = light->object->mat->diffuseIntensity;
			lb.lightColor = light->object->mat->color;
			lb.ambient = light->object->mat->specularIntensity;
			lb.MVP = (light->object->node.TopDownTransform*ViewProjection).toFloat();
			lb.lightPosition = light->object->GetWorldPosition().toFloat();
			lb.lightRadius = (float)light->object->getScale().z;
			lb.attenuation = light->attenuation;

			glBufferSubData(GL_UNIFORM_BUFFER, 0, 200, &lb);

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

			glDrawElements(GL_TRIANGLES, light->object->mesh->indicesSize, GL_UNSIGNED_INT, (void*)0);

			glCullFace(GL_BACK);
			glDisable(GL_BLEND);

			lightsRendered++;
		}
	}
	glDisable(GL_STENCIL_TEST);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
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

void Render::GenerateEBOs()
{
	glGenBuffers(1, &uboGBVars);
	glBindBuffer(GL_UNIFORM_BUFFER, uboGBVars);
	glBufferData(GL_UNIFORM_BUFFER, 172, NULL, GL_STATIC_DRAW); // allocate 172 bytes of memory
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboGBVars); //bind uniform buffer to binding point 0

	glGenBuffers(1, &uboLBVars);
	glBindBuffer(GL_UNIFORM_BUFFER, uboLBVars);
	glBufferData(GL_UNIFORM_BUFFER, 200, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboLBVars); //bind uniform buffer to binding point 1

	glGenBuffers(1, &uboCBVars);
	glBindBuffer(GL_UNIFORM_BUFFER, uboCBVars);
	glBufferData(GL_UNIFORM_BUFFER, 28, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, uboCBVars); //bind uniform buffer to binding point 2
}

void Render::UpdateEBOs()
{
	glBindBuffer(GL_UNIFORM_BUFFER, uboCBVars);
	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();
	cb.width = (float)currentCamera->windowWidth;
	cb.height = (float)currentCamera->windowHeight;
	cb.far = currentCamera->far;
	cb.near = currentCamera->near;
	cb.cameraPos = currentCamera->GetPosition2().toFloat();
	glBufferSubData(GL_UNIFORM_BUFFER, 0, 28, &cb);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
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
