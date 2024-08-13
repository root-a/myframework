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
#include "SceneGraph.h"
#include "Camera.h"
#include "Texture.h"
#include "InstanceSystem.h"
#include "FastInstanceSystem.h"
#include "Vao.h"
#include "Plane.h"
#include "Box.h"
#include "RenderPass.h"
#include "Shader.h"
#include "RenderProfile.h"
#include "TextureProfile.h"
#include "MaterialProfile.h"
#include <unordered_set>
#include <stack>
#include "ObjectProfile.h"
#include "ImGuiWrapper.h"
#include <imgui.h>
#include <chrono>
#include "ParticleSystem.h"
#include "ShaderBlockData.h"
#include "CPUBlockData.h"
#include "Times.h"


Render::Render()
{
	gamma = 2.2f;
	exposure = 1.0f;
	brightness = 0.0f;
	contrast = 1.0f;
	bloomIntensity = 0.5f;
	hdrEnabled = GL_TRUE;
	bloomEnabled = GL_TRUE;
	previousVao = nullptr;
	currentVao = nullptr;
	captureVPs.resize(6);
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
	captureVPs[0] = captureProjection * glm::lookAt(glm::vec3(), glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)); //GL_TEXTURE_CUBE_MAP_POSITIVE_X
	captureVPs[1] = captureProjection * glm::lookAt(glm::vec3(), glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)); //GL_TEXTURE_CUBE_MAP_NEGATIVE_X
	captureVPs[2] = captureProjection * glm::lookAt(glm::vec3(), glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)); //GL_TEXTURE_CUBE_MAP_POSITIVE_Y
	captureVPs[3] = captureProjection * glm::lookAt(glm::vec3(), glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)); //GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
	captureVPs[4] = captureProjection * glm::lookAt(glm::vec3(), glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)); //GL_TEXTURE_CUBE_MAP_POSITIVE_Z
	captureVPs[5] = captureProjection * glm::lookAt(glm::vec3(), glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)); //GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
}

Render::~Render()
{
}

inline void Render::FindLeastDifferentMaterial(std::vector<RenderElement*>& currentMaterial, std::vector<std::vector<Material*>*>& listOfMaterialSequences, int startFrom, int& outDifferencesCount, Material* outLeastDifferentMaterial, int& outLeastDifferentMaterialIndex)
{
	int foundDifferences = INT_MAX;
	Material* foundMaterial = nullptr;
	int foundMaterialIndex = -1;
	for (size_t j = startFrom; j < listOfMaterialSequences.size(); j++)
	{
		auto mat = (*listOfMaterialSequences[j])[0];
		int differences = 0;
		for (size_t k = 0; k < mat->elements.size(); k++)
		{
			if (mat->elements[k] != currentMaterial[k])
			{
				differences++;
			}
		}
		if (differences < foundDifferences)
		{
			foundDifferences = differences;
			foundMaterial = mat;
			foundMaterialIndex = j;
			if (foundDifferences == 1)
			{
				break; //we have found the smallest difference
			}
		}
	}
	outDifferencesCount = foundDifferences;
	outLeastDifferentMaterial = foundMaterial;
	outLeastDifferentMaterialIndex = foundMaterialIndex;
}

inline void Render::UpdateCurrentMaterialAndRenderList(std::vector<RenderElement*>& currentMaterial, std::vector<RenderElement*>& renderList, std::vector<Material*>& materialSequence)
{
	for (auto mat : materialSequence)
	{
		//the if here will push draw of previous material if it deems necessary
		//currentVao = (VertexArray*)mat->elements[(int)MaterialElements::EVao];
		//if (previousVao == nullptr)
		//{
		//	previousVao = currentVao;
		//}
		//else if (previousVao != currentVao || previousVao->instanced == false)
		//{
		//	// we could store previous shader and check if that shader has dynamicvbos, depending on that we would 
		//	// be able to tell if the function is instanced or not
		//	// the safest way is to probably just create another vao
		//	renderList.push_back(&((VertexArray*)previousVao)->draw);
		//	previousVao = currentVao;
		//	totalNrOfDrawCalls++;
		//}
		for (size_t j = 0; j < mat->elements.size(); j++)
		{
			auto foundMatElement = mat->elements[j];
			auto activeElement = currentMaterial[j];
			if (foundMatElement != activeElement)
			{
				currentMaterial[j] = foundMatElement;
				if (foundMatElement != nullptr) renderList.push_back(foundMatElement);
			}
		}

		auto vao = currentMaterial[(int)MaterialElements::EVao];
		if (vao != nullptr)
		{
			renderList.push_back(&((VertexArray*)vao)->draw);
			totalNrOfDrawCalls++;
		}
	}
}

Render*
Render::Instance()
{
	static Render instance;

	return &instance;
}

