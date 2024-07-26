#pragma once
#include <vector>
#include "Vector4F.h"
#include <GL/glew.h>
#include <string>

class RenderBuffer
{
public:
	RenderBuffer(GLint internalFormat, GLsizei width, GLsizei height, GLenum attachment);
	~RenderBuffer();
	void Update(int windowWidth, int windowHeight);
	void Generate();
	void Bind();
	void Specify();
	void GenerateBindSpecify();
	GLuint handle;
	GLint internalFormat;
	GLsizei width;
	GLsizei height;
	GLenum target;
	double aspect;
	GLenum attachment;
	std::string name;
	std::string path;
private:
	RenderBuffer() {};
};