#pragma once

#include "glm/glm.hpp"
#include "bvh/ray.hpp"

class BaseObject; // might not need this forward declaration for the hit result struct, but pretty sure you do

struct HitResult
{
	bool hit;
	float t;
	glm::vec3 normal; 
	glm::vec3 pos;
	glm::vec3 colour;
};

class BaseObject
{
public:
	glm::vec3 pos;
	glm::vec3 dir;
	glm::vec3 col;

	BaseObject(glm::vec3 position, glm::vec3 direction, glm::vec3 colour) : pos(position), dir(direction), col(colour) {}
	virtual bool intersect(const bvh::Ray<float>& ray, HitResult& hitOut) const = 0;
	virtual void setHitColour(HitResult& hitDataOut) const = 0;
};