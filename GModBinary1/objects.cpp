#include "objects.h"

using glm::vec3;

Sphere::Sphere(vec3 position, vec3 angle, vec3 colour, float radius) : BaseObject(position, angle, colour), rad(radius) {}
bool Sphere::intersect(const Ray& ray, HitResult& hitOut) const
{

	hitOut.hit = false;

	using glm::dot;
	vec3 rayPosLocal = ray.pos - pos;

	float a = dot(ray.dir, ray.dir);
	float b = 2.f * dot(rayPosLocal, ray.dir);
	float c = dot(rayPosLocal, rayPosLocal) - rad * rad;

	float discriminant = b * b - 4 * a * c;
	if (discriminant <= 0.f) return false;
	
	float t;
	float t1 = (-b - sqrt(discriminant)) / (2.f * a);
	float t2 = (-b + sqrt(discriminant)) / (2.f * a);

	if (t1 < 0.f && t2 < 0.f) return false;

	if (t1 < 0.f) t = t2;
	else if (t2 < 0.f) t = t1;
	else t = t1 < t2 ? t1 : t2;

	hitOut.t = t;
	hitOut.normal = glm::normalize(rayPosLocal + t * ray.dir);
	hitOut.colour = col;
	hitOut.hit = true;

	return true;
}

Plane::Plane(vec3 position, vec3 angle, vec3 colour) : BaseObject(position, angle, colour) {}
bool Plane::intersect(const Ray& ray, HitResult& hitOut) const
{
	float A = glm::dot(ang, ray.dir);

	if (A < 0) {
		float B = glm::dot(ang, pos - ray.pos);

		if (B < 0) {
			float bOverA = B / A;
			unsigned int x = ((ray.pos + ray.dir) * bOverA).x;
			unsigned int z = ((ray.pos + ray.dir) * bOverA).z;

			hitOut.t = bOverA;
			hitOut.normal = ang; // why is this even called ang, looks more like an up vector describing the object
			hitOut.colour = vec3(((x + z) % 2) * 127 + 255) * col;
			hitOut.hit = true;

			return true;
		}
	}

	hitOut.hit = false;

	return false;
}
