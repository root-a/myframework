#include "Vao.h"
#include "Vbo.h"
#include "Ebo.h"
#include <GL/glew.h>

VertexArray::VertexArray()
{
	primitiveMode = GL_TRIANGLES;
	activeCount = 0;
	glCreateVertexArrays(1, &handle);
	instanced = false;
	ebo = nullptr;
	draw.vao = this;
	attributeIndex = 0;
	bindingIndex = 0;
	bufferIsDynamic = false;
}

VertexArray::~VertexArray()
{
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &handle);
}

void VertexArray::Bind()
{
	//if (currentVao != this)
	{
		glBindVertexArray(handle);
		currentVao = this;
	}
}

void VertexArray::Unbind()
{
	if (currentVao != nullptr)
	{
		glBindVertexArray(0);
		currentVao = nullptr;
	}
}

void VertexArray::SetPrimitiveMode(PrimitiveMode mode)
{
	primitiveMode = PrimitiveModeToOpenGLBaseType(mode);
}

PrimitiveMode VertexArray::GetPrimitiveMode()
{
	switch (primitiveMode)
	{
	case GL_POINTS:						return PrimitiveMode::POINTS;
	case GL_LINE_STRIP:					return PrimitiveMode::LINE_STRIP;
	case GL_LINE_LOOP:					return PrimitiveMode::LINE_LOOP;
	case GL_LINES:						return PrimitiveMode::LINES;
	case GL_LINE_STRIP_ADJACENCY:		return PrimitiveMode::LINE_STRIP_ADJACENCY;
	case GL_LINES_ADJACENCY:			return PrimitiveMode::LINES_ADJACENCY;
	case GL_TRIANGLE_STRIP:				return PrimitiveMode::TRIANGLE_STRIP;
	case GL_TRIANGLE_FAN:				return PrimitiveMode::TRIANGLE_FAN;
	case GL_TRIANGLES:					return PrimitiveMode::TRIANGLES;
	case GL_TRIANGLE_STRIP_ADJACENCY:	return PrimitiveMode::TRIANGLE_STRIP_ADJACENCY;
	case GL_TRIANGLES_ADJACENCY:		return PrimitiveMode::TRIANGLES_ADJACENCY;
	}
	return PrimitiveMode::TRIANGLES;
}

const char* VertexArray::GetPrimitiveModeAsStr()
{
	switch (primitiveMode)
	{
	case GL_POINTS:						return "POINTS";
	case GL_LINE_STRIP:					return "LINE_STRIP";
	case GL_LINE_LOOP:					return "LINE_LOOP";
	case GL_LINES:						return "LINES";
	case GL_LINE_STRIP_ADJACENCY:		return "LINE_STRIP_ADJACENCY";
	case GL_LINES_ADJACENCY:			return "LINES_ADJACENCY";
	case GL_TRIANGLE_STRIP:				return "TRIANGLE_STRIP";
	case GL_TRIANGLE_FAN:				return "TRIANGLE_FAN";
	case GL_TRIANGLES:					return "TRIANGLES";
	case GL_TRIANGLE_STRIP_ADJACENCY:	return "TRIANGLE_STRIP_ADJACENCY";
	case GL_TRIANGLES_ADJACENCY:		return "TRIANGLES_ADJACENCY";
	}
	return "TRIANGLES";
}

unsigned int VertexArray::PrimitiveModeToOpenGLBaseType(PrimitiveMode mode)
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

void VertexArray::RegisterVertexBuffer(VertexBuffer* vbo)
{
	glVertexArrayVertexBuffer(handle, bindingIndex, vbo->handle, vbo->layout.GetOffset(), vbo->layout.GetStride()); //vao handle, binding index, vbo handle, offset to first element, stride (distance between elements)
	instanced = bufferIsDynamic = vbo->layout.isDynamic;
	for (const auto& location : vbo->layout.locations)
	{
		unsigned int count = ShaderDataType::ComponentCount(location.type);
		unsigned int type = ShaderDataType::ToOpenGLBaseType(location.type);
		unsigned int typeSize = ShaderDataType::BaseTypeSize(location.type);
		unsigned int rows = ShaderDataType::Rows(location.type);
		for (unsigned int i = 0; i < rows; i++)
		{
			glEnableVertexArrayAttrib(handle, attributeIndex); //vao handle, attribute index, which attrib index to enable on this vao
			glVertexArrayAttribFormat(handle, attributeIndex, count, type, location.normalized ? GL_TRUE : GL_FALSE, location.offset + typeSize * count * i); //vao handle, attribute index, values per element, type of data, normalized, relativeoffset - The distance between elements within the buffer.
			glVertexArrayAttribBinding(handle, attributeIndex, bindingIndex); //vao handle, attribute index, binding index
			glVertexArrayBindingDivisor(handle, bindingIndex, location.instancesPerAttribute);
			attributeIndex++;
		}
	}
}

