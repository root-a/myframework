#include "Texture.h"

Texture::Texture(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels, GLenum attachment)
{
	this->target = target;
	this->level = level;
	this->internalFormat = internalFormat;
	this->width = width;
	this->height = height;
	this->aspect = (double)width / (double)height;
	this->border = 0;
	this->format = format;
	this->type = type;
	this->pixels = pixels;
	this->attachment = attachment;
	this->hasBorder = false;
	this->hasMipMaps = false;
}

Texture::~Texture()
{
	glDeleteTextures(1, &handle);
}

void Texture::AddTextureParameterI(GLenum pname, GLint param)
{
	parametersI.push_back(TextureParameterI(pname, param));
}

void Texture::AddTextureParameterF(GLenum pname, GLfloat param)
{
	parametersF.push_back(TextureParameterF(pname, param));
}

void Texture::AddDefaultTextureParameters()
{
	AddTextureParameterI(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	AddTextureParameterI(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if (target == GL_TEXTURE_CUBE_MAP) AddTextureParameterI(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	AddTextureParameterI(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	AddTextureParameterI(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//AddTextureParameterI(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
}

void Texture::AddClampingToEdge()
{
	AddTextureParameterI(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	AddTextureParameterI(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if (target == GL_TEXTURE_CUBE_MAP) AddTextureParameterI(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Texture::AddClampingToBorder(mwm::Vector4F& borderColorIn)
{
	AddTextureParameterI(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	AddTextureParameterI(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	if (target == GL_TEXTURE_CUBE_MAP) AddTextureParameterI(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	borderColor = borderColorIn;
	hasBorder = true;
}

void Texture::SetLinear()
{
	AddTextureParameterI(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	AddTextureParameterI(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void Texture::SetNearest()
{
	AddTextureParameterI(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	AddTextureParameterI(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void Texture::SetFiltering(GLint MIN_FILTER, GLint MAG_FILTER)
{
	AddTextureParameterI(GL_TEXTURE_MAG_FILTER, MAG_FILTER);
	AddTextureParameterI(GL_TEXTURE_MIN_FILTER, MIN_FILTER);
}

void Texture::GenerateMipMaps()
{
	glGenerateMipmap(target);
}

void Texture::Update(int windowWidth, int windowHeight)
{
	width = windowWidth;
	height = windowHeight;
	aspect = (double)width / (double)height;
	glBindTexture(target, handle);
	glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
	//glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::Generate()
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
	//glEnable(GL_TEXTURE_2D);
	//glGenerateMipmap(GL_TEXTURE_2D);
	for (auto& paramI : parametersI)
	{
		glTexParameteri(target, paramI.pname, paramI.param);
	}
	for (auto& paramF : parametersF)
	{
		glTexParameterf(target, paramF.pname, paramF.param);
	}
	if (hasBorder)
	{
		glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, &borderColor.x);
	}
	if (hasMipMaps)
	{
		glGenerateMipmap(target);
	}
}

void Texture::ActivateAndBind(int textureSlot) const
{
	glActiveTexture(GL_TEXTURE0 + textureSlot);
	glBindTexture(target, handle);
}

void Texture::Bind() const
{
	glBindTexture(target, handle);
}

void Texture::Activate(int textureSlot)
{
	glActiveTexture(GL_TEXTURE0 + textureSlot);
}
