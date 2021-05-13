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
	FrameBuffer* Generate2DShadowMapBuffer(int width, int height);
	FrameBuffer* Generate3DShadowMapBuffer(int width, int height);
	void DeleteFrameBuffer(FrameBuffer* buffer);
	void MakeStatic(FrameBuffer* buffer);
	void MakeDynamic(FrameBuffer* buffer);
	bool IsDynamic(FrameBuffer* buffer);
	std::vector<FrameBuffer*> dynamicBuffers;
	std::vector<FrameBuffer*> staticBuffers;
	FrameBuffer* GenerateFBO(bool dynamic = true);
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