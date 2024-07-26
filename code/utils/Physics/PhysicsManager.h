#pragma once
#include "Object.h"
#include <unordered_set>
#include "OverlapPair.h"
#include "ObjectPoint.h"
#include "Vector3.h"

struct Contact
{
	glm::vec3 contactPoint;
	glm::vec3 contactNormal;
	double penetration = -1;
	RigidBody* one = nullptr;
	RigidBody* two = nullptr;
	/*
	bool operator<(const Contact& rhs) const
	{
		return penetration < rhs.penetration;  
	}
	*/
public:
	Contact(const glm::vec3& cp, const glm::vec3& cn, double p, RigidBody* oneb, RigidBody* twob)
		: contactPoint(cp),contactNormal(cn),penetration(p), one(oneb), two(twob) {}
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
			//return val->contactPoint.squareLength();//works

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
	
	glm::vec3 defAabbColor;
	glm::vec3 defObbColor;
	std::vector<ObjectPoint*> xAxis;
	std::vector<ObjectPoint*> yAxis;
	std::vector<ObjectPoint*> zAxis;
	std::unordered_set<OverlapPair> fullOverlaps;
	std::unordered_set<OverlapPair> satOverlaps;

	glm::vec3 gravity = glm::vec3(0.0, -9.0, 0.0);

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
	std::vector<glm::vec3> clipPolygon;
	std::vector<glm::vec3> newClipPolygon;
	const double k_allowedPenetration = 0.01;
	double BAUMGARTE = 0.2;
private:
	PhysicsManager();
	~PhysicsManager();
	//copy
	PhysicsManager(const PhysicsManager&);
	//assign
	PhysicsManager& operator=(const PhysicsManager&);
	void ProcessContact(const Contact& contact, glm::vec3&vel1, glm::vec3& ang_vel1, glm::vec3&vel2, glm::vec3& ang_vel2, double dtInv);
	void CalcFaceVertices(const glm::vec3& pos, glm::vec3* vertices, const glm::vec3& axis, const glm::mat3& model, const glm::vec3& halfExtents, bool counterClockwise = true);
	

	size_t DrawPlaneClipContacts(std::vector<glm::vec3> &contacts, const glm::vec3& normal, size_t vertCount, const glm::vec3& normalColor);

	void SortAxis(std::vector<ObjectPoint*>& axisList, axis axisToSort);
	bool CheckBoundingBoxes(RigidBody* body1, RigidBody* body2);
	void FlipMTVTest(glm::vec3&mtv, const glm::vec3 &toCentre);

	void FilterContactsAgainstReferenceFace(const glm::vec3& refNormal, double pos_offsett, double penetration, RigidBody* one, RigidBody* two);
	
	void ClaculateIncidentAxis(glm::vec3& incident_axis, const glm::mat3 &two, glm::vec3& smallestAxis);

	void CreateSidePlanesOffsetsAndNormals(const glm::vec3& onePosition, int typeOfCollision, glm::vec3& normal1, const glm::mat3& one, glm::vec3& normal2, double &neg_offset1, glm::vec3 oneHalfSize, double &pos_offset1, double &neg_offset2, double &pos_offset2);

	void ClipFaceToSidePlane(std::vector<glm::vec3>& clipPolygon, std::vector<glm::vec3>& newClipPolygon, const glm::vec3& normal, double plane_offset);
	void DrawCollisionNormal(Contact& contact);
	void DrawReferenceNormal(Contact& contact, int typeOfCollision);
	void DrawSidePlanes(const glm::vec3& normal1, const glm::vec3& normal2, const glm::vec3& onePosition, int index1, int index2, const glm::vec3& oneHalfSize);
	void PositionalCorrection(RigidBody* one, RigidBody* two, double penetration, glm::vec3& normal);
	void PositionalImpulseCorrection(RigidBody* one, RigidBody* two, Contact& contact);
	bool IntersectionTest(const RigidBody* oneObj, const RigidBody* twoObj, double& smallestPenetration, glm::vec3& smallestAxis, glm::vec3& toCentre, int& axisNumRes, int& bestSingleAxis);
	bool overlapOnAxis(const RigidBody* oneObj, const RigidBody* twoObj, const glm::vec3 &axis, const int axisNum, int& resAxisNum, const glm::vec3&toCentre, double& smallestPenetration, glm::vec3& smallestAxis);
	double penetrationOnAxis(const RigidBody* oneObj, const RigidBody* twoObj, const glm::vec3 &axis, const glm::vec3&toCentre);
	double transformToAxis(const glm::mat3 &boxModel, const glm::vec3 &axis, const glm::vec3 &boxHalfSize);
	void GenerateContacts(glm::vec3& MTV, const double& penetration, glm::vec3& toCentre, RigidBody* oneObj, RigidBody* twoObj, int axisNumRes, int& bestSingleAxis);