void Render::GenerateGraph()
{
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end, startGeneration, endGeneration;
	std::chrono::duration<double> elapsed_seconds;

	start = std::chrono::high_resolution_clock::now();
	startGeneration = start;
	//we could say that material is an internal material structure for one object
	//they are created and changed by the other user friendly material interface
	//we should reserve the number of materials per pass by looking at how much it was in last frame and adding a bit 
	uniqueMaterialSequencesPerPass.clear(); //they are not unique unless we use std::set instead of std::vector
	int materialCount = 0;

	for (auto pass : GraphicsStorage::renderingQueue)
	{
		auto vpProperty = ((RenderPass*)pass)->registry.GetProperty("VP");
		if (vpProperty != nullptr)
		{
			((RenderPass*)pass)->frustum.ExtractPlanes(*(glm::mat4*)vpProperty->dataAddress);
		}
		else
		{
			((RenderPass*)pass)->frustum.ExtractPlanes(CameraManager::Instance()->ViewProjection);
		}
	}

	//is it really a good idea to check frustum per pass?
	//we check all objects multiple times depending on the pass they are rendered in
	//it would be easier if I got a list of objects per pass
	//maybe that is what I have to generate?
	//generate a structure like:
	//pass -> material1 -> objects
	//	   -> material2 -> objects
	//pass -> material3 -> objects
	// right now we store pass -> material sequences
	// it's nice when material have all info they need to render
	// but now we want to share them
	// but to do that they need to know about objects
	// they can share object profile
	// they can also share material profile
	// in that moment we don't want the object profile to have data registries
	// if they share object profile but have different material profile
	// we then set them once render all with red and then render them with blue
	// problem is, is there a problem?
	// if two materials have different material profile but same object profile
	// the problem is that one material has 3 objects and other material has 10 objects
	// different transforms
	// so we have to give transforms
	//we can make the object profile shareable by adding a dynamic vector of data registries
	//when object profile will become shareable we will be able to share materials
	//if we can share materials we can also share material sequences
	//we probably have to create material sequence objects or store pointers to sequences (vectors)
	//for instanced objects we will push many data registries this is basically us telling material here are your object transforms
	//on execute we will set and send
	//for single objects we could also push data registry then set and send but! instances are about one material per instance type
	//if two single objects have same material or just have same objectprofile they would push into same op but they can't render at the same time
	//so they would need unique ops
	//the least we can do is have one op, it will have all data registries
	for (auto& object : SceneGraph::Instance()->allObjects)
	{
		for (auto& materialSq : object->materials)
		{
			Material* mat = materialSq[0];
			if (mat->unbound || materialSq[0]->vao == nullptr)
			{
				//we have to figure out how to avoid adding same materials
				for (auto mat : materialSq)
				{
					if (mat->op != nullptr)
					{
						if (mat->op->vbos.size() > 0) mat->op->registries.push_back(&object->registry); //only meant for instanced stuff
						mat->op->SetDataRegistry(&object->registry);
					}
				}
				uniqueMaterialSequencesPerPass[materialSq[0]->rps].push_back(&materialSq);
				materialCount += materialSq.size();
			}
			else
			{
				glm::mat4 meshCenter(1);

				MathUtils::SetPosition(meshCenter, materialSq[0]->vao->center);
				auto centerTransform = meshCenter * object->node->TopDownTransform;
				auto centeredPosition = MathUtils::GetPosition(centerTransform);
				auto halfExtents = (materialSq[0]->vao->dimensions * object->node->totalScale) * 0.5f;

				//auto radius = std::max(std::max(halfExtents.x, halfExtents.y), halfExtents.z); //perfect for sphere, radius around geometry
				auto circumRadius = glm::length(halfExtents);
				//per mesh frustum culling instead of per object
				//currently we can do frustum culling per object but you can have multiple materials and draw same object with multiple shapes
				//we can also create separate objects with one material for each mesh
				//this way we can easily do the frustum check on the bounds
				bool inFrustum = materialSq[0]->rps->frustum.isBoundingSphereInView(centeredPosition, circumRadius);
				//bool inFrustum = materialSq[0]->rps->frustum.isBoundingSphereInView(boundsComp->centeredPosition, boundsComp->circumRadius);
				object->inFrustum = inFrustum;
				if (inFrustum)
				{
					//we have to figure out how to avoid adding same materials
					for (auto mat : materialSq)
					{
						if (mat->op != nullptr)
						{
							if (mat->op->vbos.size() > 0) mat->op->registries.push_back(&object->registry); //only meant for instanced stuff
							mat->op->SetDataRegistry(&object->registry);
						}
					}
					uniqueMaterialSequencesPerPass[materialSq[0]->rps].push_back(&materialSq);
					materialCount += materialSq.size();
				}
			}
		}
	}
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	countingElementsTime = elapsed_seconds.count();

	start = std::chrono::high_resolution_clock::now();

	finalRenderList.clear();
	finalRenderList.reserve(materialCount * 7);
	totalNrOfDrawCalls = 0;

	for (auto& pass : GraphicsStorage::renderingQueue) //render passes are in order
	{
		auto passMaterialSequencesPairIt = uniqueMaterialSequencesPerPass.find(pass);
		if (passMaterialSequencesPairIt != uniqueMaterialSequencesPerPass.end())
		{
			auto& passMaterialSequencesPair = (*passMaterialSequencesPairIt);
			auto& passMaterialSequences = passMaterialSequencesPair.second;
			auto& materialSequence = *passMaterialSequences[0];
			std::vector<RenderElement*> activeElements(7, nullptr); //maybe we can keep it alive between passes so that if we do new pass we can compare stuff if they are different so we even optimize render pass bindings
			//for each sequence order is determined
			//this means we just want to push materials in order to the render list
			//we just don't want to push same elements
			UpdateCurrentMaterialAndRenderList(activeElements, finalRenderList, materialSequence);
			
			passMaterialSequences[0] = passMaterialSequences.back();
			passMaterialSequences.pop_back();
			//for current materialSequence in the pass
			for (size_t i = 0; i < passMaterialSequences.size(); i++)
			{
				int foundDifferences = INT_MAX;
				Material* foundMaterial = nullptr;
				int foundMaterialIndex = -1;
				//find the material with the least differences when compared to current material
				FindLeastDifferentMaterial(activeElements, passMaterialSequences, i, foundDifferences, foundMaterial, foundMaterialIndex);
				std::vector<Material*>& foundMaterialSequence = *passMaterialSequences[foundMaterialIndex];
				if (foundDifferences > 0) //avoid all duplicates, I could avoid it entirely if I used the set instead of vector
				{
					UpdateCurrentMaterialAndRenderList(activeElements, finalRenderList, foundMaterialSequence);
				}
				//if current material was not the found one then we put it in the index of the found one so that when we go to next material in next iteration we still have a chance to compare this material
				passMaterialSequences[foundMaterialIndex] = passMaterialSequences[i];
			}
		}
	}
	//if (currentVao != nullptr) // because last pass could have been without the actual draw, like blit pass, we should really fix this in the UpdateCurrentMaterialAndRenderList function above
	//{
	//	//have to push the last draw of the last material, for now this works
	//	finalRenderList.push_back(&((VertexArray*)currentVao)->draw);
	//	totalNrOfDrawCalls++;
	//}
	

	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	treeGenerationTime = elapsed_seconds.count();
	endGeneration = end;
	elapsed_seconds = endGeneration - startGeneration;
	totalGenerationTime = elapsed_seconds.count();

	if (showRenderList)
	{
		ImGui::Begin("Generated Render List", &showRenderList);
		ImGui::Text("Draw Calls: %d", totalNrOfDrawCalls);
		ImGui::Text("Render List Elements: %d", finalRenderList.size());
		ImGui::Text("Render List Size: %d bytes", finalRenderList.size() * sizeof(RenderElement*));
		for (auto& element : finalRenderList)
		{
			if (dynamic_cast<RenderProfile*>(element) != nullptr)
			{
				ImGui::Text("\tRP: %s", element->name.c_str());
			}
			else if (dynamic_cast<TextureProfile*>(element) != nullptr)
			{
				ImGui::Text("\tTP: %s", element->name.c_str());
			}
			else if (dynamic_cast<MaterialProfile*>(element) != nullptr)
			{
				ImGui::Text("\tMP: %s", element->name.c_str());
			}
			else if (dynamic_cast<VertexArray*>(element) != nullptr)
			{
				ImGui::Text("\tVao: %s", element->name.c_str());
			}
			else if (dynamic_cast<RenderPass*>(element) != nullptr)
			{
				ImGui::Text("RPS: %s", element->name.c_str());
			}
			else if (dynamic_cast<ObjectProfile*>(element) != nullptr)
			{
				ImGui::Text("\tOP: %s", element->name.c_str());
			}
			else if (dynamic_cast<Shader*>(element) != nullptr)
			{
				ImGui::Text("\tSH: %s", element->name.c_str());
			}
			else if (dynamic_cast<VertexArray::DrawElement*>(element) != nullptr)
			{
				ImGui::Text("\t\tDW");
			}
		}
		ImGui::End();
	}
}

