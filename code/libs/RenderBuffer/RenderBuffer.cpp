#include "RenderBuffer.h"

RenderBuffer::RenderBuffer(GLint internalFormat, GLsizei width, GLsizei height, GLenum attachment)
{
	this->internalFormat = internalFormat;
	this->width = width;
	this->height = height;
	this->aspect = (double)width / (double)height;
	this->attachment = attachment;
	this->target = GL_RENDERBUFFER;
}

RenderBuffer::~RenderBuffer()
{
	glDeleteRenderbuffers(1, &handle);
}

void RenderBuffer::Update(int windowWidth, int windowHeight)
{
	width = windowWidth;
	height = windowHeight;
	aspect = (double)width / (double)height;
	glRenderbufferStorage(target, internalFormat, width, height);
}

void RenderBuffer::Generate()
{
	glGenRenderbuffers(1, &handle);
}

void RenderBuffer::Bind()
{
	glBindRenderbuffer(target, handle);
}

void RenderBuffer::Specify()
{
	glRenderbufferStorage(target, internalFormat, width, height);
}

void RenderBuffer::GenerateBindSpecify()
{
	glGenRenderbuffers(1, &handle);
	glBindRenderbuffer(target, handle);
	glRenderbufferStorage(target, internalFormat, width, height);
}