#include "Vao.h"
#include <GL/glew.h>

Vao::Vao()
{
	primitiveMode = GL_TRIANGLES;
	indicesType = GL_UNSIGNED_INT;
	glCreateVertexArrays(1, &handle);
	instanced = false;
	indexBuffer = UINT_MAX;
	draw.vao = this;
	attributeIndex = 0;
	bindingIndex = 0;
}

Vao::~Vao()
{
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &handle);
	glDeleteBuffers(vertexBuffers.size(), &vertexBuffers[0]);
}

void Vao::Bind()
{
	if (currentVao != this)
	{
		glBindVertexArray(handle);
		currentVao = this;
	}
}

void Vao::Unbind()
{
	if (currentVao != nullptr)
	{
		glBindVertexArray(0);
		currentVao = nullptr;
	}
}

void Vao::SetPrimitiveMode(PrimitiveMode mode)
{
	primitiveMode = PrimitiveModeToOpenGLBaseType(mode);
}

unsigned int Vao::ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
{
	switch (type)
	{
	case ShaderDataType::Float:		return GL_FLOAT;
	case ShaderDataType::Float2:	return GL_FLOAT;
	case ShaderDataType::Float3:	return GL_FLOAT;
	case ShaderDataType::Float4:	return GL_FLOAT;
	case ShaderDataType::Mat2:		return GL_FLOAT;
	case ShaderDataType::Mat3:		return GL_FLOAT;
	case ShaderDataType::Mat4:		return GL_FLOAT;
	case ShaderDataType::Int:		return GL_INT;
	case ShaderDataType::Int2:		return GL_INT;
	case ShaderDataType::Int3:		return GL_INT;
	case ShaderDataType::Int4:		return GL_INT;
	case ShaderDataType::Bool:		return GL_BOOL;
	case ShaderDataType::Bool2:		return GL_BOOL;
	case ShaderDataType::Bool3:		return GL_BOOL;
	case ShaderDataType::Bool4:		return GL_BOOL;
	}
	return GL_FLOAT;
}

unsigned int Vao::PrimitiveModeToOpenGLBaseType(PrimitiveMode mode)
{
	switch (mode)
	{
	case PrimitiveMode::POINTS:						return GL_POINTS;
	case PrimitiveMode::LINE_STRIP:					return GL_LINE_STRIP;
	case PrimitiveMode::LINE_LOOP:					return GL_LINE_LOOP;
	case PrimitiveMode::LINES:						return GL_LINES;
	case PrimitiveMode::LINE_STRIP_ADJACENCY:		return GL_LINE_STRIP_ADJACENCY;
	case PrimitiveMode::LINES_ADJACENCY:			return GL_LINES_ADJACENCY;
	case PrimitiveMode::TRIANGLE_STRIP:				return GL_TRIANGLE_STRIP;
	case PrimitiveMode::TRIANGLE_FAN:				return GL_TRIANGLE_FAN;
	case PrimitiveMode::TRIANGLES:					return GL_TRIANGLES;
	case PrimitiveMode::TRIANGLE_STRIP_ADJACENCY:	return GL_TRIANGLE_STRIP_ADJACENCY;
	case PrimitiveMode::TRIANGLES_ADJACENCY:		return GL_TRIANGLES_ADJACENCY;
	}
	return GL_TRIANGLES;
}

unsigned int Vao::IndicesTypeToOpenGLBaseType(IndicesType type)
{
	switch (type)
	{
	case IndicesType::UNSIGNED_BYTE:	return GL_UNSIGNED_BYTE;
	case IndicesType::UNSIGNED_SHORT:	return GL_UNSIGNED_SHORT;
	case IndicesType::UNSIGNED_INT:		return GL_UNSIGNED_INT;
	}
	return 0;
}

unsigned int Vao::IndicesTypeSize(IndicesType type)
{
	switch (type)
	{
	case IndicesType::UNSIGNED_BYTE:	return sizeof(unsigned char);
	case IndicesType::UNSIGNED_SHORT:	return sizeof(unsigned short);
	case IndicesType::UNSIGNED_INT:		return sizeof(unsigned int);
	}
	return 0;
}

unsigned int Vao::AddVertexBuffer(const void* data, unsigned int size, const BufferLayout& layout)
{
	if (layout.GetLocations().size() == 0)
		return UINT_MAX;

	GLuint vertexBuffer;
	glCreateBuffers(1, &vertexBuffer);
	glNamedBufferStorage(vertexBuffer, size, data, GL_DYNAMIC_STORAGE_BIT);

	glVertexArrayVertexBuffer(handle, bindingIndex, vertexBuffer, layout.GetOffset(), layout.GetStride()); //vao handle, binding index, vbo handle, offset to first element, stride (distance between elements)
	for (const auto& location : layout.GetLocations())
	{
		unsigned int count = location.GetComponentCount();
		unsigned int type = ShaderDataTypeToOpenGLBaseType(location.type);
		unsigned int rows = location.GetTypeRows();
		for (unsigned int i = 0; i < rows; i++)
		{
			glEnableVertexArrayAttrib(handle, attributeIndex); //vao handle, attribute index, which attrib index to enable on this vao
			glVertexArrayAttribFormat(handle, attributeIndex, count, type, location.normalized ? GL_TRUE : GL_FALSE, location.offset + sizeof(float) * count * i); //vao handle, attribute index, values per element, type of data, normalized, relativeoffset - The distance between elements within the buffer.
			glVertexArrayAttribBinding(handle, attributeIndex, bindingIndex); //vao handle, attribute index, binding index
			glVertexArrayBindingDivisor(handle, bindingIndex, location.instancesPerAttribute);
			if (location.instancesPerAttribute > 0)
			{
				instanced = true;
			}
			attributeIndex++;
		}
	}

	vertexBuffers.push_back(vertexBuffer);
	bindingIndex++;
	return vertexBuffer;
}

void Vao::AddIndexBuffer(const void * indices, unsigned int count, IndicesType type)
{
	GLuint elementbuffer;
	glCreateBuffers(1, &elementbuffer);
	glNamedBufferStorage(elementbuffer, count * IndicesTypeSize(type), indices, GL_DYNAMIC_STORAGE_BIT);
	glVertexArrayElementBuffer(handle, elementbuffer);
	indicesCount = count;
	vertexBuffers.push_back(elementbuffer);
	indexBuffer = elementbuffer;
	indicesType = IndicesTypeToOpenGLBaseType(type);
}

void Vao::ValidateAndCleanup()
{
	drawFunction = vertexBuffers.size() > 0 ? instanced ? (indexBuffer != UINT_MAX ? drawElementsInstanced : drawArraysInstanced) : (indexBuffer != UINT_MAX ? drawElements : drawArrays) : none;
}

void Vao::Draw()
{
	ValidateAndCleanup();
	switch (drawFunction)
	{
	case drawElements:
		glDrawElements(primitiveMode, indicesCount, indicesType, (void*)0);
		break;
	case drawArrays:
		glDrawArrays(primitiveMode, 0, activeCount);
		break;
	case drawArraysInstanced:
		glDrawArraysInstanced(primitiveMode, 0, indicesCount, activeCount);
		break;
	case drawElementsInstanced:
		glDrawElementsInstanced(primitiveMode, indicesCount, indicesType, (void*)0, activeCount);
		break;
	default:
		break;
	}
}

void Vao::Execute()
{
	Bind();
}

Vao* Vao::currentVao = nullptr;