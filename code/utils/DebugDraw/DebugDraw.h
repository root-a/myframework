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
class Texture;
class LineSystem;
class BoundingBoxSystem;
class PointSystem;
class FrameBuffer;

class DebugDraw
{
	typedef unsigned int GLenum;
public:
	static DebugDraw* Instance();

	void DrawShapeAtPos(const char* shapeName, const mwm::Vector3& pos);
	void DrawLine(const mwm::Vector3& normal, const mwm::Vector3& position, float width = 4.f);
	void DrawPlane(const mwm::Vector3& normal, const mwm::Vector3& position, const mwm::Vector3& halfExtent = mwm::Vector3(1.0, 1.0, 1.0));
	void DrawPlaneN(const mwm::Vector3& normal, const mwm::Vector3& position, const mwm::Vector3& halfExtent = mwm::Vector3(1.0, 1.0, 1.0));
	void DrawPoint(const mwm::Vector3& position, float size = 10.f);
	void DrawNormal(const mwm::Vector3& normal, const mwm::Vector3& position, float width = 4.f);
	void DrawCrossHair(const mwm::Vector3F& color = mwm::Vector3F(1.f, 1.f, 0.f));
	void DrawQuad();
	void DrawRegion(int posX, int posY, int width, int height, const Texture* texture);
	void Clear();
	void Init(Object* debugObject);
	void DrawFastLineSystems(const FrameBuffer * fboToDrawTo, const GLenum * attachmentsToDraw, const int countOfAttachments);
	void DrawFastPointSystems(const FrameBuffer * fboToDrawTo, const GLenum * attachmentsToDraw, const int countOfAttachments);
	void DrawBoundingBoxes();
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
	std::vector<LineSystem*> lineSystems;
	std::vector<BoundingBoxSystem*> bbSystems;
	std::vector<PointSystem*> pointSystems;
private:
	DebugDraw();
	~DebugDraw();
	//copy
	DebugDraw(const DebugDraw&);
	//assign
	DebugDraw& operator=(const DebugDraw&);
	

	std::unordered_map<std::string, Object*> debugShapes;
};

