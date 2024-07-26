#include "ShaderDataType.h"

#include <GL/glew.h>

namespace ShaderDataType
{
	Type FromStr(const char* type)
	{
		if (strcmp(type, "Float") == 0) return Type::Float;
		if (strcmp(type, "Float2") == 0) return Type::Float2;
		if (strcmp(type, "Float3") == 0) return Type::Float3;
		if (strcmp(type, "Float4") == 0) return Type::Float4;
		if (strcmp(type, "Double") == 0) return Type::Double;
		if (strcmp(type, "Double2") == 0) return Type::Double2;
		if (strcmp(type, "Double3") == 0) return Type::Double3;
		if (strcmp(type, "Double4") == 0) return Type::Double4;
		if (strcmp(type, "Int") == 0) return Type::Int;
		if (strcmp(type, "Int2") == 0) return Type::Int2;
		if (strcmp(type, "Int3") == 0) return Type::Int3;
		if (strcmp(type, "Int4") == 0) return Type::Int4;
		if (strcmp(type, "UInt") == 0) return Type::UInt;
		if (strcmp(type, "UInt2") == 0) return Type::UInt2;
		if (strcmp(type, "UInt3") == 0) return Type::UInt3;
		if (strcmp(type, "UInt4") == 0) return Type::UInt4;
		if (strcmp(type, "Bool") == 0) return Type::Bool;
		if (strcmp(type, "Bool2") == 0) return Type::Bool2;
		if (strcmp(type, "Bool3") == 0) return Type::Bool3;
		if (strcmp(type, "Bool4") == 0) return Type::Bool4;
		if (strcmp(type, "FloatMat2") == 0) return Type::FloatMat2;
		if (strcmp(type, "FloatMat3") == 0) return Type::FloatMat3;
		if (strcmp(type, "FloatMat4") == 0) return Type::FloatMat4;
		if (strcmp(type, "FloatMat2x3") == 0) return Type::FloatMat2x3;
		if (strcmp(type, "FloatMat2x4") == 0) return Type::FloatMat2x4;
		if (strcmp(type, "FloatMat3x2") == 0) return Type::FloatMat3x2;
		if (strcmp(type, "FloatMat3x4") == 0) return Type::FloatMat3x4;
		if (strcmp(type, "FloatMat4x2") == 0) return Type::FloatMat4x2;
		if (strcmp(type, "FloatMat4x3") == 0) return Type::FloatMat4x3;
		if (strcmp(type, "DoubleMat2") == 0) return Type::DoubleMat2;
		if (strcmp(type, "DoubleMat3") == 0) return Type::DoubleMat3;
		if (strcmp(type, "DoubleMat4") == 0) return Type::DoubleMat4;
		if (strcmp(type, "DoubleMat2x3") == 0) return Type::DoubleMat2x3;
		if (strcmp(type, "DoubleMat2x4") == 0) return Type::DoubleMat2x4;
		if (strcmp(type, "DoubleMat3x2") == 0) return Type::DoubleMat3x2;
		if (strcmp(type, "DoubleMat3x4") == 0) return Type::DoubleMat3x4;
		if (strcmp(type, "DoubleMat4x2") == 0) return Type::DoubleMat4x2;
		if (strcmp(type, "DoubleMat4x3") == 0) return Type::DoubleMat4x3;
		if (strcmp(type, "IntMat2") == 0) return Type::IntMat2;
		if (strcmp(type, "IntMat3") == 0) return Type::IntMat3;
		if (strcmp(type, "IntMat4") == 0) return Type::IntMat4;
		if (strcmp(type, "IntMat2x3") == 0) return Type::IntMat2x3;
		if (strcmp(type, "IntMat2x4") == 0) return Type::IntMat2x4;
		if (strcmp(type, "IntMat3x2") == 0) return Type::IntMat3x2;
		if (strcmp(type, "IntMat3x4") == 0) return Type::IntMat3x4;
		if (strcmp(type, "IntMat4x2") == 0) return Type::IntMat4x2;
		if (strcmp(type, "IntMat4x3") == 0) return Type::IntMat4x3;
		if (strcmp(type, "BoolMat2") == 0) return Type::BoolMat2;
		if (strcmp(type, "BoolMat3") == 0) return Type::BoolMat3;
		if (strcmp(type, "BoolMat4") == 0) return Type::BoolMat4;
		if (strcmp(type, "BoolMat2x3") == 0) return Type::BoolMat2x3;
		if (strcmp(type, "BoolMat2x4") == 0) return Type::BoolMat2x4;
		if (strcmp(type, "BoolMat3x2") == 0) return Type::BoolMat3x2;
		if (strcmp(type, "BoolMat3x4") == 0) return Type::BoolMat3x4;
		if (strcmp(type, "BoolMat4x2") == 0) return Type::BoolMat4x2;
		if (strcmp(type, "BoolMat4x3") == 0) return Type::BoolMat4x3;
	}