void Render::RenderCubemapFacesToTexture(GLuint shaderID, GLuint captureFBO, GLuint cubemapTexture, GLuint textureToDrawTo, const glm::vec2& size) {

	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, captureFBO);
	GLint currentBoundTexture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentBoundTexture);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	int faceWidth = size.x / 3; // Width of one face in the compact layout
	int faceHeight = size.y / 2; // Height of one face in the compact layout

	std::vector<glm::ivec2> faceOffsets = {
		{0, faceHeight},         // Positive X
		{faceWidth, faceHeight}, // Negative X
		{2 * faceWidth, faceHeight}, // Positive Y
		{0, 0},                  // Negative Y
		{faceWidth, 0},         // Positive Z
		{2 * faceWidth, 0}      // Negative Z
	};

	// Attach the 2D preview texture to the framebuffer
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureToDrawTo, 0);
	ShaderManager::Instance()->SetCurrentShader(shaderID);
	Box::Instance()->vao.Bind();

	// Set the cubemap texture uniform
	GLuint CubeMapHandle = glGetUniformLocation(shaderID, "cubemap");
	glBindTextureUnit(0, cubemapTexture);

	GLuint MatrixHandle = glGetUniformLocation(shaderID, "MVP");

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < 6; ++i) {
		glViewport(faceOffsets[i].x, faceOffsets[i].y, faceWidth, faceHeight);
		glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &captureVPs.at(i)[0][0]);
		Box::Instance()->vao.Draw();
	}

	glViewport(0, 0, CameraManager::Instance()->GetCurrentCamera()->windowWidth, CameraManager::Instance()->GetCurrentCamera()->windowHeight);
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindTextureUnit(0, currentBoundTexture);

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

	glViewport(0, 0, textureToDrawTo->width, textureToDrawTo->height);
	for (size_t i = 0; i < 6; ++i)
	{
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

	Vector2F tiling(1, 1);
	Vector4F matColShininess(0, 0, 0, 40);
	int objectsRendered = 0;
	for (auto& object : objects)
	{
		if (object->inFrustum)
		{
			o_gbd->SetData("M", &object->node->TopDownTransformF, sizeof(Matrix4F));
			o_gbd->SetData("objectID", &object->ID, sizeof(unsigned int));
			m_gbd->SetData("tiling", &tiling, sizeof(Vector2F));
			m_gbd->SetData("MaterialColorShininess", &matColShininess, sizeof(Vector4F));

			o_gbd->Submit();
			m_gbd->Submit();

			object->materials[0][0]->tp->ActivateAndBindTextures();

			object->materials[0][0]->vao->Bind();
			object->materials[0][0]->vao->Draw();

			objectsRendered++;
		}
	}
	return objectsRendered;
}

int Render::drawInstancedGeometry(const GLuint shaderID, PoolParty<InstanceSystem, 1000>& iSystems, FrameBuffer * geometryBuffer)
{
	int objectsRendered = 0;
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, geometryBuffer->handle);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	ShaderManager::Instance()->SetCurrentShader(shaderID);

	Texture::Activate(0);

	GLuint tiling = glGetUniformLocation(shaderID, "tiling");
	Vector2F tile(1, 1);
	for (auto& system : iSystems)
	{
		glUniform2fv(tiling, 1, &tile.x);
		system.mat.tp->ActivateAndBindTextures();
		objectsRendered += system.Draw();
	}
	
	return objectsRendered;
}

int Render::drawFastInstancedGeometry(const GLuint shaderID, PoolParty<FastInstanceSystem, 1000>& iSystems, FrameBuffer * geometryBuffer)
{
	int objectsRendered = 0;
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, geometryBuffer->handle);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	ShaderManager::Instance()->SetCurrentShader(shaderID);

	Texture::Activate(0);

	GLuint tiling = glGetUniformLocation(shaderID, "tiling");
	Vector2F tile(1, 1);
	for (auto& system : iSystems)
	{
		glUniform2fv(tiling, 1, &tile.x);
		system.mat.tp->ActivateAndBindTextures();
		objectsRendered += system.Draw();
	}

	return objectsRendered;
}

int 
Render::draw(const GLuint shaderID, const std::vector<Object*>& objects, const glm::mat4& ViewProjection)
{
	int objectsRendered = 0;

	GLuint MatrixHandle = glGetUniformLocation(shaderID, "MVP");
	GLuint ModelMatrixHandle = glGetUniformLocation(shaderID, "M");
	GLuint MaterialColorShininessHandle = glGetUniformLocation(shaderID, "MaterialColorShininess");
	GLuint PickingObjectIndexHandle = glGetUniformLocation(shaderID, "objectID");
	GLuint tiling = glGetUniformLocation(shaderID, "tiling");

	for (auto& object : objects)
	{
		if (SceneGraph::Instance()->frustum.isBoundingSphereInView(object->bounds->centeredPosition, object->bounds->circumRadius))
		{
			object->inFrustum = true;
			glm::mat4 ModelMatrix = object->node->TopDownTransform;
			glm::mat4 MVP = (ViewProjection*object->node->TopDownTransform);

			glm::vec2 tile(1, 1);
			glUniform2fv(tiling, 1, &tile.x);

			glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixHandle, 1, GL_FALSE, &ModelMatrix[0][0]);
			glm::vec4 colorShininess(0,0,0,40);
			glUniform4fv(MaterialColorShininessHandle, 1, &colorShininess.x);

			glUniform1ui(PickingObjectIndexHandle, object->ID);

			object->materials[0][0]->tp->ActivateAndBindTextures();
			
			object->materials[0][0]->vao->Bind();
			object->materials[0][0]->vao->Draw();
			
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

	lightsRendered += drawPointLights(pointLightShader, pointLightShadowShader, *GraphicsStorage::assetRegistry.GetPool<PointLight>(), SceneGraph::Instance()->renderList, CameraManager::Instance()->ViewProjection, lightFrameBuffer, geometryBuffer->textures);
	lightsRendered += drawSpotLights(spotLightShader, spotLightShader, *GraphicsStorage::assetRegistry.GetPool<SpotLight>(), SceneGraph::Instance()->renderList, CameraManager::Instance()->ViewProjection, lightFrameBuffer, geometryBuffer->textures);
	lightsRendered += drawDirectionalLights(directionalLightShader, directionalLightShadowShader, *GraphicsStorage::assetRegistry.GetPool<DirectionalLight>(), SceneGraph::Instance()->renderList, lightFrameBuffer, geometryBuffer->textures);

	return lightsRendered;
}

void
Render::drawSingle(const GLuint shaderID, const Object* object, const glm::mat4 &ViewProjection, const GLuint currentShaderID)
{
	Vector2F tiling(1, 1);
	Vector4F matColShininess(0, 0, 0, 40);
	o_gbd->SetData("M", &object->node->TopDownTransformF, sizeof(Matrix4F));
	o_gbd->SetData("objectID", &object->ID, sizeof(unsigned int));
	m_gbd->SetData("tiling", &tiling, sizeof(Vector2F));
	m_gbd->SetData("MaterialColorShininess", &matColShininess, sizeof(Vector4F));

	o_gbd->Submit();
	m_gbd->Submit();

	object->materials[0][0]->tp->ActivateAndBindTextures();

	object->materials[0][0]->vao->Bind();
	object->materials[0][0]->vao->Draw();
}

int
Render::drawPicking(const GLuint shaderID, std::unordered_map<unsigned int, Object*>& pickingList, FrameBuffer* pickingBuffer, const GLenum * attachmentsToDraw, const int countOfAttachments)
{
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
			o_gbd->SetData("M", &object->node->TopDownTransformF, sizeof(Matrix4F));
			o_gbd->SetData("objectID", &object->ID, sizeof(unsigned int));
			o_gbd->Submit();

			object->materials[0][0]->vao->Bind();
			object->materials[0][0]->vao->Draw();

			objectsRendered++;
		}
	}
	return objectsRendered;
}

