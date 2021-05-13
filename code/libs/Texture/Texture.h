#pragma once
#include <vector>
#include "Vector4F.h"
#include <GL/glew.h>
#include <string>

class Texture
{
public:
	Texture(GLenum target, GLint level,	GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels, GLenum attachment);
	~Texture();
	void SetTextureParameterI(GLenum pname, GLint param);
	void SetTextureParameterIV(GLenum pname, int* param);
	void SetTextureParameterF(GLenum pname, GLfloat param);
	void SetTextureParameterFV(GLenum pname, float* param);
	void SetDefaultParameters();
	void SetClampingToEdge();
	void SetClampingToBorder(Vector4F& borderColor);
	void SetLinear();
	void SetNearest();
	void SetFiltering(GLint MIN_FILTER, GLint MAG_FILTER);
	void GenerateMipMaps();
	void Update(int windowWidth, int windowHeight);
	void Generate();
	void GenerateBindSpecify();
	void Specify();
	void SpecifyTexture(GLenum target, GLsizei width, GLsizei height, void* pixels);
	void ActivateAndBind(int textureSlot) const;
	void Bind() const;
	static void Activate(int textureSlot);
	GLuint handle;
	GLenum target;
	GLint level;
	GLint internalFormat;
	GLsizei width;
	GLsizei height;
	double aspect;
	GLint border;
	GLenum format;
	GLenum type;
	void* pixels;
	GLenum attachment;
	std::string name;
	bool hasMipMaps;
	Vector4F borderColor;
private:
	Texture() {};
};