	Type FromGLStr(const char* type)
	{
		if (strcmp(type, "float") == 0) return Type::Float;
		if (strcmp(type, "vec2") == 0) return Type::Float2;
		if (strcmp(type, "vec3") == 0) return Type::Float3;
		if (strcmp(type, "vec4") == 0) return Type::Float4;
		if (strcmp(type, "double") == 0) return Type::Double;
		if (strcmp(type, "dvec2") == 0) return Type::Double2;
		if (strcmp(type, "dvec3") == 0) return Type::Double3;
		if (strcmp(type, "dvec4") == 0) return Type::Double4;
		if (strcmp(type, "int") == 0) return Type::Int;
		if (strcmp(type, "ivec2") == 0) return Type::Int2;
		if (strcmp(type, "ivec3") == 0) return Type::Int3;
		if (strcmp(type, "ivec4") == 0) return Type::Int4;
		if (strcmp(type, "uint") == 0) return Type::UInt;
		if (strcmp(type, "uvec2") == 0) return Type::UInt2;
		if (strcmp(type, "uvec3") == 0) return Type::UInt3;
		if (strcmp(type, "uvec4") == 0) return Type::UInt4;
		if (strcmp(type, "bool") == 0) return Type::Bool;
		if (strcmp(type, "bvec2") == 0) return Type::Bool2;
		if (strcmp(type, "bvec3") == 0) return Type::Bool3;
		if (strcmp(type, "bvec4") == 0) return Type::Bool4;
		if (strcmp(type, "mat2") == 0) return Type::FloatMat2;
		if (strcmp(type, "mat3") == 0) return Type::FloatMat3;
		if (strcmp(type, "mat4") == 0) return Type::FloatMat4;
		if (strcmp(type, "mat2x3") == 0) return Type::FloatMat2x3;
		if (strcmp(type, "mat2x4") == 0) return Type::FloatMat2x4;
		if (strcmp(type, "mat3x2") == 0) return Type::FloatMat3x2;
		if (strcmp(type, "mat3x4") == 0) return Type::FloatMat3x4;
		if (strcmp(type, "mat4x2") == 0) return Type::FloatMat4x2;
		if (strcmp(type, "mat4x3") == 0) return Type::FloatMat4x3;
		if (strcmp(type, "dmat2") == 0) return Type::DoubleMat2;
		if (strcmp(type, "dmat3") == 0) return Type::DoubleMat3;
		if (strcmp(type, "dmat4") == 0) return Type::DoubleMat4;
		if (strcmp(type, "dmat2x3") == 0) return Type::DoubleMat2x3;
		if (strcmp(type, "dmat2x4") == 0) return Type::DoubleMat2x4;
		if (strcmp(type, "dmat3x2") == 0) return Type::DoubleMat3x2;
		if (strcmp(type, "dmat3x4") == 0) return Type::DoubleMat3x4;
		if (strcmp(type, "dmat4x2") == 0) return Type::DoubleMat4x2;
		if (strcmp(type, "dmat4x3") == 0) return Type::DoubleMat4x3;
		if (strcmp(type, "imat2") == 0) return Type::IntMat2;
		if (strcmp(type, "imat3") == 0) return Type::IntMat3;
		if (strcmp(type, "imat4") == 0) return Type::IntMat4;
		if (strcmp(type, "imat2x3") == 0) return Type::IntMat2x3;
		if (strcmp(type, "imat2x4") == 0) return Type::IntMat2x4;
		if (strcmp(type, "imat3x2") == 0) return Type::IntMat3x2;
		if (strcmp(type, "imat3x4") == 0) return Type::IntMat3x4;
		if (strcmp(type, "imat4x2") == 0) return Type::IntMat4x2;
		if (strcmp(type, "imat4x3") == 0) return Type::IntMat4x3;
		if (strcmp(type, "bmat2") == 0) return Type::BoolMat2;
		if (strcmp(type, "bmat3") == 0) return Type::BoolMat3;
		if (strcmp(type, "bmat4") == 0) return Type::BoolMat4;
		if (strcmp(type, "bmat2x3") == 0) return Type::BoolMat2x3;
		if (strcmp(type, "bmat2x4") == 0) return Type::BoolMat2x4;
		if (strcmp(type, "bmat3x2") == 0) return Type::BoolMat3x2;
		if (strcmp(type, "bmat3x4") == 0) return Type::BoolMat3x4;
		if (strcmp(type, "bmat4x2") == 0) return Type::BoolMat4x2;
		if (strcmp(type, "bmat4x3") == 0) return Type::BoolMat4x3;
		return Type::Float;
	}

