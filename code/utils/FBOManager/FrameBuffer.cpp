#include "FrameBuffer.h"
#include "Texture.h"
#include "FBOManager.h"
#include <GL/glew.h>

FrameBuffer::FrameBuffer(GLenum target, int scaleX, int scaleY, bool dynamic)
{
	glGenFramebuffers(1, &handle);
	FBOManager::Instance()->BindFrameBuffer(target, handle);
	scaleXFactor = scaleX;
	scaleYFactor = scaleY;
	dynamicSize = dynamic;
}

FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &handle);
}

void FrameBuffer::GenerateAndAddTextures()
{
	for (auto& texture : textures)
	{
		texture->Generate();
		if (texture->target == GL_TEXTURE_CUBE_MAP) glFramebufferTexture(GL_DRAW_FRAMEBUFFER, texture->attachment, texture->handle, texture->level);
		else glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, texture->attachment, texture->target, texture->handle, texture->level);
	}
}

void FrameBuffer::BindBuffer(GLenum target)
{
	FBOManager::Instance()->BindFrameBuffer(target, handle);
}

Texture* FrameBuffer::RegisterTexture(Texture* texture)
{
	textures.push_back(texture);
	if (texture->format != GL_DEPTH_COMPONENT) attachments.push_back(texture->attachment);
	return texture;
}

void FrameBuffer::RegisterChildBuffer(FrameBuffer * child)
{
	children.push_back(child);
}

void FrameBuffer::AttachTexture(Texture * texture)
{
	if (texture->target == GL_TEXTURE_CUBE_MAP) glFramebufferTexture(GL_DRAW_FRAMEBUFFER, texture->attachment, texture->handle, texture->level);
	else glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, texture->attachment, texture->target, texture->handle, texture->level);
}

void FrameBuffer::UpdateTextures(int newBufferSizeX, int newBufferSizeY)
{
	int newXSize = newBufferSizeX * scaleXFactor;
	int newYSize = newBufferSizeY * scaleYFactor;
	for (auto& texture : textures)
	{
		texture->Update(newXSize, newYSize);
	}
	for (auto& child : children)
	{
		child->UpdateTextures(newXSize, newYSize);
	}
}

void FrameBuffer::AddDefaultTextureParameters()
{
	for (auto& texture : textures)
	{
		texture->AddDefaultTextureParameters();
	}
}

void FrameBuffer::CheckAndCleanup()
{
	// Disable reading to avoid problems with older GPUs
	//can't read unless you specify it later
	//glReadBuffer(GL_NONE);
	//disable draw if we don't want to draw to all
	//glDrawBuffer(GL_NONE);
	//glDrawBuffer(GL_COLOR_ATTACHMENT0);
	//glDrawBuffer(GL_NONE);
	// Verify that the FBO is correct
	ActivateDrawBuffers();
	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		printf("FB error, status: 0x%x\n", Status);
		return;
	}

	// Restore the default framebuffer
	glBindTexture(GL_TEXTURE_2D, 0);
	FBOManager::Instance()->BindFrameBuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::ReadPixelData(GLuint x, GLuint y, GLuint width, GLuint height, GLenum sendDataType, void* data, Texture* texture)
{
	BindBuffer(GL_READ_FRAMEBUFFER);
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, handle);
	glReadBuffer(texture->attachment);
	glReadPixels(x, y, width, height, texture->format, sendDataType, data);
	//glReadBuffer(GL_NONE);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//UnBindBuffer(GL_READ_FRAMEBUFFER);
}

void FrameBuffer::DeleteAllTextures()
{
	for (auto& texture : textures)
	{
		delete texture;
	}
	textures.clear();
}

void FrameBuffer::ActivateDrawBuffers()
{
	if (attachments.size() > 0)
	{
		glDrawBuffers(attachments.size(), &attachments[0]);
	}
}

void FrameBuffer::DeactivateDrawBuffers()
{
	glDrawBuffer(GL_NONE);
}
