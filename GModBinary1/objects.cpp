#include "objects.h"

using glm::vec3;

Sphere::Sphere(vec3 position, vec3 direction, vec3 color, float radius) : BaseObject(position, direction, color), rad(radius) {}
bool Sphere::intersect(const Trace& trace, HitResult& hitOut) const
{

	hitOut.hit = false;

	using glm::dot;
	vec3 tracePosLocal = trace.pos - pos;

	float a = dot(trace.dir, trace.dir);
	float b = 2.f * dot(tracePosLocal, trace.dir);
	float c = dot(tracePosLocal, tracePosLocal) - rad * rad;

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
	hitOut.normal = glm::normalize(tracePosLocal + t * trace.dir);
	hitOut.color = col;
	hitOut.hit = true;

	return true;
}

Plane::Plane(vec3 position, vec3 direction, vec3 color) : BaseObject(position, direction, color) {}
bool Plane::intersect(const Trace& trace, HitResult& hitOut) const
{
	float A = glm::dot(dir, trace.dir);

	if (A < 0) {
		float B = glm::dot(dir, pos - trace.pos);

		if (B < 0) {
			float bOverA = B / A;
			unsigned int x = ((trace.pos + trace.dir) * bOverA).x;
			unsigned int z = ((trace.pos + trace.dir) * bOverA).z;

			hitOut.t = bOverA;
			hitOut.normal = dir; 
			hitOut.color = vec3(((x + z) % 2) * 127 + 255) * col;
			hitOut.hit = true;

			return true;
		}
	}

	hitOut.hit = false;

	return false;
}



/*
double epsilon = 1e-8;	//Unsure what this even is or what it means



Triangle::Triangle(vec3 position, vec3 direction, vec3 color, vec3 point0, vec3 point1, vec3 point2) : BaseObject(position, direction, color), pnt0(point0), pnt1(point1), pnt2(point2) {}
bool Triangle::intersect(const Trace& trace, HitResult& hitOut) const
{
	hitOut.hit = false;

	vec3 v0v1 = pnt1 - pnt0;
	vec3 v0v2 = pnt2 - pnt0;
	vec3 pvec = glm::cross(dir, v0v2);
	float det = glm::dot(v0v1, pvec);

	// ray and triangle are parallel if det is close to 0
	//pretty sure fabs means 'float absolute'
	if (fabs(det) < epsilon) {
		return false;
	}

	float invDet = 1 / det;

	vec3 tvec = pos - pnt0;
	float u = glm::dot(tvec, pvec) * invDet;
	if (u < 0 || u > 1) return false;

	vec3 qvec = glm::cross(tvec, v0v1);
	float v = glm::dot(dir, qvec) * invDet;
	if (v < 0 || u + v > 1) return false;

	float t = glm::dot(v0v2, qvec) * invDet;

	//return (t > 0) ? true : false;		// I assume if t > 0 return true, else return false
	if (t > 0) 
	{
		hitOut.t = t;
		hitOut.normal = dir;
		hitOut.color = col;
		hitOut.hit = true;

		return true;
	}

	return false;
}
*/
