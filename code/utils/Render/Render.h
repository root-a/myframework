#pragma once
namespace mwm
{
	class Matrix4;
}

class Object;

class Render
{
	typedef unsigned int GLuint;
public:
	static void drawLight(const Object* object, const mwm::Matrix4& ViewProjection, const GLuint currentShaderID);
	static void drawGeometry(const Object* object, const mwm::Matrix4& ViewProjection, const GLuint currentShaderID);
	static void draw(const Object* object, const mwm::Matrix4& ViewProjection, const GLuint currentShaderID);
	static void drawDepth(Object* object, const mwm::Matrix4& ViewProjection, const GLuint currentShaderID);
	static void drawSkybox(Object* object, const mwm::Matrix4& ViewProjection, const GLuint currentShaderID);
private:
    Render();
    ~Render();

};