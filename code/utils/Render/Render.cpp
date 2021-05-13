#include "Render.h"
#include "MyMathLib.h"
#include <GL/glew.h>
#include "Object.h"
#include "Material.h"
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "PointLight.h"
#include "Frustum.h"
#include "FBOManager.h"
#include "FrameBuffer.h"
#include "ShaderManager.h"
#include "LightProperties.h"
#include <algorithm>
#include "CameraManager.h"
#include "Camera.h"
#include "SceneGraph.h"
#include "Camera.h"
#include "Texture.h"
#include "GraphicsStorage.h"
#include "InstanceSystem.h"
#include "FastInstanceSystem.h"
#include "Vao.h"
#include "Plane.h"
#include "Box.h"
#include "ShaderBlockData.h"
#include "CPUBlockData.h"


Render::Render()
{
	pb.gamma = 2.2f;
	pb.exposure = 1.0f;
	pb.brightness = 0.0f;
	pb.contrast = 1.0f;
	pb.bloomIntensity = 0.5f;
	pb.hdrEnabled = GL_TRUE;
	pb.bloomEnabled = GL_TRUE;

	captureVPs.resize(6);
	Matrix4F captureProjection = Matrix4F::OpenGLPersp(90, 1.0, 0.1, 10.0);
	captureVPs[0] = Matrix4F::lookAt(Vector3F(), Vector3F(1.0, 0.0, 0.0), Vector3F(0.0, -1.0, 0.0)) * captureProjection; //GL_TEXTURE_CUBE_MAP_POSITIVE_X
	captureVPs[1] = Matrix4F::lookAt(Vector3F(), Vector3F(-1.0, 0.0, 0.0), Vector3F(0.0, -1.0, 0.0)) * captureProjection; //GL_TEXTURE_CUBE_MAP_NEGATIVE_X
	captureVPs[2] = Matrix4F::lookAt(Vector3F(), Vector3F(0.0, 1.0, 0.0), Vector3F(0.0, 0.0, 1.0)) * captureProjection; //GL_TEXTURE_CUBE_MAP_POSITIVE_Y
	captureVPs[3] = Matrix4F::lookAt(Vector3F(), Vector3F(0.0, -1.0, 0.0), Vector3F(0.0, 0.0, -1.0)) * captureProjection; //GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
	captureVPs[4] = Matrix4F::lookAt(Vector3F(), Vector3F(0.0, 0.0, 1.0), Vector3F(0.0, -1.0, 0.0)) * captureProjection; //GL_TEXTURE_CUBE_MAP_POSITIVE_Z
	captureVPs[5] = Matrix4F::lookAt(Vector3F(), Vector3F(0.0, 0.0, -1.0), Vector3F(0.0, -1.0, 0.0)) * captureProjection; //GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
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

void Render::captureToTexture2D(const GLuint shaderID, FrameBuffer * captureFBO, Texture* textureToDrawTo)
{
	ShaderManager::Instance()->SetCurrentShader(shaderID);
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, captureFBO->handle);
	captureFBO->SpecifyTexture(textureToDrawTo);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	
	glViewport(0, 0, textureToDrawTo->width, textureToDrawTo->height);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Plane::Instance()->vao.Bind();
	Plane::Instance()->vao.Draw();

	glViewport(0, 0, CameraManager::Instance()->GetCurrentCamera()->windowWidth, CameraManager::Instance()->GetCurrentCamera()->windowHeight);
}

void Render::captureTextureToCubeMapWithMips(const GLuint shaderID, FrameBuffer * captureFBO, Texture * textureToCapture, Texture * textureToDrawTo)
{
	ShaderManager::Instance()->SetCurrentShader(shaderID);
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, captureFBO->handle);
	unsigned int maxMipLevels = 5;
	
	GLuint roughnessUniform = glGetUniformLocation(shaderID, "roughness");
	GLuint MatrixHandle = glGetUniformLocation(shaderID, "MVPSkybox");

	textureToCapture->ActivateAndBind(0);
	Box::Instance()->vao.Bind();

	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// reisze framebuffer according to mip-level size.
		unsigned int mipWidth = textureToDrawTo->width * std::pow(0.5, mip);
		unsigned int mipHeight = textureToDrawTo->height * std::pow(0.5, mip);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		glUniform1f(roughnessUniform, roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &captureVPs.at(i)[0][0]);
			captureFBO->SpecifyTextureAndMip(textureToDrawTo, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mip);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			Box::Instance()->vao.Draw();
		}
	}
	glViewport(0, 0, CameraManager::Instance()->GetCurrentCamera()->windowWidth, CameraManager::Instance()->GetCurrentCamera()->windowHeight);
}

void Render::captureTextureToCubeMap(const GLuint shaderID, FrameBuffer* captureFBO, Texture* textureToCapture, Texture* textureToDrawTo)
{
	ShaderManager::Instance()->SetCurrentShader(shaderID);
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, captureFBO->handle);
	
	GLuint MatrixHandle = glGetUniformLocation(shaderID, "MVPSkybox");

	textureToCapture->ActivateAndBind(0);
	Box::Instance()->vao.Bind();

	//Quaternion qXangled4 = Quaternion(angleX, Vector3(1.0, 0.0, 0.0));
	//Quaternion qYangled4 = Quaternion(angleY, Vector3(0.0, 1.0, 0.0));
	//Quaternion dirTotalRotation4 = qYangled4 * qXangled4;
	//Matrix4F Model = dirTotalRotation4.ConvertToMatrix().toFloat();
	glViewport(0, 0, textureToDrawTo->width, textureToDrawTo->height);
	for (size_t i = 0; i < 6; ++i)
	{
		//Matrix4F MVP = Model * captureVPs.at(i);
		//glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &captureVPs.at(i)[0][0]);
		captureFBO->SpecifyTextureAndMip(textureToDrawTo, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Box::Instance()->vao.Draw();
	}
	glViewport(0, 0, CameraManager::Instance()->GetCurrentCamera()->windowWidth, CameraManager::Instance()->GetCurrentCamera()->windowHeight);
}