int 
Render::drawDepth(const const GLuint shaderID, const std::vector<Object*>& objects, const glm::mat4& ViewProjection)
{
	ShaderManager::Instance()->SetCurrentShader(shaderID);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	int objectsRendered = 0;
	//might want to do frustum culling, must extract planes
	for (auto object : objects)
	{
		m_lvpbd->SetData("lightVP", &ViewProjection, sizeof(Matrix4F));
		o_gbd->SetData("M", &object->node->TopDownTransformF, sizeof(Matrix4F));
		
		m_lvpbd->Submit();
		o_gbd->Submit();

		object->materials[0][0]->vao->Bind();
		object->materials[0][0]->vao->Draw();

		objectsRendered++;
	}
	return objectsRendered;
}

int
Render::drawCubeDepth(const GLuint shaderID, const std::vector<Object*>& objects, const std::vector<glm::mat4>& ViewProjection, const Object* light)
{
	for (unsigned int i = 0; i < 6; ++i)
	{
		GLuint VPMatrixHandle = glGetUniformLocation(shaderID, ("shadowMatrices[" + std::to_string(i) + "]").c_str());
		glm::mat4 VP = ViewProjection[i];
		glUniformMatrix4fv(VPMatrixHandle, 1, GL_FALSE, &VP[0][0]);
	}

	GLuint lightPosForDepth = glGetUniformLocation(shaderID, "lightPos");
	glm::vec3 lightPosDepth = light->node->GetWorldPosition();
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
		centerDistance = glm::length(light->bounds->centeredPosition - object->bounds->centeredPosition);
		if (centerDistance < radiusDistance)
		{
			glUniformMatrix4fv(ModelHandle, 1, GL_FALSE, &object->node->TopDownTransformF[0][0]);

			object->materials[0][0]->vao->Bind();
			object->materials[0][0]->vao->Draw();
			objectsRendered++;
		}
	}
	return objectsRendered;
}

void
Render::drawSkyboxWithClipPlane(const GLuint shaderID, FrameBuffer * lightFrameBuffer, Texture* texture, const glm::vec4& plane, const glm::mat4& ViewMatrix)
{
	ShaderManager::Instance()->SetCurrentShader(shaderID);

	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, lightFrameBuffer->handle);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDepthMask(GL_FALSE);
	glEnable(GL_CLIP_PLANE0);
	GLuint planeHandle = glGetUniformLocation(shaderID, "plane");
	glUniform4fv(planeHandle, 1, &plane.x);
	glm::mat4 View = ViewMatrix;
	MathUtils::ZeroPosition(View);
	glm::mat4 ViewProjection = CameraManager::Instance()->GetCurrentCamera()->ProjectionMatrix * View;

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
	glm::mat4 View = currentCamera->ViewMatrix;
	View[3][0] = 0;
	View[3][1] = 0;
	View[3][2] = 0;

	//Quaternion qXangled4 = Quaternion(angleX, Vector3(1.0, 0.0, 0.0));
	//Quaternion qYangled4 = Quaternion(angleY, Vector3(0.0, 1.0, 0.0));
	//Quaternion dirTotalRotation4 = qYangled4 * qXangled4;

	glm::mat4 ViewProjection = currentCamera->ProjectionMatrix * View;
	//ViewProjection = dirTotalRotation4.convertToMatrix() * ViewProjection;

	Box::Instance()->tex = texture;
	Box::Instance()->Draw(ViewProjection, shaderID);

	glDepthFunc(GL_LESS);
	//glDepthMask(GL_FALSE);
	//glEnable(GL_CULL_FACE);
	//glDepthRange(0.0, 1.0);
	//glDepthMask(GL_TRUE);
}

void
Render::drawGSkybox(const GLuint shaderID, FrameBuffer * lightFrameBuffer, Texture * texture)
{
	ShaderManager::Instance()->SetCurrentShader(shaderID);
	//glDepthRange(0.999999, 1.0);
	//glDisable(GL_CULL_FACE);

	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, lightFrameBuffer->handle);

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	//glDepthMask(GL_FALSE);
	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();
	glm::mat4 View = currentCamera->ViewMatrix;
	View[3][0] = 0;
	View[3][1] = 0;
	View[3][2] = 0;
	glm::mat4 ViewProjection = currentCamera->ProjectionMatrix * View;

	Box::Instance()->tex = texture;
	
	float lightPower = 10.0f;
	glm::vec3 lightColor(1, 1, 1);

	m_lbd->SetData("lightColor", &lightColor, sizeof(Vector3F));
	m_lbd->SetData("lightPower", &lightPower, sizeof(float));

	m_lbd->Submit();

	glm::mat4 MVP = ViewProjection;
	GLuint MatrixHandle = glGetUniformLocation(shaderID, "MVPSkybox");
	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);

	//binds vao, binds and activates texture
	Box::Instance()->Draw(ViewProjection, shaderID);

	glDepthFunc(GL_LESS);
	glDepthMask(GL_FALSE);
	//glEnable(GL_CULL_FACE);
	//glDepthRange(0.0, 1.0);
	//glDepthMask(GL_TRUE);
}

