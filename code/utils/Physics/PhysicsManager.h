#pragma once
#include "Object.h"
#include <unordered_set>
#include "OverlapPair.h"
#include "ObjectPoint.h"
#include "Vector3.h"

struct Contact
{
	mwm::Vector3 contactPoint;
	mwm::Vector3 contactNormal;
	double penetration = -1;
	RigidBody* one = nullptr;
	RigidBody* two = nullptr;
	/*
	bool operator<(const Contact& rhs) const
	{
		return penetration < rhs.penetration;  
	}
	*/
};

/*
//std specialization
namespace std
{
	template <>
	class hash < Contact >
	{
	public:		
		size_t operator()(const Contact val) const {
			return val.contactPoint.a ^ val.contactPoint.b ^ val.contactPoint.c;
			//return val->contactPoint.squareMag();//works

		}
	};

	template <>
	class equal_to < Contact >
	{
	public:
		bool operator()(const Contact lhs, const Contact rhs) const
		{
			return (lhs.contactPoint == rhs.contactPoint);
		}
	};
}
*/

class PhysicsManager
{
public:
	static PhysicsManager* Instance();

	enum axis
	{
		x,
		y,
		z
	};

	
	void RegisterRigidBody(RigidBody* body);
	void SortAndSweep();
	void NarrowTestSAT(double deltaTime);
	
	mwm::Vector3F defAabbColor;
	mwm::Vector3F defObbColor;
	std::vector<ObjectPoint*> xAxis;
	std::vector<ObjectPoint*> yAxis;
	std::vector<ObjectPoint*> zAxis;
	std::unordered_set<OverlapPair> fullOverlaps;
	std::unordered_set<OverlapPair> satOverlaps;

	mwm::Vector3 gravity = mwm::Vector3(0.0, -9.0, 0.0);

	void Update(double deltaTime);
	void Clear();
	double satTime;
	double pruneAndSweepTime;
	double intersectionTestTime;
	double generateContactsTime;
	double processContactTime;
	double positionalCorrectionTime;
	int iterCount;
	std::vector<Contact> contacts;
	std::vector<mwm::Vector3> clipPolygon;
	std::vector<mwm::Vector3> newClipPolygon;
	const double k_allowedPenetration = 0.01;
	double BAUMGARTE = 0.2;
private:
	PhysicsManager();
	~PhysicsManager();
	//copy
	PhysicsManager(const PhysicsManager&);
	//assign
	PhysicsManager& operator=(const PhysicsManager&);
	void ProcessContact(const Contact& contact, mwm::Vector3&vel1, mwm::Vector3& ang_vel1, mwm::Vector3&vel2, mwm::Vector3& ang_vel2, double dtInv);
	void CalcFaceVertices(const mwm::Vector3& pos, mwm::Vector3* vertices, const mwm::Vector3& axis, const mwm::Matrix3& model, const mwm::Vector3& halfExtents, bool counterClockwise = true);
	

	size_t DrawPlaneClipContacts(std::vector<mwm::Vector3> &contacts, const mwm::Vector3& normal, size_t vertCount, const mwm::Vector3F& normalColor);

	void SortAxis(std::vector<ObjectPoint*>& axisList, axis axisToSort);
	bool CheckBoundingBoxes(RigidBody* body1, RigidBody* body2);
	void FlipMTVTest(mwm::Vector3 &mtv, const mwm::Vector3 &toCentre);

	void FilterContactsAgainstReferenceFace(const mwm::Vector3& refNormal, double pos_offsett, double penetration, RigidBody* one, RigidBody* two);
	
	void ClaculateIncidentAxis(mwm::Vector3& incident_axis, const mwm::Matrix3 &two, mwm::Vector3 smallestAxis);

	void CreateSidePlanesOffsetsAndNormals(const mwm::Vector3& onePosition, int typeOfCollision, mwm::Vector3 &normal1, const mwm::Matrix3 &one, mwm::Vector3 &normal2, double &neg_offset1, mwm::Vector3 oneHalfSize, double &pos_offset1, double &neg_offset2, double &pos_offset2);

	void ClipFaceToSidePlane(std::vector<mwm::Vector3>& clipPolygon, std::vector<mwm::Vector3>& newClipPolygon, const mwm::Vector3& normal, double plane_offset);
	void DrawCollisionNormal(Contact& contact);
	void DrawReferenceNormal(Contact& contact, int typeOfCollision);
	void DrawSidePlanes(const mwm::Vector3& normal1, const mwm::Vector3& normal2, const mwm::Vector3& onePosition, int index1, int index2, const mwm::Vector3& oneHalfSize);
	void PositionalCorrection(RigidBody* one, RigidBody* two, double penetration, mwm::Vector3& normal);
	void PositionalImpulseCorrection(RigidBody* one, RigidBody* two, Contact& contact);
	bool IntersectionTest(const RigidBody* oneObj, const RigidBody* twoObj, double& smallestPenetration, mwm::Vector3& smallestAxis, mwm::Vector3& toCentre, int& axisNumRes, int& bestSingleAxis);
	bool overlapOnAxis(const RigidBody* oneObj, const RigidBody* twoObj, const mwm::Vector3 &axis, const int axisNum, int& resAxisNum, const mwm::Vector3 &toCentre, double& smallestPenetration, mwm::Vector3& smallestAxis);
	double penetrationOnAxis(const RigidBody* oneObj, const RigidBody* twoObj, const mwm::Vector3 &axis, const mwm::Vector3 &toCentre);
	double transformToAxis(const mwm::Matrix3 &boxModel, const mwm::Vector3 &axis, const mwm::Vector3 &boxHalfSize);
	void GenerateContacts(mwm::Vector3& MTV, const double& penetration, mwm::Vector3& toCentre, RigidBody* oneObj, RigidBody* twoObj, int axisNumRes, int& bestSingleAxis);