int 
Render::drawGeometry(const GLuint shaderID, const std::vector<Object*>& objects, FrameBuffer * geometryBuffer, const GLenum * attachmentsToDraw, const int countOfAttachments)
{
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, geometryBuffer->handle);
	if (countOfAttachments > 0) glDrawBuffers(countOfAttachments, attachmentsToDraw);
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	ShaderManager::Instance()->SetCurrentShader(shaderID);

	//glBindBuffer(GL_UNIFORM_BUFFER, uboGBVars);

	int objectsRendered = 0;
	for (auto& object : objects)
	{
		//if (FrustumManager::Instance()->isBoundingSphereInView(object->bounds->centeredPosition, object->bounds->circumRadius))
		//{
		//	object->inFrustum = true;
		if (object->inFrustum)
		{
			gsbd->SetData("M", &object->TopDownTransformF, sizeof(Matrix4F));
			gsbd->SetData("MaterialColorShininess", &object->materials[0]->colorShininess, sizeof(Vector4F));
			gsbd->SetData("objectID", &object->ID, sizeof(unsigned int));
			gsbd->SetData("tiling", &object->materials[0]->tile, sizeof(Vector2F));
			//gb.M = object->node->TopDownTransform.toFloat();
			//gb.MVP = (object->node->TopDownTransform*CameraManager::Instance()->ViewProjection).toFloat();
			//gb.MaterialColorShininess = object->materials[0]->colorShininess;
			//gb.objectID = object->ID;
			//gb.tiling.x = object->materials[0]->tileX;
			//gb.tiling.y = object->materials[0]->tileY;
			gsbd->Submit();
			//glBufferSubData(GL_UNIFORM_BUFFER, 0, 156, &gb);

			object->materials[0]->ActivateAndBind();

			object->vao->Bind();

			object->vao->Draw();

			objectsRendered++;
		}
			
		
			
		//}
		//else
		//{
		//	object->inFrustum = false;
		//}
	}
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);



	return objectsRendered;
}

int Render::drawInstancedGeometry(const GLuint shaderID, const std::vector<InstanceSystem*>& iSystems, FrameBuffer * geometryBuffer)
{
	int objectsRendered = 0;
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, geometryBuffer->handle);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	ShaderManager::Instance()->SetCurrentShader(shaderID);

	//Matrix4F VP = CameraManager::Instance()->ViewProjection.toFloat();
	//GLuint VPH = glGetUniformLocation(shaderID, "VP");
	//glUniformMatrix4fv(VPH, 1, GL_FALSE, &VP[0][0]);
	Texture::Activate(0);

	GLuint tiling = glGetUniformLocation(shaderID, "tiling");

	for(auto& system : iSystems)
	{
		glUniform2fv(tiling, 1, &system->mat.tile.x);
		system->mat.ActivateAndBind();
		objectsRendered += system->Draw();
	}
	
	return objectsRendered;
}

int Render::drawFastInstancedGeometry(const GLuint shaderID, const std::vector<FastInstanceSystem*>& iSystems, FrameBuffer * geometryBuffer)
{
	int objectsRendered = 0;
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, geometryBuffer->handle);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	ShaderManager::Instance()->SetCurrentShader(shaderID);

	//Matrix4F VP = CameraManager::Instance()->ViewProjection.toFloat();
	//GLuint VPH = glGetUniformLocation(shaderID, "VP");
	//glUniformMatrix4fv(VPH, 1, GL_FALSE, &VP[0][0]);
	Texture::Activate(0);

	GLuint tiling = glGetUniformLocation(shaderID, "tiling");

	for (auto& system : iSystems)
	{
		glUniform2fv(tiling, 1, &system->mat.tile.x);
		system->mat.ActivateAndBind();
		objectsRendered += system->Draw();
	}

	return objectsRendered;
}

int 
Render::draw(const GLuint shaderID, const std::vector<Object*>& objects, const Matrix4& ViewProjection)
{
	int objectsRendered = 0;

	GLuint MatrixHandle = glGetUniformLocation(shaderID, "MVP");
	GLuint ModelMatrixHandle = glGetUniformLocation(shaderID, "M");
	GLuint MaterialColorShininessHandle = glGetUniformLocation(shaderID, "MaterialColorShininess");
	GLuint PickingObjectIndexHandle = glGetUniformLocation(shaderID, "objectID");
	GLuint tiling = glGetUniformLocation(shaderID, "tiling");

	for (auto& object : objects)
	{
		if (FrustumManager::Instance()->isBoundingSphereInView(object->bounds->centeredPosition, object->bounds->circumRadius))
		{
			object->inFrustum = true;
			Matrix4F ModelMatrix = object->node->TopDownTransform.toFloat();
			Matrix4F MVP = (object->node->TopDownTransform*ViewProjection).toFloat();

			
			glUniform2fv(tiling, 1, &object->materials[0]->tile.x);

			glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixHandle, 1, GL_FALSE, &ModelMatrix[0][0]);

			glUniform4fv(MaterialColorShininessHandle, 1, &object->materials[0]->colorShininess.x);

			glUniform1ui(PickingObjectIndexHandle, object->ID);

			object->materials[0]->ActivateAndBind();
			
			object->vao->Bind();

			object->vao->Draw();
			
			objectsRendered++;
		}
		else
		{
			object->inFrustum = false;
		}
	}
	return objectsRendered;
}

int
Render::drawLight(const GLuint pointLightShader, const GLuint pointLightShadowShader, const GLuint spotLightShader, const GLuint spotLightShadowShader, const GLuint directionalLightShader, const GLuint directionalLightShadowShader, FrameBuffer * lightFrameBuffer, FrameBuffer * geometryBuffer, const GLenum * attachmentsToDraw, const int countOfAttachments)
{
	int lightsRendered = 0;
	
	FBOManager::Instance()->BindFrameBuffer(GL_FRAMEBUFFER, lightFrameBuffer->handle);
	if (countOfAttachments > 0) glDrawBuffers(countOfAttachments, attachmentsToDraw);
	glClear(GL_COLOR_BUFFER_BIT);

	lightsRendered += drawPointLights(pointLightShader, pointLightShadowShader, SceneGraph::Instance()->pointLightComponents, SceneGraph::Instance()->renderList, CameraManager::Instance()->ViewProjection, lightFrameBuffer, geometryBuffer->textures);
	lightsRendered += drawSpotLights(spotLightShader, spotLightShader, SceneGraph::Instance()->spotLightComponents, SceneGraph::Instance()->renderList, CameraManager::Instance()->ViewProjection, lightFrameBuffer, geometryBuffer->textures);
	lightsRendered += drawDirectionalLights(directionalLightShader, directionalLightShadowShader, SceneGraph::Instance()->directionalLightComponents, SceneGraph::Instance()->renderList, lightFrameBuffer, geometryBuffer->textures);

	return lightsRendered;
}