	void DrawFaceDebug(glm::vec3 * reference_face, glm::vec3 * incident_face, int typeOfCollision);

	void DrawReferenceAndIncidentFace(glm::vec3 * reference_face, glm::vec3 * incident_face);

	void GenerateContactPointToFace(const glm::vec3& toCentre, glm::vec3& smallestAxis, double smallestPen, RigidBody* oneObj, RigidBody* twoObj, int typeOfCollision);
	void GenerateContactEdgeToEdge(const glm::vec3& toCentre, glm::vec3& smallestAxis, double smallestPen, RigidBody* oneObj, RigidBody* twoObj, int axisNumRes, int& bestSingleAxis);
	glm::vec3 contactPoint(const glm::vec3& pOne, const glm::vec3& dOne, double oneSize, const glm::vec3& pTwo, const glm::vec3& dTwo, double twoSize, bool useOne) const;

};

inline bool PhysicsManager::overlapOnAxis(const RigidBody* oneObj, const RigidBody* twoObj, const glm::vec3& axis, const int axisNum, int& resAxisNum, const glm::vec3& toCentre, double& smallestPenetration, glm::vec3& smallestAxis)
{
	if (glm::dot(axis, axis) < 0.0001) return true;

	glm::vec3 axisn = glm::normalize(axis);

	double penetration = penetrationOnAxis(oneObj, twoObj, axisn, toCentre);

	if (penetration < 0.000001) return false;
	if (penetration < smallestPenetration) {
		smallestPenetration = penetration;
		smallestAxis = axisn;
		resAxisNum = axisNum;
	}
	return true;
}


inline double PhysicsManager::penetrationOnAxis(const RigidBody* oneObj, const RigidBody* twoObj, const glm::vec3 &axis, const glm::vec3&toCentre)
{
	// Project the half-size of one onto axis
	double oneProject = transformToAxis(oneObj->object->bounds->obb.rot, axis, oneObj->object->bounds->obb.halfExtents);
	double twoProject = transformToAxis(twoObj->object->bounds->obb.rot, axis, twoObj->object->bounds->obb.halfExtents);

	// Project this onto the axis
	double distance = abs(glm::dot(toCentre, axis));
	//printf("\naxis %f %f %f pen: %f", axis.x, axis.y, axis.z, toCentre.dot(axis));
	// Return the overlap (i.e. positive indicates
	// overlap, negative indicates separation).
	double overlap = oneProject + twoProject - distance;
	return overlap;
}

inline double PhysicsManager::transformToAxis(const glm::mat3&boxModel, const glm::vec3&axis, const glm::vec3&boxHalfSize)
{
	return
		boxHalfSize.x * abs(glm::dot(axis, MathUtils::GetAxis(boxModel, 0))) + //get not normalized axes otherwise you will get floating errors!
		boxHalfSize.y * abs(glm::dot(axis, MathUtils::GetAxis(boxModel, 1))) +
		boxHalfSize.z * abs(glm::dot(axis, MathUtils::GetAxis(boxModel, 2)));
}

inline glm::vec3 PhysicsManager::contactPoint(const glm::vec3& pOne, const glm::vec3& dOne, double oneSize, const glm::vec3& pTwo, const glm::vec3& dTwo, double twoSize, bool useOne) const
{
	// If useOne is true, and the contact point is outside
	// the edge (in the case of an edge-face contact) then
	// we use one's midpoint, otherwise we use two's.

	glm::vec3 toSt, cOne, cTwo;
	float dpStaOne, dpStaTwo, dpOneTwo, smOne, smTwo;
	float denom, mua, mub;

	smOne = glm::dot(dOne, dOne);
	smTwo = glm::dot(dTwo, dTwo);
	dpOneTwo = glm::dot(dTwo, dOne);

	toSt = pOne - pTwo;
	dpStaOne = glm::dot(dOne, toSt);
	dpStaTwo = glm::dot(dTwo, toSt);

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

		return cOne * 0.5f + cTwo * 0.5f;
	}
}


//half size is Dimensions / 2.f * obbScale

//it's 0.5 * scale
//it's max values * scale
//dimensions is something calculated once
//0.5 is something known for unit cube
//max values can be calculated or stored once