int
Render::drawAmbientLight(const GLuint shaderID, FrameBuffer * bufferToDrawTheLightTO, const std::vector<Texture*>& geometryTextures, const std::vector<Texture*>& pbrEnvTextures)
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
Render::drawDirectionalLights(const GLuint shaderID, const GLuint shadowShaderID, PoolParty<DirectionalLight>& lights, const std::vector<Object*>& objects, FrameBuffer* fboToDrawTheLightTO, const std::vector<Texture*>& geometryTextures)
{
	glDepthMask(GL_FALSE);
	int lightsRendered = 0;

	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();

	GLuint lightShaderNoShadows = shaderID;
	GLuint lightShaderWithShadows = shadowShaderID;
	GLuint depthShader = GraphicsStorage::shaderIDs["Depth"];
	GLuint blurShader = GraphicsStorage::shaderIDs["FastBlurShadow"];

	geometryTextures[0]->ActivateAndBind(0); //input value same as sampler uniform
	geometryTextures[1]->ActivateAndBind(1);
	geometryTextures[2]->ActivateAndBind(2);
	geometryTextures[3]->ActivateAndBind(3);
	GLuint lightShader = lightShaderNoShadows;
	for (auto& light : lights)
	{
		if (light.CanCastShadow() && dirShadowMapBuffer != nullptr)
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
			drawDepth(depthShader, objects, light.LightMatrixVP);
			glViewport(0, 0, currentCamera->windowWidth, currentCamera->windowHeight);
			glCullFace(GL_BACK);
			glDepthMask(GL_FALSE);
			glClearColor(0, 0, 0, 1);
			if (light.CanBlurShadowMap())
			{
				Texture* blurredShadowMap = nullptr;
				switch (light.blurMode)
				{
				case BlurMode::None:
					break;
				case BlurMode::OneSize:
					if (pingPongBuffers[0] != nullptr)
						blurredShadowMap = BlurTextureAtSameSize(dirShadowMapTexture, pingPongBuffers[0], pingPongBuffers[1], light.activeBlurLevel, light.blurIntensity, blurShader, currentCamera->windowWidth, currentCamera->windowHeight);
					blurredShadowMap->ActivateAndBind(4);
					break;
				case BlurMode::MultiSize:
					if (multiBlurBufferStart[0] != nullptr)
						blurredShadowMap = BlurTexture(dirShadowMapTexture, multiBlurBufferStart, multiBlurBufferTarget, light.activeBlurLevel, light.blurIntensity, blurShader, currentCamera->windowWidth, currentCamera->windowHeight);
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

			o_ldsbd->SetData("shadowFadeRange", &light.shadowFadeRange, sizeof(float));
			o_ldsbd->SetData("lightRadius", &light.radius, sizeof(float));
			o_ldsbd->SetData("depthBiasMVP", &light.BiasedLightMatrixVP, sizeof(Matrix4F));
			o_ldsbd->SetData("lightInvDir", &light.LightInvDir, sizeof(Vector3F));
			o_ldsbd->Submit();
		}
		else
		{
			lightShader = lightShaderNoShadows;
			o_ldbd->SetData("lightInvDir", &light.LightInvDir, sizeof(Vector3F));
			o_ldbd->Submit();
		}

		ShaderManager::Instance()->SetCurrentShader(lightShader);

		FBOManager::Instance()->BindFrameBuffer(GL_FRAMEBUFFER, fboToDrawTheLightTO->handle);

		glDisable(GL_DEPTH_TEST);

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);


		m_lbd->SetData("lightColor", &light.properties.color, sizeof(Vector3F));
		m_lbd->SetData("lightPower", &light.properties.power, sizeof(float));
		m_lbd->SetData("ambient", &light.properties.ambient, sizeof(float));
		m_lbd->SetData("diffuse", &light.properties.diffuse, sizeof(float));
		m_lbd->SetData("specular", &light.properties.specular, sizeof(float));

		m_lbd->Submit();

		Plane::Instance()->vao.Bind();
		Plane::Instance()->vao.Draw();

		glDisable(GL_BLEND);

		lightsRendered++;
	}
	return lightsRendered;
}

int
Render::drawPointLights(const GLuint shaderID, const GLuint shadowShaderID, PoolParty<PointLight>& lights, const std::vector<Object*>& objects, const glm::mat4& ViewProjection, FrameBuffer* fboToDrawTheLightTO, const std::vector<Texture*>& geometryTextures)
{
	glDepthMask(GL_FALSE);
	int lightsRendered = 0;

	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();
	
	GLuint lightShaderNoShadows = shaderID;
	GLuint lightShaderWithShadows = shadowShaderID;
	GLuint depthShader = GraphicsStorage::shaderIDs["CubeDepth"];
	GLuint blurShader = GraphicsStorage::shaderIDs["FastBlurShadow"];
	GLuint stencilShader = GraphicsStorage::shaderIDs["Stencil"];

	geometryTextures[0]->ActivateAndBind(0);
	geometryTextures[1]->ActivateAndBind(1);
	geometryTextures[2]->ActivateAndBind(2);
	geometryTextures[3]->ActivateAndBind(3);

	GLuint lightShader = lightShaderNoShadows;

	glEnable(GL_STENCIL_TEST);
	for (auto& light : lights)
	{
		if (SceneGraph::Instance()->frustum.isBoundingSphereInView(light.object->bounds->centeredPosition, light.object->bounds->circumRadius))
		{
			light.object->inFrustum = true;
			if (light.CanCastShadow())
			{

				lightShader = lightShaderWithShadows;

				FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, light.shadowMapBuffer->handle);

				glDepthMask(GL_TRUE);
				glClearColor(1, 1, 0, 1);
				glClear(GL_DEPTH_BUFFER_BIT);
				glEnable(GL_DEPTH_TEST);
				glCullFace(GL_FRONT);
				ShaderManager::Instance()->SetCurrentShader(depthShader);

				glViewport(0, 0, light.shadowMapTexture->width, light.shadowMapTexture->height);
				drawCubeDepth(depthShader, objects, light.LightMatrixesVP, light.object);
				glViewport(0, 0, currentCamera->windowWidth, currentCamera->windowHeight);
				glCullFace(GL_BACK);
				glDepthMask(GL_FALSE);
				glClearColor(0, 0, 0, 1);

				light.shadowMapTexture->ActivateAndBind(4);
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

			float lightRadius = (float)light.object->bounds->radius;

			o_lpbd->SetData("lightPosition", &light.object->node->TopDownTransformF[3], sizeof(glm::vec3));
			o_lpbd->SetData("lightRadius", &lightRadius, sizeof(float));
			o_lpbd->SetData("constant", &light.attenuation.Constant, sizeof(float));
			o_lpbd->SetData("linear", &light.attenuation.Linear, sizeof(float));
			o_lpbd->SetData("exponential", &light.attenuation.Exponential, sizeof(float));

			m_lbd->SetData("lightColor", &light.properties.color, sizeof(Vector3F));
			m_lbd->SetData("lightPower", &light.properties.power, sizeof(float));
			m_lbd->SetData("ambient", &light.properties.ambient, sizeof(float));
			m_lbd->SetData("diffuse", &light.properties.diffuse, sizeof(float));
			m_lbd->SetData("specular", &light.properties.specular, sizeof(float));

			o_gbd->SetData("M", &light.object->node->TopDownTransformF, sizeof(Matrix4F));

			o_lpbd->Submit();
			m_lbd->Submit();
			o_gbd->Submit();

			light.object->materials[0][0]->vao->Bind();
			light.object->materials[0][0]->vao->Draw();

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

			light.object->materials[0][0]->vao->Draw();

			glCullFace(GL_BACK);
			glDisable(GL_BLEND);

			lightsRendered++;
		}
		else
		{
			light.object->inFrustum = false;
		}
	}
	glDisable(GL_STENCIL_TEST);
	return lightsRendered;
}

