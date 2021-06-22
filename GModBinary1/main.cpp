#define _USE_MATH_DEFINES

#include "Main.h"
#include "GarrysMod/Lua/Interface.h"

#include <glm/ext/matrix_transform.hpp>


using namespace GarrysMod::Lua;
using glm::vec3;

using Vector3 = bvh::Vector3<float>;
using Triangle = bvh::Triangle<float>;
using Ray = bvh::Ray<float>;
using Bvh = bvh::Bvh<float>;

/// FUNCTIONS ///

void printLua(ILuaBase* LUA, std::string text)
{
	LUA->PushSpecial(SPECIAL_GLOB);
	LUA->GetField(-1, "print");
	LUA->PushString(text.c_str());
	LUA->Call(1, 0);
	LUA->Pop();
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

Vector3 Vec3ToVector3(const vec3& vec)	//shit function name but whatever lol
{
	return Vector3(vec.x, vec.y, vec.z);
}
vec3 Vector3ToVec3(const Vector3& vec)
{
	return vec3(vec[0], vec[1], vec[2]);
}


HitResult TraceAll(
	const Ray& ray,
	const std::vector<std::shared_ptr<BaseObject>>& objects, const std::vector<bvh::Triangle<float>>& triangles,
	const bvh::SingleRayTraverser<Bvh>& traverser, const bvh::ClosestPrimitiveIntersector<Bvh, Triangle>& intersector
)
{
	HitResult closestHit;
	closestHit.hit = false;
	closestHit.t = FLT_MAX;

	std::shared_ptr<BaseObject> closestHitObject;

	for (size_t i = 0U; i < objects.size(); i++) {
		HitResult hit;
		if (objects[i]->intersect(ray, hit) && hit.t < closestHit.t) {
			closestHit = hit;
			closestHitObject = objects[i];
		}
	}

	if (closestHit.hit) {
		closestHit.pos = Vector3ToVec3(ray.origin + ray.direction * closestHit.t); // calculate position at the end rather than every object (way faster)

		// The color calc for the plane is expensive, so I moved color calculation to a new function that gets called on the closest hit only
		if (closestHitObject) closestHitObject->setHitColor(closestHit);
	}

	if (auto bvhHit = traverser.traverse(ray, intersector)) {
		auto intersection = bvhHit->intersection;

		if (intersection.t < closestHit.t) {
			closestHit.t = intersection.t;
			closestHit.color = vec3(0, 255, 0); //just fucking make it green idc
			closestHit.normal = Vector3ToVec3(triangles[bvhHit->primitive_index].n); //how do I calculate this? You calculate it using either a) the triangle's geometric normal (calculated from the edges, which I'm using here), or b) the shading normal (calculated from interpolating the vertex normals using the hit barycentrics)
			closestHit.hit = true;
		}
	}

	if (closestHit.hit) {
		closestHit.color *= calculateLighting(closestHit);
	}

	return closestHit;
}

/// Lua functions ///

//pretty much stolen from the visual mesh tracer
LUA_FUNCTION(getAllMeshes)
{

	if (LUA->Top() == 0) LUA->CreateTable();
	else if (LUA->IsType(1, Type::Nil)) {
		LUA->Pop(LUA->Top());
		LUA->CreateTable();
	}
	else {
		LUA->CheckType(1, Type::Table);
		LUA->Pop(LUA->Top() - 1); // Pop all but the table
	}

	size_t numEntities = LUA->ObjLen();
	for (size_t entIndex = 1; entIndex <= numEntities; entIndex++) {

		// Get entity
		LUA->PushNumber(entIndex);
		LUA->GetTable(1);
		LUA->CheckType(-1, Type::Entity);



		// Get entity id

		LUA->GetField(-1, "EntIndex");
		LUA->Push(-2);	//?
		LUA->Call(1, 1);
		double entId = LUA->CheckNumber(); // Get as a double so after we check it's positive a static cast to unsigned int wont overflow rather than using int
		LUA->Pop();

		if (entId < 0.0) LUA->ThrowError("Entity ID is less than 0");	//why would this even run lol

		// Lets print all da ents to see if the module can see them
		printLua(LUA, std::to_string(entId));

		LUA->Pop();


	}
	
	return 0;
}


// Called when the module is loaded
GMOD_MODULE_OPEN()
{
	printLua(LUA, "Opened Module");

	// Time to see how long the render took
	std::chrono::steady_clock::time_point RENDER_START_TIME = std::chrono::high_resolution_clock::now();

	// Resolution
	Res[0] = 1920; // X res
	Res[1] = 1080; // Y res
	
	ImageData = std::vector<unsigned char>(Res[0] * Res[1] * 3U, 0);

	// Camera
	Camera Cam{ vec3(0.f, 0.f, 0.f), normalize(vec3(1.f, 0.f, 0.f)), 90U };

	// Hard coded everything here as you're gonna need the proper up and target vectors in from GLua
	glm::mat4 viewMatrix = glm::lookAt(
		vec3(0.f), // Camera position,
		vec3(1.f, 0.f, 0.f), // Target position (the position the camera is looking at, you'll use cam pos + cam dir from lua)
		vec3(0.f, 0.f, 1.f) // Up vector (you'll use the eyeang's up vector from lua)
	);

	// Primitive objects
	std::vector<std::shared_ptr<BaseObject>> objects;
	{
		Plane p(vec3(0.f, 0.f, -10.f), vec3(0.f, 0.f, 1.f), vec3(1.f, 0.8f, 0.2f));
		objects.push_back(std::make_shared<Plane>(p));
	}

	for(int i = 0; i < 10; i++) {
		Sphere s{ vec3(sin(deg2rad(i * 36)) * 10.f + 25.f, cos(deg2rad(i * 36)) * 10.f, -5.f), vec3(), vec3(static_cast<float>(i) / 10.f), 3.f};
		objects.push_back(std::make_shared<Sphere>(s));
	}

	// Meshes
	std::vector<bvh::Triangle<float>> triangles;

	//let us print all da meshes
	//broken, doesnt print anything??
	LUA->PushSpecial(SPECIAL_GLOB);
		LUA->PushCFunction(getAllMeshes);
	LUA->Pop();

	// Singular Triangle 
	triangles.emplace_back(
		Vector3(50.f, 0.f, 10.f),
		Vector3(50.f, -100.f, -10.f),
		Vector3(50.f, 100.f, -10.f)
	);


	// Build acceleration structure
	Bvh accelStruct = Bvh();
	bvh::SweepSahBuilder<Bvh> builder(accelStruct);
	auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(triangles.data(), triangles.size());
	auto global_bbox = bvh::compute_bounding_boxes_union(bboxes.get(), triangles.size());
	builder.build(global_bbox, bboxes.get(), centers.get(), triangles.size());

	auto intersector = bvh::ClosestPrimitiveIntersector<Bvh, Triangle>(accelStruct, triangles.data());
	auto traverser = bvh::SingleRayTraverser<Bvh>(accelStruct);

	/// Actual Tracing ///
	float scale = tan(deg2rad(Cam.fov * 0.5f));
	float imageAspectRatio = Res[0] / static_cast<float>(Res[1]);
	for (unsigned int y = 0U; y < Res[1]; y++) {
		for (unsigned int x = 0U; x < Res[0]; x++) {
			//Fov calc
			float xDir = (2.f * (x + 0.5f) / static_cast<float>(Res[0]) - 1.f) * imageAspectRatio * scale;
			float yDir = (1.f - 2.f * (y + 0.5f) / static_cast<float>(Res[1])) * scale;

			Ray ray{
				Vec3ToVector3(viewMatrix * glm::vec4(0.f, 0.f, 0.f, 1.f)),
				Vec3ToVector3(viewMatrix * glm::vec4(-yDir, -1, xDir, 0.f)),
				0.f,
				FLT_MAX
			};

			HitResult hit = TraceAll(ray, objects, triangles, traverser, intersector);

			vec3 FinalColor = vec3(168.f, 219.f, 243.f);
			if (hit.hit) {
				FinalColor = hit.color * 255.f;
			}

			writePixel(x, y, FinalColor.x, FinalColor.y, FinalColor.z);
		}
	}
	
	bool success = ppmWrite("C:\\Program Files (x86)\\Steam\\steamapps\\common\\GarrysMod\\renders\\render7.ppm", ImageData.data(), Res);
	if (success) {
		double RENDER_END_TIME = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - RENDER_START_TIME).count();
		printLua(LUA, "Finished!\nRender & Save Time: " + std::to_string(RENDER_END_TIME / 1000) + "Seconds");
	}
	else 
	{
		printLua(LUA, "File did not save correctly!");
	}

	return 0;
}

// Called when the module is unloaded
GMOD_MODULE_CLOSE()
{
	return 0;
}
