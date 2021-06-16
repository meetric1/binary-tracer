#pragma once

#include "types.h"

class Sphere : public BaseObject
{
	float rad;

	public:
		Sphere(glm::vec3 position, glm::vec3 direction, glm::vec3 color, float radius);
		bool intersect(const Ray& ray, HitResult& hitOut) const;
};

class Plane : public BaseObject
{
	public:
		Plane(glm::vec3 position, glm::vec3 direction, glm::vec3 color = glm::vec3(1.f));
		bool intersect(const Ray& ray, HitResult& hitOut) const;
};

class Triangle : public BaseObject
{
	glm::vec3 pnt0;
	glm::vec3 pnt1;
	glm::vec3 pnt2;

public:
	Triangle(glm::vec3 position, glm::vec3 direction, glm::vec3 color, glm::vec3 point0, glm::vec3 point1, glm::vec3 point2);
	bool intersect(const Ray& ray, HitResult& hitOut) const;
};

