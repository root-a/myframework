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

}; 