#include "FrameBuffer.h"
#include "Texture.h"

FrameBuffer::FrameBuffer(GLenum target, int scaleX, int scaleY, bool dynamic)
{
	glGenFramebuffers(1, &handle);
	glBindFramebuffer(target, handle);
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
	glBindFramebuffer(target, handle);
}

void FrameBuffer::UnBindBuffer(GLenum target)
{
	glBindFramebuffer(target, 0);
}

Texture* FrameBuffer::RegisterTexture(Texture* texture)
{
	textures.push_back(texture);
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
	glReadBuffer(GL_NONE);
	//disable draw if we don't want to draw to all
	//glDrawBuffer(GL_NONE);
	//glDrawBuffer(GL_COLOR_ATTACHMENT0);
	//glDrawBuffer(GL_NONE);
	// Verify that the FBO is correct
	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		printf("FB error, status: 0x%x\n", Status);
		return;
	}

	// Restore the default framebuffer
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::ReadPixelData(unsigned int x, unsigned int y, GLenum readTextureFormat, GLenum sendDataType, void* data, GLenum attachment)
{
	BindBuffer(GL_READ_FRAMEBUFFER);
	glReadBuffer(attachment);
	glReadPixels(x, y, 1, 1, readTextureFormat, sendDataType, data);
	glReadBuffer(GL_NONE);
	UnBindBuffer(GL_READ_FRAMEBUFFER);
}

void FrameBuffer::DeleteAllTextures()
{
	for (auto& texture : textures)
	{
		delete texture;
	}
	textures.clear();
}
