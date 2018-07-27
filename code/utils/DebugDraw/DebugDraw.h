#pragma once
#include "MyMathLib.h"
#include <unordered_map>
#include <string>
#include "BoundingBox.h"
#include "Line.h"
#include "Plane.h"
#include "Point.h"
#include "Box.h"

class Object;
class Mesh;
class Material;
class FrameBuffer;

class DebugDraw
{
public:
	static DebugDraw* Instance();

	void DrawShapeAtPos(const char* shapeName, const mwm::Vector3& pos);
	void DrawLine(const mwm::Vector3& normal, const mwm::Vector3& position, float width = 4.f);
	void DrawPlane(const mwm::Vector3& normal, const mwm::Vector3& position, const mwm::Vector3& halfExtent = mwm::Vector3(1.0, 1.0, 1.0));
	void DrawPlaneN(const mwm::Vector3& normal, const mwm::Vector3& position, const mwm::Vector3& halfExtent = mwm::Vector3(1.0, 1.0, 1.0));
	void DrawPoint(const mwm::Vector3& position, float size = 10.f);
	void DrawNormal(const mwm::Vector3& normal, const mwm::Vector3& position, float width = 4.f);
	void DrawCrossHair(int windowWidth, int windowHeight, const mwm::Vector3F& color = mwm::Vector3F(1.f, 1.f, 0.f));
	void DrawQuad();
	void DrawMap(int posX, int posY, int width, int height, unsigned int textureHandle, int windowWidth, int windowHeight);
	Material* debugMat;
	
	mwm::Matrix4* Projection;
	mwm::Matrix4* View;
	void LoadPrimitives();
	BoundingBox boundingBox;
	Box box;
	Line line;
	Plane plane;
	Point point;
	bool debug = false;
private:
	DebugDraw();
	~DebugDraw();
	//copy
	DebugDraw(const DebugDraw&);
	//assign
	DebugDraw& operator=(const DebugDraw&);
	

	std::unordered_map<std::string, Object*> debugShapes;
};