int
Render::drawSpotLights(const GLuint shaderID, const GLuint shadowShaderID, PoolParty<SpotLight>& lights, const std::vector<Object*>& objects, const glm::mat4& ViewProjection, FrameBuffer* fboToDrawTheLightTO, const std::vector<Texture*>& geometryTextures)
{
	glDepthMask(GL_FALSE);
	int lightsRendered = 0;

	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();

	geometryTextures[0]->ActivateAndBind(0);
	geometryTextures[1]->ActivateAndBind(1);
	geometryTextures[2]->ActivateAndBind(2);
	geometryTextures[3]->ActivateAndBind(3);

	GLuint lightShaderNoShadows = shaderID;
	GLuint lightShaderWithShadows = shadowShaderID;
	GLuint depthShader = GraphicsStorage::shaderIDs["Depth"];
	GLuint blurShader = GraphicsStorage::shaderIDs["FastBlurShadow"];
	GLuint stencilShader = GraphicsStorage::shaderIDs["Stencil"];

	GLuint lightShader = lightShaderNoShadows;

	glEnable(GL_STENCIL_TEST);
	for (auto& light : lights)
	{
		if (SceneGraph::Instance()->frustum.isBoundingSphereInView(light.object->bounds->centeredPosition, light.radius))
		{
			light.object->inFrustum = true;
			if (light.CanCastShadow())
			{

				lightShader = lightShaderWithShadows;

				FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, light.shadowMapBuffer->handle);

				glDepthMask(GL_TRUE);
				glClearColor(1, 1, 0, 1);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glEnable(GL_DEPTH_TEST);
				glCullFace(GL_FRONT);
				ShaderManager::Instance()->SetCurrentShader(depthShader);
				glViewport(0, 0, light.shadowMapTexture->width, light.shadowMapTexture->height);
				drawDepth(depthShader, objects, light.LightMatrixVP);
				glViewport(0, 0, currentCamera->windowWidth, currentCamera->windowHeight);
				glCullFace(GL_BACK);
				glDepthMask(GL_FALSE);
				glClearColor(0, 0, 0, 1);

				if (light.CanBlurShadowMap())
				{
					Texture* blurredShadowMap = nullptr;
					switch (light.blurMode)
					{
					case BlurMode::None:
						break;
					case BlurMode::OneSize:
						blurredShadowMap = BlurTextureAtSameSize(light.shadowMapTexture, light.pingPongBuffers[0], light.pingPongBuffers[1], light.activeBlurLevel, light.blurIntensity, blurShader, currentCamera->windowWidth, currentCamera->windowHeight);
						break;
					case BlurMode::MultiSize:
						blurredShadowMap = BlurTexture(light.shadowMapTexture, light.multiBlurBufferStart, light.multiBlurBufferTarget, light.activeBlurLevel, light.blurIntensity, blurShader, currentCamera->windowWidth, currentCamera->windowHeight);
						break;
					default:
						break;
					}
					blurredShadowMap->ActivateAndBind(4);
				}
				else
				{
					light.shadowMapTexture->ActivateAndBind(4);
				}
				float lightRadius = (float)light.object->bounds->radius;
				o_lssbd->SetData("depthBiasMVP", &light.BiasedLightMatrixVP, sizeof(glm::mat4));
				o_lssbd->SetData("lightPosition", &light.object->node->TopDownTransformF[3], sizeof(glm::vec3));
				o_lssbd->SetData("lightInvDir", &light.LightInvDir, sizeof(glm::vec3));
				o_lssbd->SetData("outerCutOff", &light.cosOuterCutOff, sizeof(float));
				o_lssbd->SetData("innerCutOff", &light.cosInnerCutOff, sizeof(float));
				o_lssbd->SetData("lightRadius", &lightRadius, sizeof(float));
				o_lssbd->SetData("constant", &light.attenuation.Constant, sizeof(float));
				o_lssbd->SetData("linear", &light.attenuation.Linear, sizeof(float));
				o_lssbd->SetData("exponential", &light.attenuation.Exponential, sizeof(float));
				o_lssbd->Submit();
			}
			else
			{
				lightShader = lightShaderNoShadows;
				float lightRadius = (float)light.object->bounds->radius;
				o_lsbd->SetData("lightPosition", &light.object->node->TopDownTransformF[3], sizeof(glm::vec3));
				o_lsbd->SetData("lightInvDir", &light.LightInvDir, sizeof(glm::vec3));
				o_lsbd->SetData("outerCutOff", &light.cosOuterCutOff, sizeof(float));
				o_lsbd->SetData("innerCutOff", &light.cosInnerCutOff, sizeof(float));
				o_lsbd->SetData("lightRadius", &lightRadius, sizeof(float));
				o_lsbd->SetData("constant", &light.attenuation.Constant, sizeof(float));
				o_lsbd->SetData("linear", &light.attenuation.Linear, sizeof(float));
				o_lsbd->SetData("exponential", &light.attenuation.Exponential, sizeof(float));
				o_lsbd->Submit();
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

			m_lbd->SetData("lightColor", &light.properties.color, sizeof(glm::vec3));
			m_lbd->SetData("lightPower", &light.properties.power, sizeof(float));
			m_lbd->SetData("ambient", &light.properties.ambient, sizeof(float));
			m_lbd->SetData("diffuse", &light.properties.diffuse, sizeof(float));
			m_lbd->SetData("specular", &light.properties.specular, sizeof(float));

			o_gbd->SetData("M", &light.object->node->TopDownTransformF, sizeof(glm::mat4));

			m_lbd->Submit();
			o_gbd->Submit();

			light.object->materials[0][0]->vao->Bind();
			light.object->materials[0][0]->vao->Draw();

			//-----------phase 2 light-----------

			ShaderManager::Instance()->SetCurrentShader(lightShader);

			glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

			glDisable(GL_DEPTH_TEST);

			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);

			light.object->materials[0][0]->vao->Draw();

			glCullFace(GL_BACK);
			glDisable(GL_BLEND);

			lightsRendered++;
		}
		else
		{
			light.object->inFrustum = false;
		}
	}
	glDisable(GL_STENCIL_TEST);

	return lightsRendered;
}

