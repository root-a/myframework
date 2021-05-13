#pragma once
#include <vector>
#include <string>

class Texture;
class RenderBuffer;

class FrameBuffer
{
	typedef unsigned int GLuint;
	typedef unsigned int GLenum;
	typedef int GLint;
	typedef int GLsizei;
public:
	FrameBuffer(GLenum target, int scaleX = 1, int scaleY = 1);
	~FrameBuffer();

	GLuint handle;
	std::vector<RenderBuffer*> renderBuffers;
	void SpecifyTextures();
	void BindBuffer(GLenum target);
	std::vector<Texture*> textures;
	std::vector<GLenum> attachments;
	RenderBuffer* RegisterRenderBuffer(RenderBuffer* buffer);
	void RegisterTexture(Texture* texture);
	void SpecifyTexture(Texture* texture);
	void SpecifyRenderBuffer(RenderBuffer* texture);
	void SpecifyTextureAndMip(Texture* texture, GLenum target, GLint level = 0);
	void RegisterChildBuffer(FrameBuffer* child);
	void UpdateTextures(int newBufferSizeX, int newBufferSizeY);
	void CheckAndCleanup();
	void ReadPixelData(GLuint x, GLuint y, GLuint width, GLuint height, GLenum sendDataType, void * data, Texture* texture);
	void DeleteAllTextures();
	std::vector<FrameBuffer*> children;
	double scaleXFactor;
	double scaleYFactor;
	void DeactivateDrawBuffers();
	void ActivateDrawBuffers();
	std::string name;
private:
	FrameBuffer() {};
};