	std::string_view Str(Type type)
	{
		switch (type)
		{
		case Type::Float:			return "Float";
		case Type::Float2:		return "Float2";
		case Type::Float3:		return "Float3";
		case Type::Float4:		return "Float4";
		case Type::Double:		return "Double";
		case Type::Double2:		return "Double2";
		case Type::Double3:		return "Double3";
		case Type::Double4:		return "Double4";
		case Type::Int:			return "Int";
		case Type::Int2:			return "Int2";
		case Type::Int3:			return "Int3";
		case Type::Int4:			return "Int4";
		case Type::UInt:			return "UInt";
		case Type::UInt2:			return "UInt2";
		case Type::UInt3:			return "UInt3";
		case Type::UInt4:			return "UInt4";
		case Type::Bool:			return "Bool";
		case Type::Bool2:			return "Bool2";
		case Type::Bool3:			return "Bool3";
		case Type::Bool4:			return "Bool4";
		case Type::FloatMat2:		return "FloatMat2";
		case Type::FloatMat3:		return "FloatMat3";
		case Type::FloatMat4:		return "FloatMat4";
		case Type::FloatMat2x3:	return "FloatMat2x3";
		case Type::FloatMat2x4:	return "FloatMat2x4";
		case Type::FloatMat3x2:	return "FloatMat3x2";
		case Type::FloatMat3x4:	return "FloatMat3x4";
		case Type::FloatMat4x2:	return "FloatMat4x2";
		case Type::FloatMat4x3:	return "FloatMat4x3";
		case Type::DoubleMat2:	return "DoubleMat2";
		case Type::DoubleMat3:	return "DoubleMat3";
		case Type::DoubleMat4:	return "DoubleMat4";
		case Type::DoubleMat2x3:	return "DoubleMat2x3";
		case Type::DoubleMat2x4:	return "DoubleMat2x4";
		case Type::DoubleMat3x2:	return "DoubleMat3x2";
		case Type::DoubleMat3x4:	return "DoubleMat3x4";
		case Type::DoubleMat4x2:	return "DoubleMat4x2";
		case Type::DoubleMat4x3:	return "DoubleMat4x3";
		case Type::IntMat2:		return "IntMat2";
		case Type::IntMat3:		return "IntMat3";
		case Type::IntMat4:		return "IntMat4";
		case Type::IntMat2x3:		return "IntMat2x3";
		case Type::IntMat2x4:		return "IntMat2x4";
		case Type::IntMat3x2:		return "IntMat3x2";
		case Type::IntMat3x4:		return "IntMat3x4";
		case Type::IntMat4x2:		return "IntMat4x2";
		case Type::IntMat4x3:		return "IntMat4x3";
		case Type::BoolMat2:		return "BoolMat2";
		case Type::BoolMat3:		return "BoolMat3";
		case Type::BoolMat4:		return "BoolMat4";
		case Type::BoolMat2x3:	return "BoolMat2x3";
		case Type::BoolMat2x4:	return "BoolMat2x4";
		case Type::BoolMat3x2:	return "BoolMat3x2";
		case Type::BoolMat3x4:	return "BoolMat3x4";
		case Type::BoolMat4x2:	return "BoolMat4x2";
		case Type::BoolMat4x3:	return "BoolMat4x3";
		}
		return "Float";
	}