void
Render::drawHDR(const GLuint shaderID, Texture* colorTexture, Texture* bloomTexture)
{
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, 0);

	ShaderManager::Instance()->SetCurrentShader(shaderID);

	colorTexture->ActivateAndBind(0);
	if (bloomTexture != nullptr)
	{
		bloomTexture->ActivateAndBind(1);
	}

	g_psbd->UpdateAndSubmit();

	Plane::Instance()->vao.Bind();
	Plane::Instance()->vao.Draw();
}

void Render::drawHDRequirectangular(const GLuint shaderID, Texture * colorTexture)
{
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, 0);
	ShaderManager::Instance()->SetCurrentShader(shaderID);

	Camera* currentCamera = CameraManager::Instance()->GetCurrentCamera();
	glm::mat4 View = currentCamera->ViewMatrix;
	View[3][0] = 0;
	View[3][1] = 0;
	View[3][2] = 0;
	glm::mat4 ViewProjection = currentCamera->ProjectionMatrix * View;

	glm::mat4 model = glm::mat4(1);
	MathUtils::SetScale(model, glm::vec3(20, 20, 20));
	glm::mat4 MVP = (CameraManager::Instance()->ViewProjection * model);
	GLuint MatrixHandle = glGetUniformLocation(shaderID, "MVPSkybox");
	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);

	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	colorTexture->ActivateAndBind(0);

	VertexArray* unitCube = nullptr;
	for (auto& vao : *GraphicsStorage::assetRegistry.GetPool<VertexArray>())
	{
		if (vao.name.compare("unitCube") == 0)
		{
			unitCube = &vao;
			break;
		}
	}
	
	unitCube->Bind();
	unitCube->Draw();
}

