#pragma once
#include "Object.h"
#include "RigidBody.h"
/*
//fast hash function for pointers
template<typename Tval>
struct MyTemplatePointerHash1 {
	size_t operator()(const Tval* val) const {
		static const size_t shift = (size_t)log2(1 + sizeof(Tval));
		return (size_t)(val) >> shift;
	}
};
*/

struct OverlapPair
{
	RigidBody* rbody1;
	RigidBody* rbody2;

	OverlapPair(RigidBody* body1, RigidBody* body2){ rbody1 = body1; rbody2 = body2; }
};

//std specialization
namespace std
{
	template <>
	class hash<OverlapPair>
	{
	public:
		size_t operator()(const OverlapPair &overlapPair) const{
			//Vector3* pos1 = &(overlapPair.ent1->GetPosition());
			//Vector3* pos2 = &(overlapPair.ent2->GetPosition());
			return overlapPair.rbody1->object->ID ^ overlapPair.rbody2->object->ID;//(pos1->a^pos1->b^pos1->c) + (pos2->a^pos2->b^pos2->c);
			//return overlapPair.ent1->ID + overlapPair.ent2->ID;
		}
	};

	template <>
	class equal_to<OverlapPair>
	{
	public:
		bool operator()(const OverlapPair& lhs, const OverlapPair& rhs) const
		{
			return (lhs.rbody1 == rhs.rbody1 && lhs.rbody2 == rhs.rbody2 ||
				lhs.rbody1 == rhs.rbody2 && lhs.rbody2 == rhs.rbody1);
		}
	};
}

//other way
/*
class OverlapPair_hash
{
public:
	size_t operator()(const OverlapPair &overlapPair) const{
		return overlapPair.ent1->ID + overlapPair.ent2->ID;
	}
};

class OverlapPair_equal_to
{
public:
	bool operator()(const OverlapPair& lhs, const OverlapPair& rhs) const
	{
		return (lhs.ent1 == rhs.ent1 && lhs.ent2 == rhs.ent2 ||
			lhs.ent1 == rhs.ent2 && lhs.ent2 == rhs.ent1);
	}
};

unordered_set<OverlapPair, OverlapPair_hash, OverlapPair_equal_to> fullOverlap;
*/