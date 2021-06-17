#include "objects.h"

using glm::vec3;
using Ray = bvh::Ray<float>;

#pragma region Sphere
Sphere::Sphere(vec3 position, vec3 direction, vec3 colour, float radius) : BaseObject(position, direction, colour), rad(radius) {}

bool Sphere::intersect(const Ray& ray, HitResult& hitOut) const
{
	hitOut.hit = false;
	vec3 rayOrig = vec3(ray.origin[0], ray.origin[1], ray.origin[2]);
	vec3 rayDir = vec3(ray.direction[0], ray.direction[1], ray.direction[2]);

	using glm::dot;
	vec3 rayPosLocal = rayOrig - pos;

	float a = dot(rayDir, rayDir);
	float b = 2.f * dot(rayPosLocal, rayDir);
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
	hitOut.normal = glm::normalize(rayPosLocal + t * rayDir);
	hitOut.hit = true;

	return true;
}

void Sphere::setHitColour(HitResult& hitDataOut) const
{
	hitDataOut.colour = col;
}
#pragma endregion

#pragma region Plane
Plane::Plane(vec3 position, vec3 direction, vec3 colour) : BaseObject(position, direction, colour) {}
bool Plane::intersect(const Ray& ray, HitResult& hitOut) const
{
	hitOut.hit = false;
	vec3 rayOrig = vec3(ray.origin[0], ray.origin[1], ray.origin[2]);
	vec3 rayDir = vec3(ray.direction[0], ray.direction[1], ray.direction[2]);

	float A = glm::dot(dir, rayDir);

	if (A < 0) {
		float B = glm::dot(dir, pos - rayOrig);

		if (B < 0) {
			hitOut.t = B / A;
			hitOut.normal = dir;
			hitOut.hit = true;

			return true;
		}
	}

	hitOut.hit = false;
	return false;
}

void Plane::setHitColour(HitResult& hitDataOut) const
{
	// https://computergraphics.stackexchange.com/questions/8382/how-do-i-convert-a-hit-on-an-infinite-plane-to-uv-coordinates-for-texturing-in-a
	vec3 uAxis;
	{
		vec3 a = cross(hitDataOut.normal, vec3(1.f, 0.f, 0.f));
		vec3 b = cross(hitDataOut.normal, vec3(0.f, 1.f, 0.f));

		vec3 max_ab = dot(a, a) < dot(b, b) ? b : a;

		vec3 c = cross(hitDataOut.normal, vec3(0.f, 0.f, 1.f));
		uAxis = normalize(dot(max_ab, max_ab) < dot(c, c) ? c : max_ab);
	}

	vec3 vAxis = cross(hitDataOut.normal, uAxis);

	float u = dot(uAxis, hitDataOut.pos) * 0.25f, v = dot(vAxis, hitDataOut.pos) * 0.25f; // Scale the uvs here if needed
	u -= floor(u);
	v -= floor(v);

	// This produces a grid pattern by performing an XOR comparison between whether we hit the right half of the uv segment, or the bottom half
	// In case you don't know what XOR is, it's true when either of the two booleans are true, but not when they're both true
	bool rightHalf = u > 0.5f, bottomHalf = v > 0.5f;
	hitDataOut.colour = col * (rightHalf != bottomHalf ? 0.9f : 0.3f); // Change the weights here depending on the look you want
}
#pragma endregion