void
Render::drawSingle(const GLuint shaderID, const Object * object, const Matrix4 & ViewProjection, const GLuint currentShaderID)
{
	//glBindBuffer(GL_UNIFORM_BUFFER, uboGBVars);
	Matrix4F ModelMatrix = object->node->TopDownTransform.toFloat();
	Matrix4F MVP = (object->node->TopDownTransform*ViewProjection).toFloat();

	gsbd->SetData("M", &object->TopDownTransformF, sizeof(Matrix4F));
	gsbd->SetData("MaterialColorShininess", &object->materials[0]->colorShininess, sizeof(Vector4F));
	gsbd->SetData("objectID", &object->ID, sizeof(unsigned int));
	gsbd->SetData("tiling", &object->materials[0]->tile, sizeof(Vector2F));

	//gb.M = object->node->TopDownTransform.toFloat();
	//gb.MVP = (object->node->TopDownTransform*CameraManager::Instance()->ViewProjection).toFloat();
	//gb.MaterialColorShininess = object->materials[0]->colorShininess;
	//gb.objectID = object->ID;
	//gb.tiling.x = object->materials[0]->tileX;
	//gb.tiling.y = object->materials[0]->tileY;

	//glBufferSubData(GL_UNIFORM_BUFFER, 0, 156, &gb);

	gsbd->Submit();

	object->materials[0]->ActivateAndBind();

	object->vao->Bind();

	object->vao->Draw();

	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

int
Render::drawPicking(const GLuint shaderID, std::unordered_map<unsigned int, Object*>& pickingList, FrameBuffer* pickingBuffer, const GLenum * attachmentsToDraw, const int countOfAttachments)
{
	//glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	ShaderManager::Instance()->SetCurrentShader(shaderID);

	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, pickingBuffer->handle);
	if (countOfAttachments > 0) glDrawBuffers(countOfAttachments, attachmentsToDraw);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int objectsRendered = 0;
	Object* object = nullptr;
	for (auto& objectPair : pickingList)
	{
		object = objectPair.second;
		if (object->inFrustum)
		{
			gsbd->SetData("M", &object->TopDownTransformF, sizeof(Matrix4F));
			gsbd->SetData("objectID", &object->ID, sizeof(unsigned int));
			gsbd->Submit();

			//bind vao before drawing
			object->vao->Bind();

			// Draw the triangles !
			object->vao->Draw();

			objectsRendered++;
		}
	}
	return objectsRendered;
}

int 
Render::drawDepth(const const GLuint shaderID, const std::vector<Object*>& objects, const Matrix4F& ViewProjection)
{
	ShaderManager::Instance()->SetCurrentShader(shaderID);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	int objectsRendered = 0;
	GLuint depthMatrixHandle = glGetUniformLocation(shaderID, "depthMVP");
	//might want to do frustum culling, must extract planes
	for (auto object : objects)
	{
		Matrix4F MVP = (object->node->TopDownTransform.toFloat() * ViewProjection);

		glUniformMatrix4fv(depthMatrixHandle, 1, GL_FALSE, &MVP[0][0]);

		object->vao->Bind();

		object->vao->Draw();
		objectsRendered++;
	}
	return objectsRendered;
}

int
Render::drawCubeDepth(const GLuint shaderID, const std::vector<Object*>& objects, const std::vector<Matrix4>& ViewProjection, const Object* light)
{
	for (unsigned int i = 0; i < 6; ++i)
	{
		GLuint VPMatrixHandle = glGetUniformLocation(shaderID, ("shadowMatrices[" + std::to_string(i) + "]").c_str());
		Matrix4F VP = ViewProjection[i].toFloat();
		glUniformMatrix4fv(VPMatrixHandle, 1, GL_FALSE, &VP[0][0]);
	}

	GLuint lightPosForDepth = glGetUniformLocation(shaderID, "lightPos");
	Vector3F lightPosDepth = light->node->GetWorldPosition().toFloat();
	glUniform3fv(lightPosForDepth, 1, &lightPosDepth.x);

	GLuint farPlaneForDepth = glGetUniformLocation(shaderID, "far_plane");
	glUniform1f(farPlaneForDepth, (float)light->bounds->radius);

	GLuint ModelHandle = glGetUniformLocation(shaderID, "model");
	int objectsRendered = 0;

	double radiusDistance = 0.0;
	double centerDistance = 0.0;
	for (auto object : objects)
	{
		radiusDistance = light->bounds->radius + object->bounds->radius;
		centerDistance = (light->bounds->centeredPosition - object->bounds->centeredPosition).vectLengt();
		if (centerDistance < radiusDistance)
		{
			Matrix4F model = object->node->TopDownTransform.toFloat();
			glUniformMatrix4fv(ModelHandle, 1, GL_FALSE, &model[0][0]);

			object->vao->Bind();
			object->vao->Draw();
			objectsRendered++;
		}
	}
	return objectsRendered;
}

void Render::drawSkyboxWithClipPlane(const GLuint shaderID, FrameBuffer * lightFrameBuffer, Texture* texture, const Vector4F& plane, const Matrix4& ViewMatrix)
{
	ShaderManager::Instance()->SetCurrentShader(shaderID);

	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, lightFrameBuffer->handle);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDepthMask(GL_FALSE);
	glEnable(GL_CLIP_PLANE0);
	GLuint planeHandle = glGetUniformLocation(shaderID, "plane");
	glUniform4fv(planeHandle, 1, &plane.x);
	Matrix4 View = ViewMatrix;
	View.zeroPosition();
	Matrix4& ViewProjection = View * CameraManager::Instance()->GetCurrentCamera()->ProjectionMatrix;

	Box::Instance()->tex = texture;
	Box::Instance()->Draw(ViewProjection, shaderID);
	glDisable(GL_CLIP_PLANE0);
	glDepthMask(GL_TRUE);
}

void
Render::drawSkybox(const GLuint shaderID, FrameBuffer * lightFrameBuffer, Texture* texture)
{
	//glDepthRange(0.999999, 1.0);
	//glDisable(GL_CULL_FACE);
	ShaderManager::Instance()->SetCurrentShader(shaderID);

	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, lightFrameBuffer->handle);

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	//glDepthMask(GL_FALSE);
	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();
	Matrix4 View = currentCamera->ViewMatrix;
	View[3][0] = 0;
	View[3][1] = 0;
	View[3][2] = 0;

	//Quaternion qXangled4 = Quaternion(angleX, Vector3(1.0, 0.0, 0.0));
	//Quaternion qYangled4 = Quaternion(angleY, Vector3(0.0, 1.0, 0.0));
	//Quaternion dirTotalRotation4 = qYangled4 * qXangled4;

	Matrix4 ViewProjection = View * currentCamera->ProjectionMatrix;
	//ViewProjection = dirTotalRotation4.ConvertToMatrix() * ViewProjection;

	Box::Instance()->tex = texture;
	Box::Instance()->Draw(ViewProjection, shaderID);

	glDepthFunc(GL_LESS);
	//glDepthMask(GL_FALSE);
	//glEnable(GL_CULL_FACE);
	//glDepthRange(0.0, 1.0);
	//glDepthMask(GL_TRUE);
}

