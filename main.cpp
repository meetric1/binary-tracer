/*

GMod binary vs2019 project template for building to Windows by Derpius.

Before using you'll need to change your aditional include directories in this project's settings to include the GMod headers.
(which can be found at https://github.com/Facepunch/gmod-module-base/tree/development)

Note, by default this template is configured to build a clientside binary module,
to change this simply change the output name of the .dll to gmsv_ rather than gmcl_.

If you are making a module which uses both a clientside and serverside binary, then you should
create two of these template projects in the same solution, naming one client and the other
server (or words to that effect), in order to keep everything tidy.

Before building make sure you're in release mode at the top of Visual Studio,
and change the build architecture to what you want to build to.
(Both x86 and x64 have different project settings, so you will need to specify the include directory for both,
as well as changing gmcl_ to gmsv_ on both if this is a serverside project)

Using your binary in GLua is extremely easy, you simply call require("nameOfMyModule"), and GMod will load a
binary from lua/bin matching that name (note that you do not put gmcl_/gmsv_ or _win32/_win64 in the string to require,
as GMod automatically selects an apropriate binary matching the context of where require is being called from)

*/


#define _USE_MATH_DEFINES

#include "GarrysMod/Lua/Interface.h"
#include "Main.h"

using namespace GarrysMod::Lua;


/// FUNCTIONS ///

// Prints input to in-game console
void printLua(GarrysMod::Lua::ILuaBase* inst, std::string text)
{
    inst->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
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


void addSphere(Sphere s)
{
    ObjectArray.push_back(s);

}



ObjectData TraceAll(Trace traceData)
{
    float Dist = FLT_MAX;
    ObjectData object = ObjectData{ false };

    for (unsigned int i = 0; i < ObjectArray.size(); i++)
    {
        ObjectData SphereData = ObjectArray[i].intersect(traceData);

        if (SphereData.hit)
        {
            float d = glm::distance(SphereData.hitPos, traceData.pos);

            if (d < Dist)
            {
                Dist = d;
                object = SphereData;
            }

        }

    }

    Plane p{ vec3(0.f, -5.f, 0.f), vec3(0.f, 1.f, 0.f) };
    ObjectData planeHit = p.intersect(traceData);

    if (object.hit == false)
    {
        object = planeHit;

    }

    return object;

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
    for(int i = 0; i < 10; i++)
    {
        Sphere s{ vec3(sin(deg2rad(i * 36)) * 10 - 25, -5, cos(deg2rad(i * 36)) * 10), vec3(), vec3(i * 10), 3.f};
        addSphere(s);
    }

    glm::mat3x3 matrix = glm::orientation(Cam.dir, vec3(0.f, 0.f, 1.f));

    /// Actual Tracing ///

    for (unsigned int y = 0; y < Res[1]; y++)
    {
        for (unsigned int x = 0; x < Res[0]; x++)
        {

            //Fov calc

            float scale = tan(deg2rad(Cam.fov * 0.5f));
            float imageAspectRatio = Res[0] / static_cast<float>(Res[1]);

            float xDir = (2.f * (x + 0.5f) / static_cast<float>(Res[0]) - 1) * imageAspectRatio * scale;
            float yDir = (1.f - 2.f * (y + 0.5f) / static_cast<float>(Res[1])) * scale;

            Trace traceData{ Cam.pos, vec3(xDir, yDir, 1) * matrix };

            ObjectData hitData = TraceAll(traceData);

            vec3 FinalColor = vec3(168, 219, 243);
            if (hitData.hit == true)
            {
                FinalColor = hitData.color;

            }

            writePixel(x, y, FinalColor.x, FinalColor.y, FinalColor.z);

        }

    }
    




    bool success = ppmWrite("C:\\Program Files (x86)\\Steam\\steamapps\\common\\GarrysMod\\renders\\render6.ppm", ImageData.data(), Res);
    if (success)
    {

        double RENDER_END_TIME = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - RENDER_START_TIME).count();

        printLua(LUA, "Finished!\nRender & Save Time: " + std::to_string(RENDER_END_TIME / 1000) + "Seconds");

    };

	return 0;
}

// Called when the module is unloaded
GMOD_MODULE_CLOSE()
{
	return 0;
}
