#include <string.h>
#include <vector>
#include <map>
#include "MyMathLib.h"

class OBJ
{
public:
	std::vector<unsigned int> indices;
	std::vector<mwm::Vector3F> indexed_vertices;
	std::vector<mwm::Vector2F> indexed_uvs;
	std::vector<mwm::Vector3F> indexed_normals;
	int ID;
	std::string name;
	bool LoadOBJ(
		const char * path,
		std::vector<mwm::Vector3F> & out_vertices,
		std::vector<mwm::Vector2F> & out_uvs,
		std::vector<mwm::Vector3F> & out_normals
		);
	void indexVBO(
		std::vector<mwm::Vector3F> & in_vertices,
		std::vector<mwm::Vector2F> & in_uvs,
		std::vector<mwm::Vector3F> & in_normals
		);
	void LoadAndIndexOBJ(char* path);
	
	mwm::Vector3 GetDimensions();
	mwm::Vector3 CenterOfOMesh();
	
	mwm::Vector3 dimensions;
	mwm::Vector3 center_of_mesh;

	void CalculateDimensions();
private:
	struct PackedVertex{
		mwm::Vector3F position;
		mwm::Vector2F uv;
		mwm::Vector3F normal;
		bool operator<(const PackedVertex that) const{
			return memcmp((void*)this, (void*)&that, sizeof(PackedVertex))>0;
		};
	};
	static bool getSimilarVertexIndex_fast(
		PackedVertex & packed,
		std::map<PackedVertex, unsigned int> & VertexToOutIndex,
		unsigned int & result
		);
};

