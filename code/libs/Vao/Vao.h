#pragma once
#include "MyMathLib.h"
#include <vector>
#include "RenderElement.h"

enum ShaderDataType
{
	Float, Float2, Float3, Float4, Mat2, Mat3, Mat4, Int, Int2, Int3, Int4, Bool, Bool2, Bool3, Bool4
};

enum IndicesType
{
	UNSIGNED_BYTE, UNSIGNED_SHORT, UNSIGNED_INT
};

struct LayoutLocation
{
	std::string name;
	ShaderDataType type;
	unsigned int size;
	unsigned int offset;
	bool normalized;
	unsigned int instancesPerAttribute = 0;

	unsigned int ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:		return 4;
		case ShaderDataType::Float2:	return 4 * 2;
		case ShaderDataType::Float3:	return 4 * 3;
		case ShaderDataType::Float4:	return 4 * 4;
		case ShaderDataType::Mat2:		return 4 * 2 * 2;
		case ShaderDataType::Mat3:		return 4 * 3 * 3;
		case ShaderDataType::Mat4:		return 4 * 4 * 4;
		case ShaderDataType::Int:		return 4;
		case ShaderDataType::Int2:		return 4 * 2;
		case ShaderDataType::Int3:		return 4 * 3;
		case ShaderDataType::Int4:		return 4 * 4;
		case ShaderDataType::Bool:		return 4;
		case ShaderDataType::Bool2:		return 4 * 2;
		case ShaderDataType::Bool3:		return 4 * 3;
		case ShaderDataType::Bool4:		return 4 * 4;
		}
		return 0;
	}

	unsigned int GetComponentCount() const
	{
		switch (type)
		{
		case ShaderDataType::Float:		return 1;
		case ShaderDataType::Float2:	return 2;
		case ShaderDataType::Float3:	return 3;
		case ShaderDataType::Float4:	return 4;
		case ShaderDataType::Mat2:		return 2;
		case ShaderDataType::Mat3:		return 3;
		case ShaderDataType::Mat4:		return 4;
		case ShaderDataType::Int:		return 1;
		case ShaderDataType::Int2:		return 2;
		case ShaderDataType::Int3:		return 3;
		case ShaderDataType::Int4:		return 4;
		case ShaderDataType::Bool:		return 1;
		case ShaderDataType::Bool2:		return 2;
		case ShaderDataType::Bool3:		return 3;
		case ShaderDataType::Bool4:		return 4;
		}
		return 0;
	}

	unsigned int GetTypeRows() const
	{
		switch (type)
		{
		case ShaderDataType::Float:		return 1;
		case ShaderDataType::Float2:	return 1;
		case ShaderDataType::Float3:	return 1;
		case ShaderDataType::Float4:	return 1;
		case ShaderDataType::Mat2:		return 2;
		case ShaderDataType::Mat3:		return 3;
		case ShaderDataType::Mat4:		return 4;
		case ShaderDataType::Int:		return 1;
		case ShaderDataType::Int2:		return 1;
		case ShaderDataType::Int3:		return 1;
		case ShaderDataType::Int4:		return 1;
		case ShaderDataType::Bool:		return 1;
		case ShaderDataType::Bool2:		return 1;
		case ShaderDataType::Bool3:		return 1;
		case ShaderDataType::Bool4:		return 1;
		}
		return 0;
	}

	LayoutLocation() = default;

	LayoutLocation(ShaderDataType type, const std::string& name, unsigned int instancesPerAttribute = 0, bool normalized = false)
		: name(name), type(type), size(ShaderDataTypeSize(type)), offset(0), normalized(normalized), instancesPerAttribute(instancesPerAttribute)
	{
	}
};

class BufferLayout
{
public:
	BufferLayout() {}

	BufferLayout(std::initializer_list<LayoutLocation> locationsIn)
		: locations(locationsIn)
	{
		CalculateOffsetsAndStride();
	}

	unsigned int GetStride() const { return stride; }
	unsigned int GetOffset() const { return offset; }
	const std::vector<LayoutLocation>& GetLocations() const { return locations; }

private:
	void CalculateOffsetsAndStride()
	{
		unsigned int Offset = 0;
		stride = 0;
		for (auto& location : locations)
		{
			location.offset = Offset;
			Offset += location.size;
			stride += location.size;
		}
	}
	std::vector<LayoutLocation> locations;
	unsigned int stride = 0;
	unsigned int offset = 0;
};

class Vao : public RenderElement
{
private:
	enum DrawFunction
	{
		none, drawElements, drawArrays, drawArraysInstanced, drawElementsInstanced
	};
public:
	enum PrimitiveMode
	{
		POINTS, LINE_STRIP, LINE_LOOP, LINES, LINE_STRIP_ADJACENCY, LINES_ADJACENCY, TRIANGLE_STRIP, TRIANGLE_FAN, TRIANGLES, TRIANGLE_STRIP_ADJACENCY, TRIANGLES_ADJACENCY
	};
	class DrawElement : public RenderElement
	{
	public:
		DrawElement() {}
		~DrawElement() {}
		void Execute() { vao->Draw(); }
		Vao* vao;
	private:

	};

	Vao();
	~Vao();
	void Bind();
	void Unbind();
	void SetPrimitiveMode(PrimitiveMode mode);
	DrawElement draw;
	unsigned int AddVertexBuffer(const void* data, unsigned int size, const BufferLayout& layout);
	void AddIndexBuffer(const void* indices, unsigned int count, IndicesType type);
	void ValidateAndCleanup();
	void Draw();
	unsigned int handle;
	std::vector<unsigned int> vertexBuffers;
	unsigned int indexBuffer;
	unsigned int indicesCount;
	
	unsigned int activeCount;
	DrawFunction drawFunction;
	bool instanced = false;
	void Execute();
	Vector3 center;
	Vector3 dimensions;
	unsigned int primitiveMode;
	unsigned int indicesType;
private:
	static Vao* currentVao;
	
	unsigned int ShaderDataTypeToOpenGLBaseType(ShaderDataType type);
	unsigned int PrimitiveModeToOpenGLBaseType(PrimitiveMode mode);
	unsigned int IndicesTypeToOpenGLBaseType(IndicesType type);
	unsigned int IndicesTypeSize(IndicesType type);

	unsigned int attributeIndex;
	unsigned int bindingIndex;
};