void Render::drawGSkybox(const GLuint shaderID, FrameBuffer * lightFrameBuffer, Texture * texture)
{
	ShaderManager::Instance()->SetCurrentShader(shaderID);
	//glBindBuffer(GL_UNIFORM_BUFFER, uboLBVars);
	//glDepthRange(0.999999, 1.0);
	//glDisable(GL_CULL_FACE);

	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, lightFrameBuffer->handle);

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

	Box::Instance()->tex = texture;
	
	//lb.lightColor = Vector3F(1,1,1);
	//lb.lightPower = 10.0f;
	float lightPower = 10.0f;

	lsbd->SetData("lightColor", &Vector3F(1, 1, 1), sizeof(Vector3F));
	lsbd->SetData("lightPower", &lightPower, sizeof(float));

	//glBufferSubData(GL_UNIFORM_BUFFER, 92, 16, &lb.lightPower);

	lsbd->Submit();

	Matrix4F MVP = ViewProjection.toFloat();
	GLuint MatrixHandle = glGetUniformLocation(shaderID, "MVPSkybox");
	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);

	//glDrawElements(GL_TRIANGLES, Box::Instance()->vao.indicesCount, GL_UNSIGNED_SHORT, 0);

	//binds vao, binds and activates texture
	Box::Instance()->Draw(ViewProjection, shaderID);

	glDepthFunc(GL_LESS);
	glDepthMask(GL_FALSE);
	//glEnable(GL_CULL_FACE);
	//glDepthRange(0.0, 1.0);
	//glDepthMask(GL_TRUE);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

int Render::drawAmbientLight(const GLuint shaderID, FrameBuffer * bufferToDrawTheLightTO, const std::vector<Texture*>& geometryTextures, const std::vector<Texture*>& pbrEnvTextures)
{
	ShaderManager::Instance()->SetCurrentShader(shaderID);
	glDepthMask(GL_FALSE);

	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();

	geometryTextures[0]->ActivateAndBind(0); 
	geometryTextures[1]->ActivateAndBind(1);
	geometryTextures[2]->ActivateAndBind(2);
	geometryTextures[3]->ActivateAndBind(3);
	pbrEnvTextures[0]->ActivateAndBind(4);
	pbrEnvTextures[1]->ActivateAndBind(5);
	pbrEnvTextures[2]->ActivateAndBind(6);

	
	
	FBOManager::Instance()->BindFrameBuffer(GL_FRAMEBUFFER, bufferToDrawTheLightTO->handle);

	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	Plane::Instance()->vao.Bind();

	Plane::Instance()->vao.Draw();

	glDisable(GL_BLEND);

	return 1;
}

int
Render::drawDirectionalLights(const GLuint shaderID, const GLuint shadowShaderID, const std::vector<DirectionalLight*>& lights, const std::vector<Object*>& objects, FrameBuffer* fboToDrawTheLightTO, const std::vector<Texture*>& geometryTextures)
{
	//glBindBuffer(GL_UNIFORM_BUFFER, uboLBVars);
	glDepthMask(GL_FALSE);
	int lightsRendered = 0;

	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();

	GLuint lightShaderNoShadows = shaderID;
	GLuint lightShaderWithShadows = shadowShaderID;
	GLuint depthShader = GraphicsStorage::shaderIDs["depth"];
	GLuint blurShader = GraphicsStorage::shaderIDs["fastBlurShadow"];

	geometryTextures[0]->ActivateAndBind(0); //input value same as sampler uniform
	geometryTextures[1]->ActivateAndBind(1);
	geometryTextures[2]->ActivateAndBind(2);
	geometryTextures[3]->ActivateAndBind(3);
	//int uniformBufferUpdateSize = 64;
	//int uniformBufferOffset = 64;
	//void* bufferMemoryStart = &lb.lightInvDir;
	GLuint lightShader = lightShaderNoShadows;
	for (auto& light : lights)
	{
		if (light->CanCastShadow() && dirShadowMapBuffer != nullptr)
		{
			lightShader = lightShaderWithShadows;

			FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, dirShadowMapBuffer->handle);

			glDepthMask(GL_TRUE);
			glClearColor(1, 1, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glCullFace(GL_FRONT);
			ShaderManager::Instance()->SetCurrentShader(depthShader);
			glViewport(0, 0, dirShadowMapTexture->width, dirShadowMapTexture->height);
			drawDepth(depthShader, objects, light->LightMatrixVP);
			glViewport(0, 0, currentCamera->windowWidth, currentCamera->windowHeight);
			glCullFace(GL_BACK);
			glDepthMask(GL_FALSE);
			glClearColor(0, 0, 0, 1);
			if (light->CanBlurShadowMap())
			{
				Texture* blurredShadowMap = nullptr;
				switch (light->blurMode)
				{
				case None:
					break;
				case OneSize:
					if (pingPongBuffers[0] != nullptr)
					blurredShadowMap = BlurTextureAtSameSize(dirShadowMapTexture, pingPongBuffers[0], pingPongBuffers[1], light->activeBlurLevel, light->blurIntensity, blurShader, currentCamera->windowWidth, currentCamera->windowHeight);
					blurredShadowMap->ActivateAndBind(4);
					break;
				case MultiSize:
					if (multiBlurBufferStart[0] != nullptr)
					blurredShadowMap = BlurTexture(dirShadowMapTexture, multiBlurBufferStart, multiBlurBufferTarget, light->activeBlurLevel, light->blurIntensity, blurShader, currentCamera->windowWidth, currentCamera->windowHeight);
					blurredShadowMap->ActivateAndBind(4);
					break;
				default:
					break;
				}
			}
			else
			{
				dirShadowMapTexture->ActivateAndBind(4);
			}
			
			//lb.shadowTransitionSize = light->shadowFadeRange;
			//lb.lightRadius = light->radius;
			//lb.depthBiasMVP = light->BiasedLightMatrixVP;

			lsbd->SetData("shadowTransitionSize", &light->shadowFadeRange, sizeof(float));
			lsbd->SetData("lightRadius", &light->radius, sizeof(float));
			lsbd->SetData("depthBiasMVP", &light->BiasedLightMatrixVP, sizeof(Matrix4F));

			//uniformBufferUpdateSize = 120;
			//uniformBufferOffset = 0;
			//bufferMemoryStart = &lb;
		}
		else
		{
			lightShader = lightShaderNoShadows;
			//uniformBufferUpdateSize = 64;
			//uniformBufferOffset = 64;
			//bufferMemoryStart = &lb.lightInvDir;
		}

		ShaderManager::Instance()->SetCurrentShader(lightShader);

		FBOManager::Instance()->BindFrameBuffer(GL_FRAMEBUFFER, fboToDrawTheLightTO->handle);

		glDisable(GL_DEPTH_TEST);

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		//lb.lightInvDir = light->LightInvDir;
		//lb.lightColor = light->properties.color;
		//lb.lightPower = light->properties.power;
		//lb.ambient = light->properties.ambient;
		//lb.diffuse = light->properties.diffuse;
		//lb.specular = light->properties.specular;

		lsbd->SetData("lightInvDir", &light->LightInvDir, sizeof(Vector3F));
		lsbd->SetData("lightColor", &light->properties.color, sizeof(Vector3F));
		lsbd->SetData("lightPower", &light->properties.power, sizeof(float));
		lsbd->SetData("ambient", &light->properties.ambient, sizeof(float));
		lsbd->SetData("diffuse", &light->properties.diffuse, sizeof(float));
		lsbd->SetData("specular", &light->properties.specular, sizeof(float));

		//glBufferSubData(GL_UNIFORM_BUFFER, uniformBufferOffset, uniformBufferUpdateSize, bufferMemoryStart);

		lsbd->Submit();

		Plane::Instance()->vao.Bind();

		Plane::Instance()->vao.Draw();

		glDisable(GL_BLEND);

		lightsRendered++;
	}

	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return lightsRendered;
}

