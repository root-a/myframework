#pragma once
#include "MyMathLib.h"
#include <vector>
#include <functional>
#include "RenderElement.h"

class ElementBuffer;
class VertexBuffer;
class VertexBufferDynamic;

enum class PrimitiveMode
{
	POINTS, LINE_STRIP, LINE_LOOP, LINES, LINE_STRIP_ADJACENCY, LINES_ADJACENCY, TRIANGLE_STRIP, TRIANGLE_FAN, TRIANGLES, TRIANGLE_STRIP_ADJACENCY, TRIANGLES_ADJACENCY
};

class VertexArray : public RenderElement
{
private:
	enum class DrawFunction
	{
		none, drawElements, drawArrays, drawArraysInstanced, drawElementsInstanced
	};
public:
	class DrawElement : public RenderElement
	{
	public:
		DrawElement() {}
		~DrawElement() {}
		void Execute() { vao->Draw(); }
		VertexArray* vao;
	private:

	};

	VertexArray();
	~VertexArray();
	void Bind();
	void Unbind();
	void SetPrimitiveMode(PrimitiveMode mode);
	PrimitiveMode GetPrimitiveMode();
	const char* GetPrimitiveModeAsStr();
	DrawElement draw;
	unsigned int AddVertexBuffer(VertexBuffer* vbo);
	unsigned int ReAddVertexBuffer(VertexBuffer* vbo);
	void AddElementBuffer(ElementBuffer* newEbo);
	unsigned int ReAddElementBuffer(ElementBuffer* newEbo);
	void UpdateDrawFunction();
	void ResizeVertexBuffer(VertexBuffer* bufferToResize, void* newData, unsigned int newElementCount);
	void ResizeElementBuffer(ElementBuffer* bufferToResize, void* newData, unsigned int newElementCount);
	void Draw();
	unsigned int handle;
	std::vector<unsigned int> attributeIndexes;
	std::vector<unsigned int> bindingIndexes;
	std::vector<VertexBuffer*> vbos;
	ElementBuffer* ebo;
	std::string configPath;
	std::string meshPath;
	std::string path;

	std::vector<VertexBufferDynamic*> dynamicVBOs;
	unsigned int vertexCount;
	
	unsigned int activeCount;
	DrawFunction drawFunction;
	bool instanced = false;
	void Execute();
	glm::vec3 center;
	glm::vec3 dimensions;
	void* dataToUpdate;
private:
	static VertexArray* currentVao;
	
	unsigned int PrimitiveModeToOpenGLBaseType(PrimitiveMode mode);
	void RegisterVertexBuffer(VertexBuffer* vbo);
	unsigned int primitiveMode;
	unsigned int attributeIndex;
	unsigned int bindingIndex;
	bool bufferIsDynamic;
};

