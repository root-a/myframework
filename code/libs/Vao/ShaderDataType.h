#pragma once
#include <string_view>

namespace ShaderDataType
{
	enum class Type
	{
		Float, Float2, Float3, Float4,
		Double, Double2, Double3, Double4,
		Int, Int2, Int3, Int4,
		UInt, UInt2, UInt3, UInt4,
		Bool, Bool2, Bool3, Bool4,
		FloatMat2, FloatMat3, FloatMat4, FloatMat2x3, FloatMat2x4, FloatMat3x2, FloatMat3x4, FloatMat4x2, FloatMat4x3,
		DoubleMat2, DoubleMat3, DoubleMat4, DoubleMat2x3, DoubleMat2x4, DoubleMat3x2, DoubleMat3x4, DoubleMat4x2, DoubleMat4x3,
		IntMat2, IntMat3, IntMat4, IntMat2x3, IntMat2x4, IntMat3x2, IntMat3x4, IntMat4x2, IntMat4x3,
		BoolMat2, BoolMat3, BoolMat4, BoolMat2x3, BoolMat2x4, BoolMat3x2, BoolMat3x4, BoolMat4x2, BoolMat4x3
	};


	Type FromGLStr(const char* type);
	Type FromStr(const char* type);
	std::string_view Str(Type);
	unsigned int Size(Type type);
	unsigned int ComponentCount(Type type);
	unsigned int Rows(Type type);
	unsigned int ToOpenGLBaseType(Type type);
	unsigned int BaseTypeSize(Type type);
	Type FromOpenGLType(unsigned int type);
};