int
Render::drawPointLights(const GLuint shaderID, const GLuint shadowShaderID, const std::vector<PointLight*>& lights, const std::vector<Object*>& objects, const Matrix4& ViewProjection, FrameBuffer* fboToDrawTheLightTO, const std::vector<Texture*>& geometryTextures)
{
	//glBindBuffer(GL_UNIFORM_BUFFER, uboLBVars);

	glDepthMask(GL_FALSE);
	int lightsRendered = 0;

	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();
	
	GLuint lightShaderNoShadows = shaderID;
	GLuint lightShaderWithShadows = shadowShaderID;
	GLuint depthShader = GraphicsStorage::shaderIDs["depthCube"];
	GLuint blurShader = GraphicsStorage::shaderIDs["fastBlurShadow"];
	GLuint stencilShader = GraphicsStorage::shaderIDs["stencil"];

	geometryTextures[0]->ActivateAndBind(0);
	geometryTextures[1]->ActivateAndBind(1);
	geometryTextures[2]->ActivateAndBind(2);
	geometryTextures[3]->ActivateAndBind(3);

	GLuint lightShader = lightShaderNoShadows;

	glEnable(GL_STENCIL_TEST);
	for (auto& light : lights)
	{
		if (FrustumManager::Instance()->isBoundingSphereInView(light->object->bounds->centeredPosition, light->object->bounds->circumRadius))
		{
			light->object->inFrustum = true;
			if (light->CanCastShadow())
			{

				lightShader = lightShaderWithShadows;
				
				FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, light->shadowMapBuffer->handle);
				
				glDepthMask(GL_TRUE);
				glClearColor(1, 1, 0, 1);
				glClear(GL_DEPTH_BUFFER_BIT);
				glEnable(GL_DEPTH_TEST);
				glCullFace(GL_FRONT);
				ShaderManager::Instance()->SetCurrentShader(depthShader);

				glViewport(0, 0, light->shadowMapTexture->width, light->shadowMapTexture->height);
				drawCubeDepth(depthShader, objects, light->LightMatrixesVP, light->object);
				glViewport(0, 0, currentCamera->windowWidth, currentCamera->windowHeight);
				glCullFace(GL_BACK);
				glDepthMask(GL_FALSE);
				glClearColor(0, 0, 0, 1);

				light->shadowMapTexture->ActivateAndBind(4);
			}
			else
			{
				lightShader = lightShaderNoShadows;
			}
			
			//-----------phase 1 stencil-----------
			//enable stencil shader 
			ShaderManager::Instance()->SetCurrentShader(stencilShader);

			FBOManager::Instance()->BindFrameBuffer(GL_FRAMEBUFFER, fboToDrawTheLightTO->handle);

			glEnable(GL_DEPTH_TEST);

			glDisable(GL_CULL_FACE);

			glClear(GL_STENCIL_BUFFER_BIT);

			// We need the stencil test to be enabled but we want it
			// to succeed always. Only the depth test matters.
			glStencilFunc(GL_ALWAYS, 0, 0);

			glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
			glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

			//lb.lightRadius = (float)light->object->bounds->radius;
			//lb.lightPower = light->properties.power;
			//lb.ambient = light->properties.ambient;
			//lb.diffuse = light->properties.diffuse;
			//lb.specular = light->properties.specular;
			//lb.lightColor = light->properties.color;
			//lb.lightPosition = light->object->node->GetWorldPosition().toFloat();
			//lb.attenuation = light->attenuation;
			//lb.MVP = (light->object->node->TopDownTransform*ViewProjection).toFloat();

			float lightRadius = (float)light->object->bounds->radius;
			lsbd->SetData("lightRadius", &lightRadius, sizeof(float));
			lsbd->SetData("lightPower", &light->properties.power, sizeof(float));
			lsbd->SetData("ambient", &light->properties.ambient, sizeof(float));
			lsbd->SetData("diffuse", &light->properties.diffuse, sizeof(float));
			lsbd->SetData("specular", &light->properties.specular, sizeof(float));
			lsbd->SetData("lightColor", &light->properties.color, sizeof(Vector3F));
			lsbd->SetData("lightPosition", light->object->TopDownTransformF[3], sizeof(Vector3F));
			lsbd->SetData("constant", &light->attenuation.Constant, sizeof(float));
			lsbd->SetData("linear", &light->attenuation.Linear, sizeof(float));
			lsbd->SetData("exponential", &light->attenuation.Exponential, sizeof(float));
			gsbd->SetData("M", &light->object->TopDownTransformF, sizeof(Matrix4F));
			
			lsbd->Submit();
			gsbd->Submit();

			//glBufferSubData(GL_UNIFORM_BUFFER, 88, 128, &lb.lightRadius);



			light->object->vao->Bind();
			light->object->vao->Draw();

			//-----------phase 2 light-----------
			//enable light shader
			ShaderManager::Instance()->SetCurrentShader(lightShader);

			glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

			glDisable(GL_DEPTH_TEST);

			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);

			light->object->vao->Draw();

			glCullFace(GL_BACK);
			glDisable(GL_BLEND);
			
			lightsRendered++;
		}
		else
		{
			light->object->inFrustum = false;
		}
	}
	glDisable(GL_STENCIL_TEST);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return lightsRendered;
}

