#pragma once
#include <vector>
#include "Vector4F.h"
#include <GL/glew.h>

struct TextureParameterI
{
	GLenum pname;
	GLint param;

	TextureParameterI(GLenum pname, GLint param) : pname(pname), param(param) {}
	~TextureParameterI() {};
};

struct TextureParameterF
{
	GLenum pname;
	GLfloat param;

	TextureParameterF(GLenum pname, GLfloat param) : pname(pname), param(param) {}
	~TextureParameterF() {};
};

class Texture
{
public:
	Texture(GLenum target, GLint level,	GLint interlalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels, GLenum attachment);
	~Texture();
	void AddTextureParameterI(GLenum pname, GLint param);
	void AddTextureParameterF(GLenum pname, GLfloat param);
	void AddDefaultTextureParameters();
	void AddClampingToEdge();
	void AddClampingToBorder(mwm::Vector4F& borderColor);
	void SetLinear();
	void SetNearest();
	void Update(int windowWidth, int windowHeight);
	void Generate();
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
	const void* pixels;
	GLenum attachment;
	std::vector<TextureParameterI> parametersI;
	std::vector<TextureParameterF> parametersF;
	bool customSize;
	bool hasBorder;
	mwm::Vector4F borderColor;
private:
	Texture() {};
};