void Render::drawRegion(const GLuint shaderID, int posX, int posY, int width, int height, const Texture * texture)
{
	if (texture != nullptr)
	{	
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
}

void
Render::AddPingPongBuffer(int width, int height)
{
	for (int i = 0; i < 2; i++)
	{
		FrameBuffer* pingPongBuffer = GraphicsStorage::assetRegistry.AllocAsset<FrameBuffer>((unsigned int)GL_FRAMEBUFFER);
		FBOManager::Instance()->AddFrameBuffer(pingPongBuffer, false);
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
		FrameBuffer* multiBlurBuffer = GraphicsStorage::assetRegistry.AllocAsset<FrameBuffer>((unsigned int)GL_FRAMEBUFFER);
		FBOManager::Instance()->AddFrameBuffer(multiBlurBuffer, true);
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
			FrameBuffer* childBlurBuffer = GraphicsStorage::assetRegistry.AllocAsset<FrameBuffer>((unsigned int)GL_FRAMEBUFFER);
			FBOManager::Instance()->AddFrameBuffer(multiBlurBuffer, false);
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

void
Render::InitializeShderBlockDatas()
{
	//geometry
	//ShaderBlockData* o_gbd;
	//ShaderBlockData* m_gbd;
	//light material buffer
	//ShaderBlockData* m_lbd;
	//directional
	//ShaderBlockData* o_ldbd;
	//ShaderBlockData* o_ldsbd;
	//spot
	//ShaderBlockData* o_lsbd;
	//ShaderBlockData* o_lssbd;
	//point
	//ShaderBlockData* o_lpbd;
	//camera
	//ShaderBlockData* g_csbd;
	//post
	//ShaderBlockData* g_psbd;
	//time
	//ShaderBlockData* g_tsbd;
	//depth
	//ShaderBlockData* m_lvpbd;
	auto ub = GraphicsStorage::GetUniformBuffer("O_GBVars");
	if (ub != nullptr) o_gbd = new ShaderBlockData(ub);
	ub = GraphicsStorage::GetUniformBuffer("M_GBVars");
	if (ub != nullptr) m_gbd = new ShaderBlockData(ub);
	ub = GraphicsStorage::GetUniformBuffer("M_LBVars");
	if (ub != nullptr) m_lbd = new ShaderBlockData(ub);
	ub = GraphicsStorage::GetUniformBuffer("O_LDBVars");
	if (ub != nullptr) o_ldbd = new ShaderBlockData(ub);
	ub = GraphicsStorage::GetUniformBuffer("O_LDSBVars");
	if (ub != nullptr) o_ldsbd = new ShaderBlockData(ub);
	ub = GraphicsStorage::GetUniformBuffer("O_LSBVars");
	if (ub != nullptr) o_lsbd = new ShaderBlockData(ub);
	ub = GraphicsStorage::GetUniformBuffer("O_LSSBVars");
	if (ub != nullptr) o_lssbd = new ShaderBlockData(ub);
	ub = GraphicsStorage::GetUniformBuffer("O_LPBVars");
	if (ub != nullptr) o_lpbd = new ShaderBlockData(ub);
	ub = GraphicsStorage::GetUniformBuffer("M_LVPVars");
	if (ub != nullptr) m_lvpbd = new ShaderBlockData(ub);
	ub = GraphicsStorage::GetUniformBuffer("G_CBVars");
	if (ub != nullptr)
	{
		g_csbd = new ShaderBlockData(ub);
		g_csbd->RegisterPropertyData("VP", &CameraManager::Instance()->ViewProjectionF, sizeof(glm::mat4));
		g_csbd->RegisterPropertyData("screenSize", &CameraManager::Instance()->screenSize, sizeof(glm::vec2));
		g_csbd->RegisterPropertyData("far", &CameraManager::Instance()->far, sizeof(float));
		g_csbd->RegisterPropertyData("near", &CameraManager::Instance()->near, sizeof(float));
		g_csbd->RegisterPropertyData("cameraPos", &CameraManager::Instance()->cameraPos, sizeof(glm::vec3));
		g_csbd->RegisterPropertyData("cameraUp", &CameraManager::Instance()->cameraUp, sizeof(glm::vec3));
		g_csbd->RegisterPropertyData("cameraRight", &CameraManager::Instance()->cameraRight, sizeof(glm::vec3));
		g_csbd->RegisterPropertyData("cameraForward", &CameraManager::Instance()->cameraForward, sizeof(glm::vec3));
	}
	ub = GraphicsStorage::GetUniformBuffer("G_PBVars");
	if (ub != nullptr)
	{
		g_psbd = new ShaderBlockData(ub);
		g_psbd->RegisterPropertyData("gamma", &gamma, sizeof(float));
		g_psbd->RegisterPropertyData("exposure", &exposure, sizeof(float));
		g_psbd->RegisterPropertyData("brightness", &brightness, sizeof(float));
		g_psbd->RegisterPropertyData("contrast", &contrast, sizeof(float));
		g_psbd->RegisterPropertyData("bloomIntensity", &bloomIntensity, sizeof(float));
		g_psbd->RegisterPropertyData("hdrEnabled", &hdrEnabled, sizeof(bool));
		g_psbd->RegisterPropertyData("bloomEnabled", &bloomEnabled, sizeof(bool));
	}
	ub = GraphicsStorage::GetUniformBuffer("G_TBVars");
	if (ub != nullptr)
	{
		g_tsbd = new ShaderBlockData(ub);
		g_tsbd->RegisterPropertyData("currentTime", &Times::Instance()->currentTimeF, sizeof(float));
		g_tsbd->RegisterPropertyData("deltaTime", &Times::Instance()->deltaTimeF, sizeof(float));
		g_tsbd->RegisterPropertyData("deltaTimeInverse", &Times::Instance()->dtInvF, sizeof(float));
		g_tsbd->RegisterPropertyData("previousTime", &Times::Instance()->previousTimeF, sizeof(float));
		g_tsbd->RegisterPropertyData("timeStep", &Times::Instance()->timeStepF, sizeof(float));
	}
}

void
Render::UpdateShaderBlockDatas()
{
	if (g_csbd != nullptr) g_csbd->UpdateAndSubmit();
	if (g_psbd != nullptr) g_psbd->UpdateAndSubmit();
	if (g_tsbd != nullptr) g_tsbd->UpdateAndSubmit();
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
	dirShadowMapBuffer = GraphicsStorage::assetRegistry.AllocAsset<FrameBuffer>((unsigned int)GL_FRAMEBUFFER);
	FBOManager::Instance()->AddFrameBuffer(dirShadowMapBuffer, false);
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

void Render::RenderGraph()
{
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	std::chrono::duration<double> elapsed_seconds;

	start = std::chrono::high_resolution_clock::now();
	/*
	if (showRenderList)
	{
		ImGui::Begin("Render List", &showRenderList);
		ImGui::Text("Draw Calls: %d", totalNrOfDrawCalls);
		for (auto& element : finalRenderList)
		{
			if (dynamic_cast<RenderProfile*>(element) != nullptr)
			{
				ImGui::Text("\tRP: %s", element->name.c_str());
			}
			else if (dynamic_cast<TextureProfile*>(element) != nullptr)
			{
				ImGui::Text("\tTP: %s", element->name.c_str());
			}
			else if (dynamic_cast<MaterialProfile*>(element) != nullptr)
			{
				ImGui::Text("\tMP: %s", element->name.c_str());
			}
			else if (dynamic_cast<VertexArray*>(element) != nullptr)
			{
				ImGui::Text("\tVao: %s", element->name.c_str());
			}
			else if (dynamic_cast<RenderPass*>(element) != nullptr)
			{
				ImGui::Text("RPS: %s", element->name.c_str());
			}
			else if (dynamic_cast<ObjectProfile*>(element) != nullptr)
			{
				ImGui::Text("\tOP: %s", element->name.c_str());
			}
			else if (dynamic_cast<Shader*>(element) != nullptr)
			{
				ImGui::Text("\tSH: %s", element->name.c_str());
			}
			else if (dynamic_cast<VertexArray::DrawElement*>(element) != nullptr)
			{
				ImGui::Text("\t\tDW");
			}
		}
		ImGui::End();
	}
	*/

	for (auto& element : finalRenderList)
	{
		element->Execute();
	}

	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	executingGraphTime = elapsed_seconds.count();
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

	for (int i = 0; i < outputLevel + 1; i++)
	{
		textureWidth = startFrameBuffer[i]->textures[0]->width;
		textureHeight = startFrameBuffer[i]->textures[0]->height;

		glViewport(0, 0, textureWidth, textureHeight);
		
		BlurOnOneAxis(HorizontalSourceTexture, startFrameBuffer[i], blurSize / ((float)textureWidth), 0.f, offset); //horizontally
		BlurOnOneAxis(startFrameBuffer[i]->textures[0], targetFrameBuffer[i], 0.f, blurSize / ((float)textureHeight), offset); //vertically
		HorizontalSourceTexture = targetFrameBuffer[i]->textures[0];
	}

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

	for (int i = 0; i < outputLevel + 1; i++)
	{
		BlurOnOneAxis(HorizontalSourceTexture, startFrameBuffer, offsetWidth, 0.f, offset); //horizontally
		BlurOnOneAxis(startFrameBuffer->textures[0], targetFrameBuffer, 0.f, offsetHeight, offset); //vertically
		HorizontalSourceTexture = targetFrameBuffer->textures[0];
	}

	glViewport(0, 0, windowWidth, windowHeight);

	return targetFrameBuffer->textures[0];
}

void
Render::drawParticles(const GLuint shaderID, std::vector<ParticleSystem*>& particleSystems, FrameBuffer* targetFrameBuffer)
{
	FBOManager::Instance()->BindFrameBuffer(GL_DRAW_FRAMEBUFFER, targetFrameBuffer->handle); //we bind the lightandposteffect buffer for drawing

	ShaderManager::Instance()->SetCurrentShader(shaderID);

	Camera* camera = CameraManager::Instance()->GetCurrentCamera();
	int windowWidth = camera->windowWidth;
	int windowHeight = camera->windowHeight;
	float softScale = 0.5f;
	float contrastPower = 0.5f;

	//function should take in the render targets list
	//GraphicsStorage::renderTargets["V_depth"]->ActivateAndBind(1);

	//global
	GLuint screenSize = glGetUniformLocation(shaderID, "screenSize");
	glUniform2f(screenSize, windowWidth, windowHeight);

	GLuint farPlane = glGetUniformLocation(shaderID, "far");
	glUniform1f(farPlane, camera->far);

	GLuint nearPlane = glGetUniformLocation(shaderID, "near");
	glUniform1f(nearPlane, camera->near);

	//material profile
	GLuint soft = glGetUniformLocation(shaderID, "softScale");
	glUniform1f(soft, softScale);

	GLuint contrast = glGetUniformLocation(shaderID, "contrastPower");
	glUniform1f(contrast, contrastPower);

	int particlesRendered = 0;
	for (auto& pSystem : particleSystems) //particles not affected by light, rendered in forward rendering
	{
		if (SceneGraph::Instance()->frustum.isBoundingSphereInView(pSystem->object->bounds->centeredPosition, 1.0)) {
			particlesRendered += pSystem->Draw();
		}
	}
}
