#pragma once

#include "glm/glm.hpp"

class BaseObject; // might not need this forward declaration for the hit result struct, but pretty sure you do

struct HitResult
{
	bool hit;
	float t;
	glm::vec3 normal; 
	glm::vec3 pos;
	glm::vec3 colour;
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

	BaseObject(glm::vec3 position, glm::vec3 angle, glm::vec3 colour) : pos(position), ang(angle), col(colour) {}
	virtual bool intersect(const Ray& ray, HitResult& hitOut) const = 0;
};