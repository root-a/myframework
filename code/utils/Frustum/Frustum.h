#include "MyMathLib.h"

class FrustumManager
{
public:
	static FrustumManager* Instance();
	void ExtractPlanes(const mwm::Matrix4& VP);
	bool isBoundingSphereInView(mwm::Vector3 position, double radius);
	
private:
	
	FrustumManager();
	~FrustumManager();
	//copy
	FrustumManager(const FrustumManager&);
	//assign
	FrustumManager& operator=(const FrustumManager&);
	mwm::Vector4 planes[6];
	mwm::Vector4 col1;
	mwm::Vector4 col2;
	mwm::Vector4 col3;
	mwm::Vector4 col4;
	mwm::Vector3 plane3D;
	mwm::Vector3 plane3DNormal;
}; 