int
Render::drawSpotLights(const GLuint shaderID, const GLuint shadowShaderID, const std::vector<SpotLight*>& lights, const std::vector<Object*>& objects, const Matrix4 & ViewProjection, FrameBuffer* fboToDrawTheLightTO, const std::vector<Texture*>& geometryTextures)
{
	//glBindBuffer(GL_UNIFORM_BUFFER, uboLBVars);
	glDepthMask(GL_FALSE);
	int lightsRendered = 0;

	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();

	geometryTextures[0]->ActivateAndBind(0);
	geometryTextures[1]->ActivateAndBind(1);
	geometryTextures[2]->ActivateAndBind(2);
	geometryTextures[3]->ActivateAndBind(3);

	GLuint lightShaderNoShadows = shaderID;
	GLuint lightShaderWithShadows = shadowShaderID;
	GLuint depthShader = GraphicsStorage::shaderIDs["depth"];
	GLuint blurShader = GraphicsStorage::shaderIDs["fastBlurShadow"];
	GLuint stencilShader = GraphicsStorage::shaderIDs["stencil"];

	GLuint lightShader = lightShaderNoShadows;

	//int uniformBufferOffset = 64;
	//int uniformBufferUpdateSize = 152;
	//void* bufferMemoryStart = &lb.lightInvDir;
	glEnable(GL_STENCIL_TEST);
	for (auto& light : lights)
	{
		if (FrustumManager::Instance()->isBoundingSphereInView(light->object->bounds->centeredPosition, light->radius))
		{
			light->object->inFrustum = true;
			if (light->CanCastShadow())
			{
				
				lightShader = lightShaderWithShadows;
				
				FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, light->shadowMapBuffer->handle);
				
				glDepthMask(GL_TRUE);
				glClearColor(1, 1, 0, 1);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glEnable(GL_DEPTH_TEST);
				glCullFace(GL_FRONT);
				ShaderManager::Instance()->SetCurrentShader(depthShader);
				glViewport(0, 0, light->shadowMapTexture->width, light->shadowMapTexture->height);
				drawDepth(depthShader, objects, light->LightMatrixVP);
				glViewport(0, 0, currentCamera->windowWidth, currentCamera->windowHeight);
				glCullFace(GL_BACK);
				glDepthMask(GL_FALSE);
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
					blurredShadowMap->ActivateAndBind(4);
				}
				else
				{
					light->shadowMapTexture->ActivateAndBind(4);
				}
				//lb.depthBiasMVP = light->BiasedLightMatrixVP;
				lsbd->SetData("depthBiasMVP", &light->BiasedLightMatrixVP, sizeof(Matrix4F));
				//uniformBufferOffset = 0;
				//uniformBufferUpdateSize = 216;
				//bufferMemoryStart = &lb;
			}
			else
			{
				lightShader = lightShaderNoShadows;
				//uniformBufferOffset = 64;
				//uniformBufferUpdateSize = 152;
				//bufferMemoryStart = &lb.lightInvDir;
			}

			FBOManager::Instance()->BindFrameBuffer(GL_FRAMEBUFFER, fboToDrawTheLightTO->handle);

			//-----------phase 1 stencil-----------
			//enable stencil shader 
			ShaderManager::Instance()->SetCurrentShader(stencilShader);

			glEnable(GL_DEPTH_TEST);

			glDisable(GL_CULL_FACE);

			glClear(GL_STENCIL_BUFFER_BIT);

			// We need the stencil test to be enabled but we want it
			// to succeed always. Only the depth test matters.
			glStencilFunc(GL_ALWAYS, 0, 0);

			glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
			glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
			
			float lightRadius = (float)light->object->bounds->radius;
			lsbd->SetData("outerCutOff", &light->cosOuterCutOff, sizeof(float));
			lsbd->SetData("innerCutOff", &light->cosInnerCutOff, sizeof(float));
			lsbd->SetData("lightInvDir", &light->LightInvDir, sizeof(Vector3F));
			lsbd->SetData("lightPower", &light->properties.power, sizeof(float));
			lsbd->SetData("lightColor", &light->properties.color, sizeof(Vector3F));
			lsbd->SetData("ambient", &light->properties.ambient, sizeof(float));
			lsbd->SetData("diffuse", &light->properties.diffuse, sizeof(float));
			lsbd->SetData("specular", &light->properties.specular, sizeof(float));
			lsbd->SetData("lightPosition", light->object->TopDownTransformF[3], sizeof(Vector3F));
			lsbd->SetData("lightRadius", &lightRadius, sizeof(float));
			lsbd->SetData("constant", &light->attenuation.Constant, sizeof(float));
			lsbd->SetData("linear", &light->attenuation.Linear, sizeof(float));
			lsbd->SetData("exponential", &light->attenuation.Exponential, sizeof(float));
			gsbd->SetData("M", &light->object->TopDownTransformF, sizeof(Matrix4F));

			lsbd->Submit();
			gsbd->Submit();

			//lb.outerCutOff = light->cosOuterCutOff;
			//lb.innerCutOff = light->cosInnerCutOff;
			//lb.lightInvDir = light->LightInvDir;
			//lb.lightPower = light->properties.power;
			//lb.lightColor = light->properties.color;
			//lb.ambient = light->properties.ambient;
			//lb.diffuse = light->properties.diffuse;
			//lb.specular = light->properties.specular;
			//lb.lightPosition = light->object->node->GetWorldPosition().toFloat();
			//lb.lightRadius = (float)light->object->node->getScale().z;
			//lb.attenuation = light->attenuation;
			//lb.MVP = (light->object->node->TopDownTransform*ViewProjection).toFloat();

			//glBufferSubData(GL_UNIFORM_BUFFER, uniformBufferOffset, uniformBufferUpdateSize, bufferMemoryStart);

			//bind vao before drawing
			light->object->vao->Bind();

			// Draw the triangles !
			light->object->vao->Draw();

			//-----------phase 2 light-----------
			
			ShaderManager::Instance()->SetCurrentShader(lightShader);

			glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

			glDisable(GL_DEPTH_TEST);

			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);

			light->object->vao->Draw();

			glCullFace(GL_BACK);
			glDisable(GL_BLEND);

			lightsRendered++;
		}
		else
		{
			light->object->inFrustum = false;
		}
	}
	glDisable(GL_STENCIL_TEST);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return lightsRendered;
}

void
Render::drawHDR(const GLuint shaderID, Texture* colorTexture, Texture* bloomTexture)
{
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, 0);

	ShaderManager::Instance()->SetCurrentShader(shaderID);

	colorTexture->ActivateAndBind(0);
	bloomTexture->ActivateAndBind(1);

	psbd->UpdateAndSubmit();
	//glBindBuffer(GL_UNIFORM_BUFFER, uboPBVars); //we bind ubos only to update them they are accessible all the time for shaders even when not bound
	//glBufferSubData(GL_UNIFORM_BUFFER, 0, 28, &pb);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);

	Plane::Instance()->vao.Bind();
	Plane::Instance()->vao.Draw();
}