unsigned int VertexArray::AddVertexBuffer(VertexBuffer* vbo)
{
	vbos.push_back(vbo);
	attributeIndexes.push_back(attributeIndex);
	RegisterVertexBuffer(vbo);
	if (instanced)
	{
		dynamicVBOs.push_back((VertexBufferDynamic*)vbo);
		auto vbo = dynamicVBOs.back();
		auto res = std::find(vbo->vaos.begin(), vbo->vaos.end(), this);
		if (res == vbo->vaos.end())
			vbo->vaos.push_back(this);		
	}
	bindingIndexes.push_back(bindingIndex);
	bindingIndex++;
	UpdateDrawFunction();
	return vbo->handle;
}

unsigned int VertexArray::ReAddVertexBuffer(VertexBuffer* vbo)
{
	int index = -1;
	for (size_t i = 0; i < vbos.size(); i++)
	{
		if (vbos[i] == vbo)
		{
			index = i;
			break;
		}
	}
	if (index != -1)
	{
		unsigned int oldAttributeIndex = attributeIndex;
		unsigned int oldBindingIndex = bindingIndex;
		attributeIndex = attributeIndexes[index];
		bindingIndex = bindingIndexes[index];
		RegisterVertexBuffer(vbo);
		UpdateDrawFunction();
		attributeIndex = oldAttributeIndex;
		bindingIndex = oldBindingIndex;
	}
	return vbo->handle;
}

void VertexArray::AddElementBuffer(ElementBuffer* newEbo)
{
	ebo = newEbo;
	glVertexArrayElementBuffer(handle, ebo->handle);
	UpdateDrawFunction();
}

unsigned int VertexArray::ReAddElementBuffer(ElementBuffer* newEbo)
{
	ebo = newEbo;
	glVertexArrayElementBuffer(handle, ebo->handle);
	UpdateDrawFunction();
	return ebo->handle;
}

void VertexArray::UpdateDrawFunction()
{
	drawFunction = vbos.size() > 0 ? instanced ? (ebo != nullptr ? DrawFunction::drawElementsInstanced : DrawFunction::drawArraysInstanced) : (ebo != nullptr ? DrawFunction::drawElements : DrawFunction::drawArrays) : DrawFunction::none;
}

void VertexArray::ResizeVertexBuffer(VertexBuffer* bufferToResize, void* newData, unsigned int newElementCount)
{
	int index = -1;
	for (size_t i = 0; i < vbos.size(); i++)
	{
		if (vbos[i] == bufferToResize)
		{
			index = i;
			break;
		}
	}
	if (index != -1)
	{
		bufferToResize->Resize(newData, newElementCount);

		ReAddVertexBuffer(bufferToResize);

		if (bufferToResize->layout.isDynamic)
		{
			activeCount = 0;
		}
	}
}

void VertexArray::ResizeElementBuffer(ElementBuffer* bufferToResize, void* newData, unsigned int newElementCount)
{
	if (ebo == bufferToResize)
	{
		bufferToResize->Resize(newData, newElementCount);
		ReAddElementBuffer(bufferToResize);
	}
}

void VertexArray::Draw()
{
	switch (drawFunction)
	{
	case DrawFunction::drawElements:
		glDrawElements(primitiveMode, ebo->indicesCount, ebo->indicesType, (void*)0);
		break;
	case DrawFunction::drawArrays:
		glDrawArrays(primitiveMode, 0, activeCount);
		break;
	case DrawFunction::drawArraysInstanced:
		activeCount = vbos.size() > 0 ? vbos[0]->maxElementCount : 0;
		glDrawArraysInstanced(primitiveMode, 0, vertexCount, activeCount); // indicesCount is nr of elements that makes up a mesh bound in non dynamic buffer, so basically vertex count
		break;
	case DrawFunction::drawElementsInstanced:
		activeCount = dynamicVBOs.size() > 0 ? dynamicVBOs[0]->activeCount : activeCount;
		glDrawElementsInstanced(primitiveMode, ebo->indicesCount, ebo->indicesType, (void*)0, activeCount);
		for (auto& vbod : dynamicVBOs)
		{
			vbod->activeCount = 0;
		}
		activeCount = 0;
		break;
	default:
		break;
	}
}

void VertexArray::Execute()
{
	Bind();
}

VertexArray* VertexArray::currentVao = nullptr;
