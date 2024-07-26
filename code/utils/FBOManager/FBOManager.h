#pragma once
#include <vector>

class Texture;
class FrameBuffer;

class FBOManager
{
	typedef unsigned int GLuint;
	typedef unsigned int GLenum;
public:
	static FBOManager* Instance();
	void UpdateTextureBuffers(int windowWidth, int windowHeight);
	void BindFrameBuffer(GLuint readWriteMode, GLuint frameBuffer);
	FrameBuffer* Generate2DShadowMapBuffer(FrameBuffer* staticFrameBuffer, int width, int height);
	FrameBuffer* Generate3DShadowMapBuffer(FrameBuffer* staticFrameBuffer, int width, int height);
	FrameBuffer* GetCurrentDrawFrameBuffer();
	FrameBuffer* GetCurrentReadFrameBuffer();
	void DeleteFrameBuffer(FrameBuffer* buffer);
	void MakeStatic(FrameBuffer* buffer);
	void MakeDynamic(FrameBuffer* buffer);
	bool IsDynamic(FrameBuffer* buffer);
	std::vector<FrameBuffer*> dynamicBuffers;
	std::vector<FrameBuffer*> staticBuffers;
	void AddFrameBuffer(FrameBuffer* buffer, bool dynamic = true);
	GLuint readBuffer;
	GLuint drawBuffer;
private:
	FBOManager();
	~FBOManager();
	//copy
	FBOManager(const FBOManager&);
	//assign
	FBOManager& operator=(const FBOManager&);
};