void Render::drawHDRequirectangular(const GLuint shaderID, Texture * colorTexture)
{
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, 0);
	ShaderManager::Instance()->SetCurrentShader(shaderID);

	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();
	Matrix4 View = currentCamera->ViewMatrix;
	View[3][0] = 0;
	View[3][1] = 0;
	View[3][2] = 0;
	Matrix4 ViewProjection = View * currentCamera->ProjectionMatrix;

	//Matrix4F MVP = ViewProjection.toFloat();
	Matrix4 model = Matrix4();
	model.setIdentity();
	model.setScale(Vector3(20, 20, 20));
	Matrix4F MVP = (model * CameraManager::Instance()->ViewProjection).toFloat();
	GLuint MatrixHandle = glGetUniformLocation(shaderID, "MVPSkybox");
	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);

	//GLuint fastBBShader = GraphicsStorage::shaderIDs["fastBB"];
	//ShaderManager::Instance()->SetCurrentShader(fastBBShader);

	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	//glDepthMask(GL_TRUE);

	colorTexture->ActivateAndBind(0);

	Vao* vao = GraphicsStorage::vaos["unitCube"];
	vao->Bind();
	//Box::Instance()->vao.Bind();
	//glDrawElements(GL_TRIANGLES, Box::Instance()->vao.indicesCount, GL_UNSIGNED_SHORT, 0);
	vao->Draw();
	//Box::Instance()->Draw(ViewProjection, radiance);

	//glDepthFunc(GL_LESS);
	//glDepthMask(GL_FALSE);
}

void Render::drawRegion(const GLuint shaderID, int posX, int posY, int width, int height, const Texture * texture)
{
	//GLuint shaderID = GraphicsStorage::shaderIDs["depthPanel"];
	ShaderManager::Instance()->SetCurrentShader(shaderID);
	glEnable(GL_SCISSOR_TEST);
	glScissor(posX, posY, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
	glViewport(posX, posY, width, height);

	texture->ActivateAndBind(0);
	
	Plane::Instance()->vao.Bind();
	Plane::Instance()->vao.Draw();

	glViewport(0, 0, CameraManager::Instance()->GetCurrentCamera()->windowWidth, CameraManager::Instance()->GetCurrentCamera()->windowHeight);
}

void
Render::AddPingPongBuffer(int width, int height)
{
	for (int i = 0; i < 2; i++)
	{
		FrameBuffer* pingPongBuffer = FBOManager::Instance()->GenerateFBO(false);
		Texture* blurTexture = new Texture(GL_TEXTURE_2D, 0, GL_RG32F, width, height, GL_RG, GL_FLOAT, NULL, GL_COLOR_ATTACHMENT0);
		blurTexture->GenerateBindSpecify();
		blurTexture->SetLinear();
		blurTexture->SetClampingToBorder(Vector4F(1.f, 1.f, 1.f, 1.f));

		pingPongBuffer->RegisterTexture(blurTexture);
		pingPongBuffer->SpecifyTextures();
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
		Texture* blurTexture = new Texture(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, GL_RGB, GL_FLOAT, NULL, GL_COLOR_ATTACHMENT0);
		blurTexture->GenerateBindSpecify();
		blurTexture->SetClampingToEdge();
		blurTexture->SetLinear();

		multiBlurBuffer->RegisterTexture(blurTexture);
		multiBlurBuffer->SpecifyTextures();
		multiBlurBuffer->CheckAndCleanup();

		bufferStorage->push_back(multiBlurBuffer);
		FrameBuffer* parentBuffer = multiBlurBuffer;
		for (int j = 1; j < levels; j++)
		{
			FrameBuffer* childBlurBuffer = FBOManager::Instance()->GenerateFBO(false);
			Texture* blurTexture = new Texture(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, GL_RGB, GL_FLOAT, NULL, GL_COLOR_ATTACHMENT0);
			blurTexture->GenerateBindSpecify();
			blurTexture->SetClampingToEdge();
			blurTexture->SetLinear();

			childBlurBuffer->RegisterTexture(blurTexture);
			childBlurBuffer->scaleXFactor = scaleX;
			childBlurBuffer->scaleYFactor = scaleY;
			childBlurBuffer->SpecifyTextures();
			childBlurBuffer->CheckAndCleanup();

			bufferStorage->push_back(childBlurBuffer);

			parentBuffer->RegisterChildBuffer(childBlurBuffer);
			parentBuffer = childBlurBuffer;
		}
		multiBlurBuffer->UpdateTextures(width, height);
		bufferStorage = &multiBlurBufferTarget;
	}
}

void Render::InitializeShderBlockDatas()
{
	auto ub = GraphicsStorage::GetUniformBuffer(0);
	if (ub != nullptr) gsbd = new ShaderBlockData(ub);
	ub = GraphicsStorage::GetUniformBuffer(1);
	if (ub != nullptr) lsbd = new ShaderBlockData(ub);
	ub = GraphicsStorage::GetUniformBuffer(2);
	if (ub != nullptr)
	{
		csbd = new ShaderBlockData(ub);
		csbd->RegisterPropertyData("VP", &CameraManager::Instance()->ViewProjectionF, sizeof(Matrix4F));
		csbd->RegisterPropertyData("screenSize", &CameraManager::Instance()->screenSize, sizeof(Vector2F));
		csbd->RegisterPropertyData("far", &CameraManager::Instance()->far, sizeof(float));
		csbd->RegisterPropertyData("near", &CameraManager::Instance()->near, sizeof(float));
		csbd->RegisterPropertyData("cameraPos", &CameraManager::Instance()->cameraPos, sizeof(Vector3F));
		csbd->RegisterPropertyData("cameraUp", &CameraManager::Instance()->cameraUp, sizeof(Vector3F));
		csbd->RegisterPropertyData("cameraRight", &CameraManager::Instance()->cameraRight, sizeof(Vector3F));
		csbd->RegisterPropertyData("cameraForward", &CameraManager::Instance()->cameraForward, sizeof(Vector3F));
	}
	ub = GraphicsStorage::GetUniformBuffer(3);
	if (ub != nullptr)
	{
		psbd = new ShaderBlockData(ub);
		psbd->RegisterPropertyData("gamma", &pb.gamma, sizeof(float));
		psbd->RegisterPropertyData("exposure", &pb.exposure, sizeof(float));
		psbd->RegisterPropertyData("brightness", &pb.brightness, sizeof(float));
		psbd->RegisterPropertyData("contrast", &pb.contrast, sizeof(float));
		psbd->RegisterPropertyData("bloomIntensity", &pb.bloomIntensity, sizeof(float));
		psbd->RegisterPropertyData("hdrEnabled", &pb.hdrEnabled, sizeof(bool));
		psbd->RegisterPropertyData("bloomEnabled", &pb.bloomEnabled, sizeof(bool));
	}



	//we need to make it easy to add properties and then submit
	//in this scenario we want to reuse same cpu buffer, change it's values on cpu
	//and upload it to the gpu
	//in what scenario would we ever want unique cpu buffer?
	//data registry holds all property pairings we should update buffer from these pairings
	//so the cpu could be just one
	//
	////int gbsize = sizeof(GBVars);
	//glGenBuffers(1, &uboGBVars);
	//glBindBuffer(GL_UNIFORM_BUFFER, uboGBVars);
	//glBufferData(GL_UNIFORM_BUFFER, sizeof(GBVars), NULL, GL_STATIC_DRAW); // allocate 172 bytes of memory
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
	//glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboGBVars); //bind uniform buffer to binding point 0
	////int lbsize = sizeof(LightVars);
	//glGenBuffers(1, &uboLBVars);
	//glBindBuffer(GL_UNIFORM_BUFFER, uboLBVars);
	//glBufferData(GL_UNIFORM_BUFFER, sizeof(LightVars), NULL, GL_STATIC_DRAW);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
	//glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboLBVars); //bind uniform buffer to binding point 1
	////int cbsize = sizeof(CamVars);
	//glGenBuffers(1, &uboCBVars);
	//glBindBuffer(GL_UNIFORM_BUFFER, uboCBVars);
	//glBufferData(GL_UNIFORM_BUFFER, sizeof(CamVars), NULL, GL_STATIC_DRAW);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
	//glBindBufferBase(GL_UNIFORM_BUFFER, 2, uboCBVars); //bind uniform buffer to binding point 2
	////int pbsize = sizeof(PostHDRBloom);
	//glGenBuffers(1, &uboPBVars);
	//glBindBuffer(GL_UNIFORM_BUFFER, uboPBVars);
	//glBufferData(GL_UNIFORM_BUFFER, sizeof(PostHDRBloom), NULL, GL_STATIC_DRAW);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
	//glBindBufferBase(GL_UNIFORM_BUFFER, 3, uboPBVars); //bind uniform buffer to binding point 3
}

void Render::UpdateShaderBlockDatas()
{
	//glBindBuffer(GL_UNIFORM_BUFFER, uboCBVars);
	//Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();
	//cb.width = (float)currentCamera->windowWidth;
	//cb.height = (float)currentCamera->windowHeight;
	//cb.far = currentCamera->far;
	//cb.near = currentCamera->near;
	//cb.cameraPos = currentCamera->GetPosition2().toFloat();
	//glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CamVars), &cb);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
	if (csbd != nullptr) csbd->UpdateAndSubmit();
	if (psbd != nullptr) psbd->UpdateAndSubmit();
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
	dirShadowMapBuffer = FBOManager::Instance()->GenerateFBO(false);
	dirShadowMapTexture = new Texture(GL_TEXTURE_2D, 0, GL_RG32F, width, height, GL_RG, GL_FLOAT, NULL, GL_COLOR_ATTACHMENT0);
	dirShadowMapTexture->GenerateBindSpecify();
	dirShadowMapTexture->SetLinear();
	dirShadowMapTexture->SetClampingToBorder(Vector4F(1.f, 1.f, 1.f, 1.f));
	dirShadowMapBuffer->RegisterTexture(dirShadowMapTexture);

	Texture* shadowDepthTexture = new Texture(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, NULL, GL_DEPTH_ATTACHMENT);
	shadowDepthTexture->GenerateBindSpecify();
	shadowDepthTexture->SetDefaultParameters();
	dirShadowMapBuffer->RegisterTexture(shadowDepthTexture);

	dirShadowMapBuffer->SpecifyTextures();
	dirShadowMapBuffer->CheckAndCleanup();
	return dirShadowMapBuffer;
}

void
Render::BlurOnOneAxis(Texture* sourceTexture, FrameBuffer* destinationFbo, float offsetxVal, float offsetyVal, GLuint offset)
{
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, destinationFbo->handle);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUniform2f(offset, offsetxVal, offsetyVal);

	sourceTexture->Bind();

	Plane::Instance()->vao.Draw();
}

