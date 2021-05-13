#pragma once
#include "MyMathLib.h"
#include <unordered_map>
#include <string>

class Object;
class Material;
class FrameBuffer;
class Texture;
class LineSystem;
class BoundingBoxSystem;
class PointSystem;
class FrameBuffer;
class Node;

class DebugDraw
{
	typedef unsigned int GLenum;
	typedef unsigned int GLuint;
public:
	static DebugDraw* Instance();

	void DrawShapeAtPos(const char* shapeName, const Vector3& pos);
	void DrawLine(const Vector3& normal, const Vector3& position, float width = 4.f);
	void DrawPlane(const Vector3& normal, const Vector3& position, const Vector3& halfExtent = Vector3(1.0, 1.0, 1.0));
	void DrawPlaneN(const Vector3& normal, const Vector3& position, const Vector3& halfExtent = Vector3(1.0, 1.0, 1.0));
	void DrawPoint(const Vector3& position, float size = 10.f);
	void DrawNormal(const Vector3& normal, const Vector3& position, float width = 4.f);
	void DrawCrossHair(const Vector3F& color = Vector3F(1.f, 1.f, 0.f));
	void DrawQuad();
	void DrawRegion(int posX, int posY, int width, int height, const Texture* texture);
	void Clear();
	void Init(Object* debugObject);
	void DrawFastLineSystems(GLuint fboToDrawTo = 0);
	void DrawFastPointSystems(GLuint fboToDrawTo = 0);
	void DrawBoundingBoxes(GLuint fboToDrawTo = 0);
	void GenerateBoudingBoxes();
	void GenerateFastLines();
	void GenerateFastLineChildren(Node * parent, Node * child);
	Material* debugMat;
	
	Matrix4* Projection;
	Matrix4* View;
	void LoadPrimitives();
	bool debug = false;
	std::vector<LineSystem*> lineSystems;
	std::vector<BoundingBoxSystem*> bbSystems;
	std::vector<PointSystem*> pointSystems;
	int boundingBoxesDrawn = 0;
private:
	DebugDraw();
	~DebugDraw();
	//copy
	DebugDraw(const DebugDraw&);
	//assign
	DebugDraw& operator=(const DebugDraw&);
	std::unordered_map<std::string, Object*> debugShapes;
};

