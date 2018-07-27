#pragma once
#include <GL/glew.h>
#include <vector>
class Texture;

class FrameBuffer
{
public:
	FrameBuffer(GLenum target, int scaleX = 1, int scaleY = 1, bool dynamic = true);
	~FrameBuffer() {};

	GLuint handle;
	void GenerateAndAddTextures();
	void BindBuffer(GLenum target);
	void UnBindBuffer(GLenum target);
	std::vector<Texture*> textures;
	Texture* RegisterTexture(Texture* texture);
	void AttachTexture(Texture* texture);
	void RegisterChildBuffer(FrameBuffer* child);
	void UpdateTextures(int newBufferSizeX, int newBufferSizeY);
	void AddDefaultTextureParameters();
	void CheckAndCleanup();
	void ReadPixelData(unsigned int x, unsigned int y, GLenum readTextureFormat, GLenum sendDataType, void * data, GLenum attachment);
	std::vector<FrameBuffer*> children;
	double scaleXFactor;
	double scaleYFactor;
	bool dynamicSize;
private:
	FrameBuffer() {};
};