	unsigned int Size(Type type)
	{
		switch (type)
		{
		case Type::Float:			return 4;
		case Type::Float2:		return 4 * 2;
		case Type::Float3:		return 4 * 3;
		case Type::Float4:		return 4 * 4;
		case Type::Double:		return 8;
		case Type::Double2:		return 8 * 2;
		case Type::Double3:		return 8 * 3;
		case Type::Double4:		return 8 * 4;
		case Type::Int:			return 4;
		case Type::Int2:			return 4 * 2;
		case Type::Int3:			return 4 * 3;
		case Type::Int4:			return 4 * 4;
		case Type::UInt:			return 4;
		case Type::UInt2:			return 4 * 2;
		case Type::UInt3:			return 4 * 3;
		case Type::UInt4:			return 4 * 4;
		case Type::Bool:			return 4;
		case Type::Bool2:			return 4 * 2;
		case Type::Bool3:			return 4 * 3;
		case Type::Bool4:			return 4 * 4;
		case Type::FloatMat2:		return 4 * 2 * 2;
		case Type::FloatMat3:		return 4 * 3 * 3;
		case Type::FloatMat4:		return 4 * 4 * 4;
		case Type::FloatMat2x3:	return 4 * 2 * 3;
		case Type::FloatMat2x4:	return 4 * 2 * 4;
		case Type::FloatMat3x2:	return 4 * 3 * 2;
		case Type::FloatMat3x4:	return 4 * 3 * 4;
		case Type::FloatMat4x2:	return 4 * 4 * 2;
		case Type::FloatMat4x3:	return 4 * 4 * 3;
		case Type::DoubleMat2:	return 8 * 2 * 2;
		case Type::DoubleMat3:	return 8 * 3 * 3;
		case Type::DoubleMat4:	return 8 * 4 * 4;
		case Type::DoubleMat2x3:	return 8 * 2 * 3;
		case Type::DoubleMat2x4:	return 8 * 2 * 4;
		case Type::DoubleMat3x2:	return 8 * 3 * 2;
		case Type::DoubleMat3x4:	return 8 * 3 * 4;
		case Type::DoubleMat4x2:	return 8 * 4 * 2;
		case Type::DoubleMat4x3:	return 8 * 4 * 3;
		case Type::IntMat2:		return 4 * 2 * 2;
		case Type::IntMat3:		return 4 * 3 * 3;
		case Type::IntMat4:		return 4 * 4 * 4;
		case Type::IntMat2x3:		return 4 * 2 * 3;
		case Type::IntMat2x4:		return 4 * 2 * 4;
		case Type::IntMat3x2:		return 4 * 3 * 2;
		case Type::IntMat3x4:		return 4 * 3 * 4;
		case Type::IntMat4x2:		return 4 * 4 * 2;
		case Type::IntMat4x3:		return 4 * 4 * 3;
		case Type::BoolMat2:		return 4 * 2 * 2;
		case Type::BoolMat3:		return 4 * 3 * 3;
		case Type::BoolMat4:		return 4 * 4 * 4;
		case Type::BoolMat2x3:	return 4 * 2 * 3;
		case Type::BoolMat2x4:	return 4 * 2 * 4;
		case Type::BoolMat3x2:	return 4 * 3 * 2;
		case Type::BoolMat3x4:	return 4 * 3 * 4;
		case Type::BoolMat4x2:	return 4 * 4 * 2;
		case Type::BoolMat4x3:	return 4 * 4 * 3;
		}
		return 4;
	}