	void DrawFaceDebug(mwm::Vector3 * reference_face, mwm::Vector3 * incident_face, int typeOfCollision);

	void DrawReferenceAndIncidentFace(mwm::Vector3 * reference_face, mwm::Vector3 * incident_face);

	void GenerateContactPointToFace(mwm::Vector3 &toCentre, mwm::Vector3& smallestAxis, double smallestPen, RigidBody* oneObj, RigidBody* twoObj, int typeOfCollision);
	void GenerateContactEdgeToEdge(const mwm::Vector3 &toCentre, mwm::Vector3& smallestAxis, double smallestPen, RigidBody* oneObj, RigidBody* twoObj, int axisNumRes, int& bestSingleAxis);
	mwm::Vector3 contactPoint(const mwm::Vector3 &pOne, const mwm::Vector3 &dOne, double oneSize, const mwm::Vector3 &pTwo, const mwm::Vector3 &dTwo, double twoSize, bool useOne) const;

};

inline bool PhysicsManager::overlapOnAxis(const RigidBody* oneObj, const RigidBody* twoObj, const mwm::Vector3 &axis, const int axisNum, int& resAxisNum, const mwm::Vector3 &toCentre, double& smallestPenetration, mwm::Vector3& smallestAxis)
{
	if (axis.squareMag() < 0.0001) return true;

	mwm::Vector3& axisn = axis.vectNormalize();

	double penetration = penetrationOnAxis(oneObj, twoObj, axisn, toCentre);

	if (penetration < 0.000001) return false;
	if (penetration < smallestPenetration) {
		smallestPenetration = penetration;
		smallestAxis = axisn;
		resAxisNum = axisNum;
	}
	return true;
}


inline double PhysicsManager::penetrationOnAxis(const RigidBody* oneObj, const RigidBody* twoObj, const mwm::Vector3 &axis, const mwm::Vector3 &toCentre)
{
	// Project the half-size of one onto axis
	double oneProject = transformToAxis(oneObj->object->bounds->obb.rot, axis, oneObj->object->bounds->obb.halfExtents);
	double twoProject = transformToAxis(twoObj->object->bounds->obb.rot, axis, twoObj->object->bounds->obb.halfExtents);

	// Project this onto the axis
	double distance = abs(toCentre.dotAKAscalar(axis));
	//printf("\naxis %f %f %f pen: %f", axis.x, axis.y, axis.z, toCentre.dotAKAscalar(axis));
	// Return the overlap (i.e. positive indicates
	// overlap, negative indicates separation).
	double overlap = oneProject + twoProject - distance;
	return overlap;
}

inline double PhysicsManager::transformToAxis(const mwm::Matrix3 &boxModel, const mwm::Vector3 &axis, const mwm::Vector3 &boxHalfSize)
{
	return
		boxHalfSize.x * abs(axis.dotAKAscalar(boxModel.getAxis(0))) + //get not normalized axes otherwise you will get floating errors!
		boxHalfSize.y * abs(axis.dotAKAscalar(boxModel.getAxis(1))) +
		boxHalfSize.z * abs(axis.dotAKAscalar(boxModel.getAxis(2)));
}

inline mwm::Vector3 PhysicsManager::contactPoint(const mwm::Vector3 &pOne, const mwm::Vector3 &dOne, double oneSize, const mwm::Vector3 &pTwo, const mwm::Vector3 &dTwo, double twoSize, bool useOne) const
{
	// If useOne is true, and the contact point is outside
	// the edge (in the case of an edge-face contact) then
	// we use one's midpoint, otherwise we use two's.

	mwm::Vector3 toSt, cOne, cTwo;
	double dpStaOne, dpStaTwo, dpOneTwo, smOne, smTwo;
	double denom, mua, mub;

	smOne = dOne.squareMag();
	smTwo = dTwo.squareMag();
	dpOneTwo = dTwo.dotAKAscalar(dOne);

	toSt = pOne - pTwo;
	dpStaOne = dOne.dotAKAscalar(toSt);
	dpStaTwo = dTwo.dotAKAscalar(toSt);

	denom = smOne * smTwo - dpOneTwo * dpOneTwo;

	// Zero denominator indicates parrallel lines
	if (abs(denom) < 0.0001f) {
		return useOne ? pOne : pTwo;
	}

	mua = (dpOneTwo * dpStaTwo - smTwo * dpStaOne) / denom;
	mub = (smOne * dpStaTwo - dpOneTwo * dpStaOne) / denom;

	// If either of the edges has the nearest point out
	// of bounds, then the edges aren't crossed, we have
	// an edge-face contact. Our point is on the edge, which
	// we know from the useOne parameter.
	if (mua > oneSize ||
		mua < -oneSize ||
		mub > twoSize ||
		mub < -twoSize)
	{
		return useOne ? pOne : pTwo;
	}
	else
	{
		cOne = pOne + mua * dOne;
		cTwo = pTwo + mub * dTwo;

		return cOne * 0.5 + cTwo * 0.5;
	}
}


//half size is Dimensions / 2.f * obbScale

//it's 0.5 * scale
//it's max values * scale
//dimensions is something calculated once
//0.5 is something known for unit cube
//max values can be calculated or stored once