Texture*
Render::BlurTexture(Texture* sourceTexture, std::vector<FrameBuffer*>& startFrameBuffer, std::vector<FrameBuffer*>& targetFrameBuffer, int outputLevel, float blurSize, GLuint shader, int windowWidth, int windowHeight)
{
	ShaderManager::Instance()->SetCurrentShader(shader);

	GLuint offset = glGetUniformLocation(shader, "offset");

	Texture::Activate(5); //glActiveTexture(GL_TEXTURE5); //we activate texture bank 5, next time we call bind on texture it will get attached to the active texture bank
	
	Plane::Instance()->vao.Bind();

	int textureWidth = 0;
	int textureHeight = 0;
	Texture* HorizontalSourceTexture = sourceTexture;

	for (size_t i = 0; i < outputLevel + 1; i++)
	{
		textureWidth = startFrameBuffer[i]->textures[0]->width;
		textureHeight = startFrameBuffer[i]->textures[0]->height;

		glViewport(0, 0, textureWidth, textureHeight);

		BlurOnOneAxis(HorizontalSourceTexture, startFrameBuffer[i], blurSize / ((float)textureWidth), 0.f, offset); //horizontally
		BlurOnOneAxis(startFrameBuffer[i]->textures[0], targetFrameBuffer[i], 0.f, blurSize / ((float)textureHeight), offset); //vertically
		HorizontalSourceTexture = targetFrameBuffer[i]->textures[0];
	}

	//glBindTexture(GL_TEXTURE_2D, 0); //we unbind the texture from the active texture bank (GL_TEXTURE5)

	glViewport(0, 0, windowWidth, windowHeight);

	return targetFrameBuffer[outputLevel]->textures[0];
}

Texture*
Render::BlurTextureAtSameSize(Texture* sourceTexture, FrameBuffer* startFrameBuffer, FrameBuffer* targetFrameBuffer, int outputLevel, float blurSize, GLuint shader, int windowWidth, int windowHeight)
{
	ShaderManager::Instance()->SetCurrentShader(shader);

	GLuint offset = glGetUniformLocation(shader, "offset");

	Texture::Activate(5); //glActiveTexture(GL_TEXTURE5); //we activate texture bank 5, next time we call bind on texture it will get attached to the active texture bank

	Plane::Instance()->vao.Bind();
	
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
		BlurOnOneAxis(HorizontalSourceTexture, startFrameBuffer, offsetWidth, 0.f, offset); //horizontally
		BlurOnOneAxis(startFrameBuffer->textures[0], targetFrameBuffer, 0.f, offsetHeight, offset); //vertically
		HorizontalSourceTexture = targetFrameBuffer->textures[0];
	}

	//glBindTexture(GL_TEXTURE_2D, 0); //we unbind the texture from the active texture bank (GL_TEXTURE5)

	glViewport(0, 0, windowWidth, windowHeight);

	return targetFrameBuffer->textures[0];
}
