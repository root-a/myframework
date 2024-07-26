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
class CircleSystem;
class Shader;

class DebugDraw
{
	typedef unsigned int GLenum;
	typedef unsigned int GLuint;
public:
	static DebugDraw* Instance();

	void DrawShapeAtPos(const char* shapeName, const glm::vec3& pos);
	void DrawLine(const glm::vec3& normal, const glm::vec3& position);
	void DrawPlane(const glm::vec3& normal, const glm::vec3& position, const glm::vec3& halfExtent = glm::vec3(1.0, 1.0, 1.0));
	void DrawPlaneN(const glm::vec3& normal, const glm::vec3& position, const glm::vec3& halfExtent = glm::vec3(1.0, 1.0, 1.0));
	void DrawPoint(const glm::vec3& position, float size = 10.f);
	void DrawNormal(const glm::vec3& normal, const glm::vec3& position);
	void DrawFrustum(glm::vec3* frustumVertices) const;
	void DrawCrossHair(const glm::vec3& color = glm::vec3(1.f, 1.f, 0.f));
	void DrawQuad();
	void DrawRegion(int posX, int posY, int width, int height, const Texture* texture);
	void Clear();
	void Init(Object* debugObject);
	void DrawFastLineSystems(GLuint fboToDrawTo = 0);
	void DrawFastPointSystems(GLuint fboToDrawTo = 0);
	void DrawFastCircleSystems(GLuint fboToDrawTo = 0);
	void DrawBoundingBoxes(GLuint fboToDrawTo = 0);
	void GenerateBoudingBoxes();
	void GenerateFastLines();
	void GenerateFastLineChildren(Node * parent, Node * child);
	Material* debugMat;
	
	glm::mat4* Projection;
	glm::mat4* View;
	void LoadPrimitives();
	bool debug = false;
	std::vector<LineSystem*> lineSystems;
	std::vector<BoundingBoxSystem*> bbSystems;
	std::vector<PointSystem*> pointSystems;
	std::vector<CircleSystem*> circleSystems;
	int boundingBoxesDrawn = 0;
private:
	DebugDraw();
	~DebugDraw();
	//copy
	DebugDraw(const DebugDraw&);
	//assign
	DebugDraw& operator=(const DebugDraw&);
	std::unordered_map<std::string, Object*> debugShapes;
	Shader* wireframeShader;
	Shader* fastBBShader;
	Shader* fastLineShader;
	Shader* fastCircleShader;
};

