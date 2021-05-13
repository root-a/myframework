#include "RenderBuffer.h"

RenderBuffer::RenderBuffer(GLint internalFormat, GLsizei width, GLsizei height, GLenum attachment)
{
	this->internalFormat = internalFormat;
	this->width = width;
	this->height = height;
	this->aspect = (double)width / (double)height;
	this->attachment = attachment;
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
	glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
}

void RenderBuffer::Generate()
{
	glGenRenderbuffers(1, &handle);
}

void RenderBuffer::Bind()
{
	glBindRenderbuffer(GL_RENDERBUFFER, handle);
}

void RenderBuffer::Specify()
{
	glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
}

void RenderBuffer::GenerateBindSpecify()
{
	glGenRenderbuffers(1, &handle);
	glBindRenderbuffer(GL_RENDERBUFFER, handle);
	glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
}