	unsigned int ComponentCount(Type type)
	{
		switch (type)
		{
		case Type::Float:			return 1;
		case Type::Float2:		return 2;
		case Type::Float3:		return 3;
		case Type::Float4:		return 4;
		case Type::Double:		return 1;
		case Type::Double2:		return 2;
		case Type::Double3:		return 3;
		case Type::Double4:		return 4;
		case Type::Int:			return 1;
		case Type::Int2:			return 2;
		case Type::Int3:			return 3;
		case Type::Int4:			return 4;
		case Type::UInt:			return 1;
		case Type::UInt2:			return 2;
		case Type::UInt3:			return 3;
		case Type::UInt4:			return 4;
		case Type::Bool:			return 1;
		case Type::Bool2:			return 2;
		case Type::Bool3:			return 3;
		case Type::Bool4:			return 4;
		case Type::FloatMat2:		return 2;
		case Type::FloatMat3:		return 3;
		case Type::FloatMat4:		return 4;
		case Type::FloatMat2x3:	return 3;
		case Type::FloatMat2x4:	return 4;
		case Type::FloatMat3x2:	return 2;
		case Type::FloatMat3x4:	return 4;
		case Type::FloatMat4x2:	return 2;
		case Type::FloatMat4x3:	return 3;
		case Type::DoubleMat2:	return 2;
		case Type::DoubleMat3:	return 3;
		case Type::DoubleMat4:	return 4;
		case Type::DoubleMat2x3:	return 3;
		case Type::DoubleMat2x4:	return 4;
		case Type::DoubleMat3x2:	return 2;
		case Type::DoubleMat3x4:	return 4;
		case Type::DoubleMat4x2:	return 2;
		case Type::DoubleMat4x3:	return 3;
		case Type::IntMat2:		return 2;
		case Type::IntMat3:		return 3;
		case Type::IntMat4:		return 4;
		case Type::IntMat2x3:		return 3;
		case Type::IntMat2x4:		return 4;
		case Type::IntMat3x2:		return 2;
		case Type::IntMat3x4:		return 4;
		case Type::IntMat4x2:		return 2;
		case Type::IntMat4x3:		return 3;
		case Type::BoolMat2:		return 2;
		case Type::BoolMat3:		return 3;
		case Type::BoolMat4:		return 4;
		case Type::BoolMat2x3:	return 3;
		case Type::BoolMat2x4:	return 4;
		case Type::BoolMat3x2:	return 2;
		case Type::BoolMat3x4:	return 4;
		case Type::BoolMat4x2:	return 2;
		case Type::BoolMat4x3:	return 3;
		}
		return 1;
	}

