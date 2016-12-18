#include "Render.h"
#include "MyMathLib.h"
#include <GL/glew.h>
#include "Object.h"
#include "Material.h"
#include "Mesh.h"

using namespace mwm;

void 
Render::drawLight(const Object* object, const Matrix4& ViewProjection, const GLuint currentShaderID)
{
	Matrix4 dModel = object->CalculateOffsettedModel();
	Matrix4F MVP = (dModel*ViewProjection).toFloat();

	GLuint MatrixHandle = glGetUniformLocation(currentShaderID, "MVP");
	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);

	GLuint LightPosHandle = glGetUniformLocation(currentShaderID, "LightPosition_worldspace");

	glUniform3fv(LightPosHandle, 1, &object->node.position.x);

	GLuint LightRadius = glGetUniformLocation(currentShaderID, "lightRadius");
	glUniform1f(LightRadius, object->radius);

	GLuint LightPower = glGetUniformLocation(currentShaderID, "lightPower");
	glUniform1f(LightPower, object->mat->diffuseIntensity);

	GLuint LightColor = glGetUniformLocation(currentShaderID, "lightColor");
	glUniform3fv(LightColor, 1, &object->mat->color.x);

	//bind vao before drawing
	glBindVertexArray(object->mesh->vaoHandle);

	// Draw the triangles !
	glDrawElements(GL_TRIANGLES, object->mesh->indicesSize, GL_UNSIGNED_INT, (void*)0); // mode, count, type, element array buffer offset
}

void 
Render::drawGeometry(const Object* object, const Matrix4& ViewProjection, const GLuint currentShaderID)
{
	Matrix4 dModel = object->CalculateOffsettedModel();
	Matrix4F ModelMatrix = dModel.toFloat();
	Matrix4F MVP = (dModel*ViewProjection).toFloat();
	//vertex
	GLuint MatrixHandle = glGetUniformLocation(currentShaderID, "MVP");
	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);

	GLuint ModelMatrixHandle = glGetUniformLocation(currentShaderID, "M");
	glUniformMatrix4fv(ModelMatrixHandle, 1, GL_FALSE, &ModelMatrix[0][0]);

	GLuint MaterialPropertiesHandle = glGetUniformLocation(currentShaderID, "MaterialProperties");
	GLuint MaterialColorHandle = glGetUniformLocation(currentShaderID, "MaterialColor");
	Vector4 matProperties = Vector4(object->mat->ambientIntensity, object->mat->diffuseIntensity, object->mat->specularIntensity, object->mat->shininess);
	glUniform4fv(MaterialPropertiesHandle, 1, &matProperties.x);
	glUniform3fv(MaterialColorHandle, 1, &object->mat->color.x);

	GLuint TextureSamplerHandle = glGetUniformLocation(currentShaderID, "myTextureSampler");
	//we bind texture for each object since it can be different 
	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, object->mat->texture2D->TextureID);
	// Set our "myTextureSampler" sampler to user Texture Unit 0
	glUniform1i(TextureSamplerHandle, 0);

	//bind vao before drawing
	glBindVertexArray(object->mesh->vaoHandle);

	// Draw the triangles !
	glDrawElements(GL_TRIANGLES, object->mesh->indicesSize, GL_UNSIGNED_INT, (void*)0); // mode, count, type, element array buffer offset
}


void 
Render::draw(const Object* object, const Matrix4& ViewProjection, const GLuint currentShaderID)
{
	Matrix4 dModel = object->CalculateOffsettedModel();
	Matrix4F ModelMatrix = dModel.toFloat();
	Matrix4F MVP = (dModel*ViewProjection).toFloat();
	Matrix4F depthBiasMVP = (object->depthMVP*Matrix4::biasMatrix()).toFloat();

	GLuint MatrixHandle = glGetUniformLocation(currentShaderID, "MVP");
	GLuint DepthBiasMatrixHandle = glGetUniformLocation(currentShaderID, "DepthBiasMVP");
	GLuint ModelMatrixHandle = glGetUniformLocation(currentShaderID, "M");

	GLuint MaterialPropertiesHandle = glGetUniformLocation(currentShaderID, "MaterialProperties");
	GLuint MaterialColorHandle = glGetUniformLocation(currentShaderID, "MaterialColor");
	GLuint PickingObjectIndexHandle = glGetUniformLocation(currentShaderID, "objectID");
	GLuint TextureSamplerHandle = glGetUniformLocation(currentShaderID, "myTextureSampler");

	glUniformMatrix4fv(MatrixHandle, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(ModelMatrixHandle, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(DepthBiasMatrixHandle, 1, GL_FALSE, &depthBiasMVP[0][0]);

	Vector4 matProperties = Vector4(object->mat->ambientIntensity, object->mat->diffuseIntensity, object->mat->specularIntensity, object->mat->shininess);
	glUniform4fv(MaterialPropertiesHandle, 1, &matProperties.x);
	glUniform3fv(MaterialColorHandle, 1, &object->mat->color.x);

	glUniform1ui(PickingObjectIndexHandle, object->ID);

	//we bind texture for each object since it can be different 
	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, object->mat->texture2D->TextureID);
	// Set our "myTextureSampler" sampler to user Texture Unit 0
	glUniform1i(TextureSamplerHandle, 0);

	//bind vao before drawing
	glBindVertexArray(object->mesh->vaoHandle);

	// Draw the triangles !
	glDrawElements(GL_TRIANGLES, object->mesh->indicesSize, GL_UNSIGNED_INT, (void*)0); // mode, count, type, element array buffer offset
}

void 
Render::drawDepth(Object* object, const Matrix4& ViewProjection, const GLuint currentShaderID)
{
	Matrix4 dModel = object->CalculateOffsettedModel();
	object->depthMVP = dModel*ViewProjection;
	Matrix4F MVP = (object->depthMVP).toFloat();

	GLuint depthMatrixHandle = glGetUniformLocation(currentShaderID, "depthMVP");

	glUniformMatrix4fv(depthMatrixHandle, 1, GL_FALSE, &MVP[0][0]);

	//bind vao before drawing
	glBindVertexArray(object->mesh->vaoHandle);

	// Draw the triangles !
	glDrawElements(GL_TRIANGLES, object->mesh->indicesSize, GL_UNSIGNED_INT, (void*)0); // mode, count, type, element array buffer offset
}