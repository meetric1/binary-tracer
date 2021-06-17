#define _USE_MATH_DEFINES

#include "Main.h"

#include "GarrysMod/Lua/Interface.h"
#include "glm/gtx/rotate_vector.hpp"

#include "lighting.h"

using namespace GarrysMod::Lua;
using vec3 = glm::vec3;

using Vector3 = bvh::Vector3<float>;
using Triangle = bvh::Triangle<float>;
using Ray = bvh::Ray<float>;
using Bvh = bvh::Bvh<float>;

/// FUNCTIONS ///

// Prints input to in-game console
void printLua(ILuaBase* inst, std::string text)
{
	inst->PushSpecial(SPECIAL_GLOB);
	inst->GetField(-1, "print");
	inst->PushString(text.c_str());
	inst->Call(1, 0);
	inst->Pop();
}

// Creates and saves an image
bool ppmWrite(const char* path, const unsigned char* data, const unsigned int res[2])
{
	std::ofstream file(path, std::ofstream::binary);
	if (!file.is_open()) return false;

	const std::string hdr = "P6\n" + std::to_string(res[0]) + " " + std::to_string(res[1]) + "\n255";
	file << hdr.c_str() << std::endl;

	file.write(reinterpret_cast<const char*>(data), res[0] * res[1] * 3);

	file.close();
	return true;
}

//Writes a color to the image data array to be saved later
void writePixel(const int x, const int y, unsigned char r, unsigned char g, unsigned char b)
{
	unsigned int i = (y * Res[0] + x) * 3U;

	ImageData[i] = r;
	ImageData[i + 1U] = g;
	ImageData[i + 2U] = b;
}

//To be used later in path tracing
vec3 uniformSampleHemisphere(const float& r1, const float& r2)
{
	float sinTheta = sqrtf(1 - r1 * r1);
	float phi = 2.f * M_PI * r2;
	float x = sinTheta * cosf(phi);
	float z = sinTheta * sinf(phi);
	return vec3(x, r1, z);
}

float deg2rad(float degrees)
{
	return degrees * M_PI / 180.f;
}

std::string vec2string(vec3 v)
{
	return std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z);
}

Vector3 Vec2Vector(vec3 vec)	//shit function name but whatever lol
{
	return Vector3(vec.x, vec.y, vec.z);

}


HitResult TraceAll(Trace trace, ILuaBase* LuaBase)
{
	HitResult closestHit;
	closestHit.t = FLT_MAX;

	for (size_t i = 0U; i < ObjectArray.size(); i++) {
		HitResult hit;
		if (ObjectArray[i]->intersect(trace, hit) && hit.t < closestHit.t) {
			closestHit = hit;
		}
	}

	// Begin horrid triangle shit

	/* Calculating meshes every pixel is horribly inefficient, change this later when it is working properly! */

	Bvh bvh;	

	// Create an acceleration data structure on those triangles
	bvh::SweepSahBuilder<Bvh> builder(bvh);
	auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(TriangleArray.data(), TriangleArray.size());
	auto global_bbox = bvh::compute_bounding_boxes_union(bboxes.get(), TriangleArray.size());
	builder.build(global_bbox, bboxes.get(), centers.get(), TriangleArray.size());

	// Intersect a ray with the data structure
	Ray ray(
		Vec2Vector(trace.pos),		// origin
		Vec2Vector(trace.dir),		// direction
		0.f,						// minimum distance
		FLT_MAX						// maximum distance
	);
	bvh::ClosestPrimitiveIntersector<Bvh, Triangle> primitive_intersector(bvh, TriangleArray.data());
	bvh::SingleRayTraverser<Bvh> traverser(bvh);

	auto hit = traverser.traverse(ray, primitive_intersector);

	if (hit) {
		auto intersection = hit->intersection;
		
		if (intersection.t < closestHit.t)
		{
			closestHit.t = intersection.t;
			closestHit.color = vec3(0, 255, 0);			//just fucking make it green idc
			closestHit.normal = vec3(1.f, 0.f, 0.f);	//how do I calculate this?
			closestHit.hit = true;
			

		}
		//intersection.u
		//intersection.v
	}


	// End triangle shit

	if (closestHit.hit)
	{
		closestHit.color = closestHit.color * calculateLighting(closestHit);
	}

	closestHit.pos = trace.pos + trace.dir * closestHit.t; // calculate position at the end rather than every object (way faster)
	return closestHit;
}

// Called when the module is loaded
GMOD_MODULE_OPEN()
{
	/// Variable creation ///

	// Time to see how long the render took
	std::chrono::steady_clock::time_point RENDER_START_TIME = std::chrono::high_resolution_clock::now();

	// Resolution
	Res[0] = 1920;         // X res
	Res[1] = 1080;         // Y res
	
	ImageData = std::vector<unsigned char>(Res[0] * Res[1] * 3U, 0);

	// Camera
	Camera Cam{ vec3(0.f, 0.f, 0.f), normalize(vec3(1.f, 0.f, 0.f)), 90U };


	// Objects
	{
		Plane p(vec3(0.f, -10.f, 0.f), vec3(0.f, 1.f, 0.f));
		ObjectArray.push_back(std::make_shared<Plane>(p));
	}

	for(int i = 0; i < 10; i++) {
		Sphere s{ vec3(sin(deg2rad(i * 36)) * 10 - 25, -5, cos(deg2rad(i * 36)) * 10), vec3(), vec3(i * 10), 3.f};
		ObjectArray.push_back(std::make_shared<Sphere>(s));
	}

	// Singular Triangle 
	TriangleArray.emplace_back(
		Vector3(10.f, -10.f, 10.f),
		Vector3(10.f, 10.f, 10.f),
		Vector3(-10.f, 10.f, 10.f)
	);




	glm::mat3x3 matrix = glm::orientation(Cam.dir, vec3(0.f, 0.f, 1.f));

	/// Actual Tracing ///
	for (unsigned int y = 0; y < Res[1]; y++) {
		for (unsigned int x = 0; x < Res[0]; x++) {
			//Fov calc

			float scale = tan(deg2rad(Cam.fov * 0.5f));
			float imageAspectRatio = Res[0] / static_cast<float>(Res[1]);

			float xDir = (2.f * (x + 0.5f) / static_cast<float>(Res[0]) - 1) * imageAspectRatio * scale;
			float yDir = (1.f - 2.f * (y + 0.5f) / static_cast<float>(Res[1])) * scale;

			Trace trace{ Cam.pos, vec3(xDir, yDir, 1) * matrix };

			HitResult hit = TraceAll(trace, LUA);

			vec3 FinalColor = vec3(168, 219, 243);
			if (hit.hit) {
				FinalColor = hit.color;
			}

			writePixel(x, y, FinalColor.x, FinalColor.y, FinalColor.z);
		}
	}
	
	bool success = ppmWrite("C:\\Program Files (x86)\\Steam\\steamapps\\common\\GarrysMod\\renders\\render7.ppm", ImageData.data(), Res);
	if (success) {
		double RENDER_END_TIME = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - RENDER_START_TIME).count();
		printLua(LUA, "Finished!\nRender & Save Time: " + std::to_string(RENDER_END_TIME / 1000) + "Seconds");
	}

	return 0;
}

// Called when the module is unloaded
GMOD_MODULE_CLOSE()
{
	return 0;
}