	unsigned int Rows(Type type)
	{
		switch (type)
		{
		case Type::Float:			return 1;
		case Type::Float2:		return 1;
		case Type::Float3:		return 1;
		case Type::Float4:		return 1;
		case Type::Double:		return 1;
		case Type::Double2:		return 1;
		case Type::Double3:		return 1;
		case Type::Double4:		return 1;
		case Type::Int:			return 1;
		case Type::Int2:			return 1;
		case Type::Int3:			return 1;
		case Type::Int4:			return 1;
		case Type::UInt:			return 1;
		case Type::UInt2:			return 1;
		case Type::UInt3:			return 1;
		case Type::UInt4:			return 1;
		case Type::Bool:			return 1;
		case Type::Bool2:			return 1;
		case Type::Bool3:			return 1;
		case Type::Bool4:			return 1;
		case Type::FloatMat2:		return 2;
		case Type::FloatMat3:		return 3;
		case Type::FloatMat4:		return 4;
		case Type::FloatMat2x3:	return 2;
		case Type::FloatMat2x4:	return 2;
		case Type::FloatMat3x2:	return 3;
		case Type::FloatMat3x4:	return 3;
		case Type::FloatMat4x2:	return 4;
		case Type::FloatMat4x3:	return 4;
		case Type::DoubleMat2:	return 2;
		case Type::DoubleMat3:	return 3;
		case Type::DoubleMat4:	return 4;
		case Type::DoubleMat2x3:	return 2;
		case Type::DoubleMat2x4:	return 2;
		case Type::DoubleMat3x2:	return 3;
		case Type::DoubleMat3x4:	return 3;
		case Type::DoubleMat4x2:	return 4;
		case Type::DoubleMat4x3:	return 4;
		case Type::IntMat2:		return 2;
		case Type::IntMat3:		return 3;
		case Type::IntMat4:		return 4;
		case Type::IntMat2x3:		return 2;
		case Type::IntMat2x4:		return 2;
		case Type::IntMat3x2:		return 3;
		case Type::IntMat3x4:		return 3;
		case Type::IntMat4x2:		return 4;
		case Type::IntMat4x3:		return 4;
		case Type::BoolMat2:		return 2;
		case Type::BoolMat3:		return 3;
		case Type::BoolMat4:		return 4;
		case Type::BoolMat2x3:	return 2;
		case Type::BoolMat2x4:	return 2;
		case Type::BoolMat3x2:	return 3;
		case Type::BoolMat3x4:	return 3;
		case Type::BoolMat4x2:	return 4;
		case Type::BoolMat4x3:	return 4;
		}
		return 1;
	}

	unsigned int ToOpenGLBaseType(Type type)
	{
		switch (type)
		{
		case Type::Float:			return GL_FLOAT;
		case Type::Float2:		return GL_FLOAT;
		case Type::Float3:		return GL_FLOAT;
		case Type::Float4:		return GL_FLOAT;
		case Type::Double:		return GL_DOUBLE;
		case Type::Double2:		return GL_DOUBLE;
		case Type::Double3:		return GL_DOUBLE;
		case Type::Double4:		return GL_DOUBLE;
		case Type::Int:			return GL_INT;
		case Type::Int2:			return GL_INT;
		case Type::Int3:			return GL_INT;
		case Type::Int4:			return GL_INT;
		case Type::UInt:			return GL_UNSIGNED_INT;
		case Type::UInt2:			return GL_UNSIGNED_INT;
		case Type::UInt3:			return GL_UNSIGNED_INT;
		case Type::UInt4:			return GL_UNSIGNED_INT;
		case Type::Bool:			return GL_BOOL;
		case Type::Bool2:			return GL_BOOL;
		case Type::Bool3:			return GL_BOOL;
		case Type::Bool4:			return GL_BOOL;
		case Type::FloatMat2:		return GL_FLOAT;
		case Type::FloatMat3:		return GL_FLOAT;
		case Type::FloatMat4:		return GL_FLOAT;
		case Type::FloatMat2x3:	return GL_FLOAT;
		case Type::FloatMat2x4:	return GL_FLOAT;
		case Type::FloatMat3x2:	return GL_FLOAT;
		case Type::FloatMat3x4:	return GL_FLOAT;
		case Type::FloatMat4x2:	return GL_FLOAT;
		case Type::FloatMat4x3:	return GL_FLOAT;
		case Type::DoubleMat2:	return GL_DOUBLE;
		case Type::DoubleMat3:	return GL_DOUBLE;
		case Type::DoubleMat4:	return GL_DOUBLE;
		case Type::DoubleMat2x3:	return GL_DOUBLE;
		case Type::DoubleMat2x4:	return GL_DOUBLE;
		case Type::DoubleMat3x2:	return GL_DOUBLE;
		case Type::DoubleMat3x4:	return GL_DOUBLE;
		case Type::DoubleMat4x2:	return GL_DOUBLE;
		case Type::DoubleMat4x3:	return GL_DOUBLE;
		case Type::IntMat2:		return GL_INT;
		case Type::IntMat3:		return GL_INT;
		case Type::IntMat4:		return GL_INT;
		case Type::IntMat2x3:		return GL_INT;
		case Type::IntMat2x4:		return GL_INT;
		case Type::IntMat3x2:		return GL_INT;
		case Type::IntMat3x4:		return GL_INT;
		case Type::IntMat4x2:		return GL_INT;
		case Type::IntMat4x3:		return GL_INT;
		case Type::BoolMat2:		return GL_BOOL;
		case Type::BoolMat3:		return GL_BOOL;
		case Type::BoolMat4:		return GL_BOOL;
		case Type::BoolMat2x3:	return GL_BOOL;
		case Type::BoolMat2x4:	return GL_BOOL;
		case Type::BoolMat3x2:	return GL_BOOL;
		case Type::BoolMat3x4:	return GL_BOOL;
		case Type::BoolMat4x2:	return GL_BOOL;
		case Type::BoolMat4x3:	return GL_BOOL;
		}
		return GL_FLOAT;
	}

