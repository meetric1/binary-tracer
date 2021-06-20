
#include "mesh.h"

using namespace GarrysMod::Lua;
using glm::vec3;


// Prints input to in-game console
void printLua(ILuaBase* LUA, std::string text)
{
	LUA->PushSpecial(SPECIAL_GLOB);
	LUA->GetField(-1, "print");
	LUA->PushString(text.c_str());
	LUA->Call(1, 0);
	LUA->Pop();
}

//pretty much stolen from the visual mesh tracer
void getAllMeshes(ILuaBase* LuaBase)
{

	if (LuaBase->Top() == 0) LuaBase->CreateTable();
	else if (LuaBase->IsType(1, Type::Nil)) {
		LuaBase->Pop(LuaBase->Top());
		LuaBase->CreateTable();
	}
	else {
		LuaBase->CheckType(1, Type::Table);
		LuaBase->Pop(LuaBase->Top() - 1); // Pop all but the table
	}

	size_t numEntities = LuaBase->ObjLen();
	for (size_t entIndex = 1; entIndex <= numEntities; entIndex++) {

		// Get entity
		LuaBase->PushNumber(entIndex);
		LuaBase->GetTable(1);	
		LuaBase->CheckType(-1, Type::Entity);

		// Get entity id
		std::pair<unsigned int, unsigned int> id;
		{
			LuaBase->GetField(-1, "EntIndex");
			LuaBase->Push(-2);	//?
			LuaBase->Call(1, 1);
			double entId = LuaBase->CheckNumber(); // Get as a double so after we check it's positive a static cast to unsigned int wont overflow rather than using int
			LuaBase->Pop();

			if (entId < 0.0) LuaBase->ThrowError("Entity ID is less than 0");	//why would this even run lol
			//also wtf... 1 line if statments... crazy

			// Lets print all da ents to see if the module can see them
			printLua(LuaBase, std::to_string(entId));

			id.first = entId;
		}


	}
	
	//return void;	//another megamind moment, took me like 3 minutes to figure out what was wrong with my code
}