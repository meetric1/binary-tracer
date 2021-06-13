#pragma once

#include "glm/glm.hpp"

class BaseObject; // might not need this forward declaration for the hit result struct, but pretty sure you do

struct HitResult
{
	bool hit;
	float t;
	glm::vec3 normal; 
	glm::vec3 pos;
	glm::vec3 color;
};

struct Ray
{
	glm::vec3 pos;
	glm::vec3 dir;
};

class BaseObject
{
public:
	glm::vec3 pos;
	glm::vec3 ang;
	glm::vec3 col;

	BaseObject(vec3 position, vec3 angle, vec3 colour) : pos(position), ang(angle), color(colour) {}
	virtual bool intersect(const Ray& ray, HitResult& hitOut) const = 0;
};