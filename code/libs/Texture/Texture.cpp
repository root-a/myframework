#include "Texture.h"

Texture::Texture(GLenum Target, GLint Level, GLint InternalFormat, GLsizei Width, GLsizei Height, GLenum Format, GLenum Type, void* Pixels, GLenum Attachment)
{
	target = Target;
	level = Level;
	internalFormat = InternalFormat;
	width = Width;
	height = Height;
	aspect = (double)Width / (double)Height;
	border = 0;
	format = Format;
	type = Type;
	pixels = Pixels;
	attachment = Attachment;
	hasMipMaps = false;
}

Texture::Texture(GLint Target, GLint Level, GLint InternalFormat, GLsizei Width, GLsizei Height, GLint Format, GLint Type, void* Pixels, GLint Attachment)
{
	target = Target;
	level = Level;
	internalFormat = InternalFormat;
	width = Width;
	height = Height;
	aspect = (double)Width / (double)Height;
	border = 0;
	format = Format;
	type = Type;
	pixels = Pixels;
	attachment = Attachment;
	hasMipMaps = false;
}

Texture::~Texture()
{
	glDeleteTextures(1, &handle);
}

void Texture::SetTextureParameterI(GLenum pname, GLint param)
{
	glTexParameteri(target, pname, param);
}

void Texture::SetTextureParameterIV(GLenum pname, int * param)
{
	glTexParameteriv(target, pname, param);
}

void Texture::SetTextureParameterF(GLenum pname, GLfloat param)
{
	glTexParameterf(target, pname, param);
}

void Texture::SetTextureParameterFV(GLenum pname, float * param)
{
	glTexParameterfv(target, pname, param);
}

void Texture::SetDefaultParameters()
{
	if (target == GL_TEXTURE_CUBE_MAP)
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
	else
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (hasMipMaps) glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	else glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void Texture::SetClampingToEdge()
{
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if (target == GL_TEXTURE_CUBE_MAP) glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Texture::SetClampingToBorder(const Vector4F& borderColorIn)
{
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	if (target == GL_TEXTURE_CUBE_MAP) glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, &borderColorIn.x);
}

void Texture::SetLinear()
{
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (hasMipMaps) glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	else glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void Texture::SetNearest()
{
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	if (hasMipMaps) glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	else glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void Texture::SetFiltering(GLint MIN_FILTER, GLint MAG_FILTER)
{
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, MAG_FILTER);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, MIN_FILTER);
}

void Texture::GenerateMipMaps()
{
	glGenerateMipmap(target);
	hasMipMaps = true;
}

void Texture::Update(int windowWidth, int windowHeight)
{
	width = windowWidth;
	height = windowHeight;
	aspect = (double)width / (double)height;
	glBindTexture(target, handle);
	glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
	if (hasMipMaps) glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::Generate()
{
	glGenTextures(1, &handle);
}

void Texture::GenerateBindSpecify()
{
	glGenTextures(1, &handle);
	glBindTexture(target, handle);
	if (target == GL_TEXTURE_CUBE_MAP)
	{
		for (int i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, level, internalFormat, width, height, border, format, type, pixels);
		}
	}
	else
	{
		glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
	}
}

void Texture::Specify()
{
	if (target == GL_TEXTURE_CUBE_MAP)
	{
		for (int i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, level, internalFormat, width, height, border, format, type, pixels);
		}
	}
	else
	{
		glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
	}
}

void Texture::SpecifyTexture(GLenum Target, GLsizei Width, GLsizei Height, void* Pixels)
{
	glTexImage2D(Target, level, internalFormat, Width, Height, border, format, type, Pixels);
}

void Texture::ActivateAndBind(int textureSlot) const
{
	glBindTextureUnit(textureSlot, handle);
}

void Texture::Bind() const
{
	glBindTexture(target, handle);
}

void Texture::Activate(int textureSlot)
{
	glActiveTexture(GL_TEXTURE0 + textureSlot);
}