	unsigned int BaseTypeSize(Type type)
	{
		switch (type)
		{
		case Type::Float:			return 4;
		case Type::Double:			return 8;
		case Type::Int:				return 4;
		case Type::UInt:			return 4;
		case Type::Bool:			return 1;
		}
		return 4;
	}

	ShaderDataType::Type FromOpenGLType(unsigned int type)
	{
		switch (type)
		{
		case GL_FLOAT:				return Type::Float;
		case GL_FLOAT_VEC2:			return Type::Float2;
		case GL_FLOAT_VEC3:			return Type::Float3;
		case GL_FLOAT_VEC4:			return Type::Float4;
		case GL_DOUBLE:				return Type::Double;
		case GL_DOUBLE_VEC2:		return Type::Double2;
		case GL_DOUBLE_VEC3:		return Type::Double3;
		case GL_DOUBLE_VEC4:		return Type::Double4;
		case GL_INT:				return Type::Int;
		case GL_INT_VEC2:			return Type::Int2;
		case GL_INT_VEC3:			return Type::Int3;
		case GL_INT_VEC4:			return Type::Int4;
		case GL_UNSIGNED_INT:		return Type::UInt;
		case GL_UNSIGNED_INT_VEC2:	return Type::UInt2;
		case GL_UNSIGNED_INT_VEC3:	return Type::UInt3;
		case GL_UNSIGNED_INT_VEC4:	return Type::UInt4;
		case GL_BOOL:				return Type::Bool;
		case GL_BOOL_VEC2:			return Type::Bool2;
		case GL_BOOL_VEC3:			return Type::Bool3;
		case GL_BOOL_VEC4:			return Type::Bool4;
		case GL_FLOAT_MAT2:			return Type::FloatMat2;
		case GL_FLOAT_MAT3:			return Type::FloatMat3;
		case GL_FLOAT_MAT4:			return Type::FloatMat4;
		case GL_FLOAT_MAT2x3:		return Type::FloatMat2x3;
		case GL_FLOAT_MAT2x4:		return Type::FloatMat2x4;
		case GL_FLOAT_MAT3x2:		return Type::FloatMat3x2;
		case GL_FLOAT_MAT3x4:		return Type::FloatMat3x4;
		case GL_FLOAT_MAT4x2:		return Type::FloatMat4x2;
		case GL_FLOAT_MAT4x3:		return Type::FloatMat4x3;
		case GL_DOUBLE_MAT2:		return Type::DoubleMat2;
		case GL_DOUBLE_MAT3:		return Type::DoubleMat3;
		case GL_DOUBLE_MAT4:		return Type::DoubleMat4;
		case GL_DOUBLE_MAT2x3:		return Type::DoubleMat2x3;
		case GL_DOUBLE_MAT2x4:		return Type::DoubleMat2x4;
		case GL_DOUBLE_MAT3x2:		return Type::DoubleMat3x2;
		case GL_DOUBLE_MAT3x4:		return Type::DoubleMat3x4;
		case GL_DOUBLE_MAT4x2:		return Type::DoubleMat4x2;
		case GL_DOUBLE_MAT4x3:		return Type::DoubleMat4x3;
		default:
			break;
		}
		return Type::Float;
	}
}