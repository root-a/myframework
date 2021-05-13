#include "MyMathLib.h"

class FrustumManager
{
public:
	static FrustumManager* Instance();
	void ExtractPlanes(const Matrix4& VP);
	bool isBoundingSphereInView(Vector3 position, double radius);
	
private:
	
	FrustumManager();
	~FrustumManager();
	//copy
	FrustumManager(const FrustumManager&);
	//assign
	FrustumManager& operator=(const FrustumManager&);
	Vector4 planes[6];
	Vector4 col1;
	Vector4 col2;
	Vector4 col3;
	Vector4 col4;
	Vector3 plane3D;
	Vector3 plane3DNormal;
}; 