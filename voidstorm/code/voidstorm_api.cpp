#define LUA_ENUM(L, name, val)			\
    lua_pushlstring(L, #name, sizeof(#name)-1); \
    lua_pushnumber(L, val);			\
    lua_settable(L, -3);

#ifdef VOIDSTORM_INTERNAL
struct LuaFile
{
    char name[180];
    char path[180];
    uint32_t hash;
    FileTime timeWhenLoaded;
};

extern tinystl::vector<LuaFile>* g_luaFiles;
#endif

namespace api
{    
  
/* 
   Lua stackdump code.
   http://cc.byexamples.com/2008/11/19/lua-stack-dump-for-c/
*/
    static int stackdump_g(lua_State* l)
    {
	int i;
	int top = lua_gettop(l);
 
	PRINT("total in stack %d\n",top);
 
	for (i = 1; i <= top; i++)
	{  /* repeat for each level */
	    int t = lua_type(l, i);
	    switch (t) {
	    case LUA_TSTRING:  /* strings */
		PRINT("string: '%s'\n", lua_tostring(l, i));
		break;
	    case LUA_TBOOLEAN:  /* booleans */
		PRINT("boolean %s\n",lua_toboolean(l, i) ? "true" : "false");
		break;
	    case LUA_TNUMBER:  /* numbers */
		PRINT("number: %g\n", lua_tonumber(l, i));
		break;
	    default:  /* other values */
		PRINT("%s\n", lua_typename(l, t));
		break;
	    }
	    PRINT("  ");  /* put a separator */
	}
	PRINT("\n");  /* end the listing */
	return 0;
    }

    static int tabledump_g(lua_State* luaState, int index)
    {
	/* table is in the stack at index 't' */
	lua_pushnil(luaState);  /* first key */
	while (lua_next(luaState, index) != 0) {
	    /* uses 'key' (at index -2) and 'value' (at index -1) */
	    PRINT("%s - %s\n",
		  lua_typename(luaState, lua_type(luaState, -2)),
		  lua_typename(luaState, lua_type(luaState, -1)));
	    /* removes 'value'; keeps 'key' for next iteration */
	    lua_pop(luaState, 1);
	}
	return 0;
    }

    static int myDofile(lua_State* L)
    {	
	int result = 0;
	LuaFile file;
	const char* filename = lua_tostring(L, 1);
	strcpy(file.name, filename);

#ifdef ANDROID
	SDL_RWops* handle = SDL_RWFromFile(filename, "r");
	if(handle == 0)
	{
	    PRINT("Error open file %s. \n", filename);
	}
	else
	{
	    SDL_RWseek(handle, 0, SEEK_END);
	    size_t size = (size_t)SDL_RWtell(handle); 
	    SDL_RWseek(handle, 0, SEEK_SET);

	    char* data = new char[size + 1];
	    size_t bytes = SDL_RWread(handle, data, 1, size);
	    if(bytes != size)
	    {
		PRINT("Error reading file %s. \n", filename);
	    }
	    else
	    {
		data[size] = '\0';
		int error = luaL_dostring(L, data);
		if(error != 0){
		    PRINT("luaL_dostring: %s @ %s\n", filename, lua_tostring(L,-1));
		    error = error;
		}
	    }

	    delete[] data;
	    SDL_RWclose(handle);
	}
#else	
	strcpy(file.path, VOIDSTORM_SCRIPT_DIRECTORY);
	strcat(file.path, file.name);
  
	if (luaL_dofile(L, file.path))
	{
	    PRINT("luaL_dofile: %s @ %s\n", file.path, lua_tostring(L, -1));
	    return 0;
	}

	PRINT("File loaded: %s\n", file.path);

#ifdef VOIDSTORM_INTERNAL
	if (getLastWriteTime(file.path, &file.timeWhenLoaded) != 0)
	{
	    file.hash = dcutil::fnv32(&file.name[0]);

		bool newFile = true;
	    for (size_t i = 0; i < g_luaFiles->size(); ++i)
	    {
		LuaFile& loadedFile = g_luaFiles->operator[](i);
		if (loadedFile.hash == file.hash)
		{
		    loadedFile.timeWhenLoaded = file.timeWhenLoaded;
			newFile = false;
		}
	    }

		if(newFile)
 	    g_luaFiles->push_back(file);
	}
#endif
#endif
	return result;
    }

    static luaL_Reg glib [] =
    {
	{ "stackdump", stackdump_g},
	{ "dofile", myDofile },
	{ NULL, NULL }
    };

    inline VoidstormContext* getContext(lua_State* luaState)
    {
	lua_getglobal(luaState, "_G");
	lua_getfield(luaState, -1, "state");
	VoidstormContext* context = (VoidstormContext*)lua_topointer(luaState, -1);
	lua_pop(luaState, 1);
	return context;
    }
    
    /*
      NOTE: VECTOR 2 FUNCTIONS
    */
          
    inline void toVec2(lua_State* luaState, const glm::vec2& vec)
    {
	glm::vec2* s = (glm::vec2*)lua_newuserdata(luaState, sizeof(glm::vec2));
	*s = vec;
	lua_getglobal(luaState, "vec2_m");
	lua_setmetatable(luaState, -2);
    }

    static int vec2SetValue(lua_State* luaState)
    {
	glm::vec2* ptemp = (glm::vec2*)lua_touserdata(luaState, 1);
	const char* memberName = lua_tostring(luaState, 2);
	float value  = (float)luaL_checknumber(luaState, 3);
	if(strcmp(memberName, "x") == 0)
	    ptemp->x = value;
	if(strcmp(memberName, "y") == 0)
	    ptemp->y = value;
	return 0;
    }

    static int vec2GetValue(lua_State* luaState)
    {
	glm::vec2* ptemp = (glm::vec2*)lua_touserdata(luaState, 1);
	const char* memberName = lua_tostring(luaState, 2);
	if(strcmp(memberName, "x") == 0)
	    lua_pushnumber(luaState, ptemp->x);
	if(strcmp(memberName, "y") == 0)
	    lua_pushnumber(luaState, ptemp->y);

	// TODO (daniel): This should be a static function.
	if(strcmp(memberName, "normalize") == 0)
	    toVec2(luaState, glm::normalize(*ptemp));
	return 1;
    }

    static int vec2Add(lua_State* luaState)
    {
	glm::vec2* v1 = (glm::vec2*)lua_touserdata(luaState, 1);
	glm::vec2* v2 = (glm::vec2*)lua_touserdata(luaState, 2);
	glm::vec2 v3 = (*v1)+(*v2);

	toVec2(luaState, v3);
	return 1;
    }

    static int vec2Sub(lua_State* luaState)
    {
	glm::vec2* v1 = (glm::vec2*)lua_touserdata(luaState, 1);
	glm::vec2* v2 = (glm::vec2*)lua_touserdata(luaState, 2);
	glm::vec2 v3 = (*v1)-(*v2);

	toVec2(luaState, v3);
	return 1;
    }

    static int vec2Div(lua_State* luaState)
    {
	glm::vec2* v1 = (glm::vec2*)lua_touserdata(luaState, 1);
	glm::vec2* v2 = (glm::vec2*)lua_touserdata(luaState, 2);
	glm::vec2 v3 = (*v1)/(*v2);

	toVec2(luaState, v3);
	return 1;
    }

    static int vec2Mul(lua_State* luaState)
    {
	glm::vec2* v1 = (glm::vec2*)lua_touserdata(luaState, 1);
	glm::vec2 result;
	if(lua_isnumber(luaState, 2))
	{
	    float scalar = (float)luaL_checknumber(luaState, 2);
	    result = *v1 * scalar;
	}
	else
	{
	    glm::vec2* v2 = (glm::vec2*)lua_touserdata(luaState, 2);
	    result = (*v1)*(*v2);
	}

	toVec2(luaState, result);
	return 1;
    }

    static int vec2Length(lua_State* luaState)
    {
	glm::vec2* v1 = (glm::vec2*)lua_touserdata(luaState, 1);
	float vecdot = glm::length((*v1));
	lua_pushnumber(luaState, vecdot);
	return 1;
    }

    const struct luaL_Reg vec2_functions_m[] =
    {
	{"__newindex", vec2SetValue},
	{"__index", vec2GetValue},
	{"__add", vec2Add},
	{"__sub", vec2Sub},
	{"__div", vec2Div},
	{"__mul", vec2Mul},
	{"__len", vec2Length},
	{NULL, NULL}
    };

    static int vec2New(lua_State* luaState)
    {
	float x = (float)lua_tonumber(luaState, 1);
	float y = (float)lua_tonumber(luaState, 2);

	glm::vec2* v = (glm::vec2*)lua_newuserdata(luaState, sizeof(glm::vec2));
	v->x = x;
	v->y = y;

	toVec2(luaState, *v);
	return 1;
    }

    const struct luaL_Reg vec2_functions_s[] =
    {
	{"new", vec2New},
	{NULL, NULL}
    };

    /*
      NOTE: COLOR FUNCTIONS
    */
    
    inline void toColor(lua_State* luaState, const glm::vec4& color)
    {
	glm::vec4 *s = (glm::vec4*)lua_newuserdata(luaState, sizeof(glm::vec4));
	*s = color;
	lua_getglobal(luaState, "color_m");
	lua_setmetatable(luaState, -2);
    }

    static int colorSetValue(lua_State* luaState)
    {
	glm::vec4* ptemp = (glm::vec4*)lua_touserdata(luaState, 1);
	const char* memberName = lua_tostring(luaState, 2);
	float value  = (float)luaL_checknumber(luaState, 3);
	
	if(strcmp(memberName, "x") == 0 || strcmp(memberName, "r") == 0 || strcmp(memberName, "h") == 0)
	    ptemp->x = value;
	if(strcmp(memberName, "y") == 0 || strcmp(memberName, "g") == 0 || strcmp(memberName, "s") == 0)
	    ptemp->y = value;
	if(strcmp(memberName, "z") == 0 || strcmp(memberName, "b") == 0 || strcmp(memberName, "v") == 0)
	    ptemp->z = value;
	if(strcmp(memberName, "w") == 0 || strcmp(memberName, "a") == 0)
	    ptemp->w = value;
	
	return 0;
    }

    static int colorGetValue(lua_State* luaState)
    {
	glm::vec4* ptemp = (glm::vec4*)lua_touserdata(luaState, 1);
	const char* memberName = lua_tostring(luaState, 2);
	
	if(strcmp(memberName, "x") == 0 || strcmp(memberName, "r") == 0 || strcmp(memberName, "h") == 0)
	    lua_pushnumber(luaState, ptemp->x);
	if(strcmp(memberName, "y") == 0 || strcmp(memberName, "g") == 0 || strcmp(memberName, "s") == 0)
	    lua_pushnumber(luaState, ptemp->y);
	if(strcmp(memberName, "z") == 0 || strcmp(memberName, "b") == 0 || strcmp(memberName, "v") == 0)
	    lua_pushnumber(luaState, ptemp->w);
	if(strcmp(memberName, "w") == 0 || strcmp(memberName, "a") == 0)
	    lua_pushnumber(luaState, ptemp->z);

	return 1;
    }

    static int colorAdd(lua_State* luaState)
    {
	glm::vec4* c1 = (glm::vec4*)lua_touserdata(luaState, 1);
	glm::vec4* c2 = (glm::vec4*)lua_touserdata(luaState, 2);
	glm::vec4 c3 = (*c1)+(*c2);

	toColor(luaState, c3);
	return 1;
    }

    static int colorSub(lua_State* luaState)
    {
	glm::vec4* c1 = (glm::vec4*)lua_touserdata(luaState, 1);
	glm::vec4* c2 = (glm::vec4*)lua_touserdata(luaState, 2);
	glm::vec4 c3 = (*c1)-(*c2);

	toColor(luaState, c3);
	return 1;
    }

    static int colorDiv(lua_State* luaState)
    {
	glm::vec4* c1 = (glm::vec4*)lua_touserdata(luaState, 1);
	glm::vec4* c2 = (glm::vec4*)lua_touserdata(luaState, 2);
	glm::vec4 c3 = (*c1)/(*c2);

	toColor(luaState, c3);
	return 1;
    }

    static int colorMul(lua_State* luaState)
    {
	glm::vec4* c1 = (glm::vec4*)lua_touserdata(luaState, 1);
	glm::vec4 result;
	if(lua_isnumber(luaState, 2))
	{
	    float scalar = (float)luaL_checknumber(luaState, 2);
	    result = *c1 * scalar;
	}
	else
	{
	    glm::vec4* c2 = (glm::vec4*)lua_touserdata(luaState, 2);
	    result = (*c1)*(*c2);
	}

	toColor(luaState, result);
	return 1;
    }

    // TODO (daniel): Move these conversion functions to the dcutil library.
    static int toRGBFromHSV(lua_State* luaState)
    {
	glm::vec4* hsv = (glm::vec4*)lua_touserdata(luaState, 1);

	// H [0, 360] S and V [0.0, 1.0].
	float h = hsv->x;
	float s = hsv->y;
	float v = hsv->z;

	int i = (int)floor(h/60.0f) % 6;
	float f = h/60.0f - floor(h/60.0f);
	float p = v * (float)(1 - s);
	float q = v * (float)(1 - s * f);
	float t = v * (float)(1 - (1 - f) * s);

	glm::vec4 rgb = glm::vec4(0,0,0,hsv->w);
    
	switch (i) {
	case 0:
	    rgb.x = v;
	    rgb.y = t;
	    rgb.z = p;
	    break;
	case 1:
	    rgb.x = q;
	    rgb.y = v;
	    rgb.z = p;
	    break;
	case 2:
	    rgb.x = p;
	    rgb.y = v;
	    rgb.z = t;
	    break;
	case 3:
	    rgb.x = p;
	    rgb.y = q;
	    rgb.z = v;
	    break;
	case 4:
	    rgb.x = t;
	    rgb.y = p;
	    rgb.z = v;
	    break;
	case 5:
	    rgb.x = v;
	    rgb.y = p;
	    rgb.z = q;
	    break;
	}
	toColor(luaState, rgb);
	return 1;
    };

    static int toHSVFromRGB(lua_State* luaState)
    {
	glm::vec4* rgb = (glm::vec4*)lua_touserdata(luaState, 1);

	float r = rgb->x;
	float g = rgb->y;
	float b = rgb->z;

	float minRGB = glm::min(r, glm::min(g, b));
	float maxRGB = glm::max(r, glm::max(g, b));

	glm::vec4 hsv = glm::vec4(0,0,0,rgb->w);
    
	if(minRGB == maxRGB)
	{
	    hsv.z = minRGB;
	    toColor(luaState, hsv);
	    return 1;
	}

	float d = (r == minRGB) ? g-b : ((b == minRGB) ? r-g : b-r);
	int h = (r == minRGB) ? 3 : ((b == minRGB) ? 1 : 5);

	hsv.x = 60.0f * (h - d / (maxRGB - minRGB));
	hsv.y = (maxRGB - minRGB) / maxRGB;
	hsv.z = maxRGB;
    
	toColor(luaState, hsv);
	return 1;
    };

    const struct luaL_Reg color_functions_m[] =
    {
	{ "__index", colorGetValue },
	{ "__newindex", colorSetValue },
	{ "__add", colorAdd },
	{ "__sub", colorSub },
	{ "__div", colorDiv },
	{ "__mul", colorMul },
	{ NULL, NULL }
    };

    static int colorNew(lua_State* luaState)
    {
	float r = (float)lua_tonumber(luaState, 1);
	float g = (float)lua_tonumber(luaState, 2);
	float b = (float)lua_tonumber(luaState, 3);
	float a = (float)lua_tonumber(luaState, 4);    

	glm::vec4* v = (glm::vec4*)lua_newuserdata(luaState, sizeof(glm::vec4));
	v->x = r;
	v->y = g;
	v->z = b;
	v->w = a;

	toColor(luaState, *v);  
	return 1;
    }

    const struct luaL_Reg color_functions_s[] =
    {
	{ "new", colorNew },
	{ "toRGBFromHSV", toRGBFromHSV },
	{ "toHSVFromRGB", toHSVFromRGB },
	{ NULL, NULL }
    };

    /*
      TEXTURE FUNCTIONS
    */
    static int textureSize(lua_State* luaState)
    {
	Texture* texture = *(Texture**)lua_touserdata(luaState, 1);

	glm::vec2* v = (glm::vec2*)lua_newuserdata(luaState, sizeof(glm::vec2));
	v->x = (float)texture->width;
	v->y = (float)texture->height;

	toVec2(luaState, *v);
	return 1;
    }

    const struct luaL_Reg texture_functions_m[] =
    {
	{ "size", textureSize },
	{ NULL, NULL }
    };

    static int textureNew(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);
	const char* path = lua_tostring(luaState, 1);

	Texture** texture = (Texture**)lua_newuserdata(luaState, sizeof(Texture*));
	*texture = context->resources->textures.load(path);

	lua_getglobal(luaState, "texture_m");
	lua_setmetatable(luaState, -2);
	return 1;
    };
    
    static int textureDelete(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);
	Texture** texture = (Texture**)lua_touserdata(luaState, 1);

	context->resources->textures.remove(*texture);
	return 0;
    };

    const struct luaL_Reg texture_functions_s[] =
    {
	{ "new", textureNew },
	{ "delete", textureDelete },
	{ NULL, NULL }
    };
    
    /*
       PARTICLE FUNCTIONS
    */
    inline void toParticle(lua_State* luaState, ParticleEffectHandle handle)
    {
	ParticleEffectHandle* ud = (ParticleEffectHandle*)lua_newuserdata(luaState, sizeof(ParticleEffectHandle));
	*ud = handle;
    
	lua_getglobal(luaState, "particle_m");
	lua_setmetatable(luaState, -2);
    }

    static int particlePlay(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);
	ParticleEffectHandle handle = *(ParticleEffectHandle*)lua_touserdata(luaState, 1);
	uint32_t frames = (uint32_t)lua_tointeger(luaState, 2);
	
	context->renderer->getParticleEngine()->play(handle, frames);
	return 0;
    }

    static int particleStop(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);
	ParticleEffectHandle handle = *(ParticleEffectHandle*)lua_touserdata(luaState, 1);
    
	context->renderer->getParticleEngine()->stop(handle);
	return 0;
    }

    static int particleSetPosition(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);
	ParticleEffectHandle handle = *(ParticleEffectHandle*)lua_touserdata(luaState, 1);
	glm::vec2 position = *(glm::vec2*)lua_touserdata(luaState, 2);

	context->renderer->getParticleEngine()->setPosition(handle, position);
	return 0;
    }

    static int particleSetRotation(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);
	ParticleEffectHandle handle = *(ParticleEffectHandle*)lua_touserdata(luaState, 1);
	float rotation = (float)lua_tonumber(luaState, 2);

	context->renderer->getParticleEngine()->setRotation(handle, rotation);
	return 0;
    }

    static int particleSetDepth(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);
	ParticleEffectHandle handle = *(ParticleEffectHandle*)lua_touserdata(luaState, 1);
	float depth = (float)lua_tonumber(luaState, 2);

	context->renderer->getParticleEngine()->setDepth(handle, depth);
	return 0;
    }

    const struct luaL_Reg particle_functions_m[] =
    {
	{ "play", particlePlay },
	{ "stop", particleStop },
	{ "setPosition", particleSetPosition },
	{ "setRotation", particleSetRotation },
	{ "setDepth", particleSetDepth },
	{ NULL, NULL }
    };

    static int particleNew(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);
	
	ParticleEffectDescription* effectDesc = context->resources->effects.load(luaState);   
	ParticleEffectHandle handle = context->renderer->getParticleEngine()->createHandle(effectDesc);
    
	toParticle(luaState, handle);    
	return 1;
    };

    static int particleStore(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);
	context->resources->effects.load(luaState);
	return 0;
    };

    static int particleDelete(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);
	ParticleEffectHandle handle = *(ParticleEffectHandle*)lua_touserdata(luaState, 1);

	context->renderer->getParticleEngine()->deleteHandle(handle);    
	return 0;
    };

    const struct luaL_Reg particle_functions_s[] =
    {
	{ "new", particleNew },
	{ "store", particleStore },
	{ "delete", particleDelete },
	{ NULL, NULL }
    };

    /*
      INPUT FUNCTIONS
    */   
    static int isButtonDown(lua_State* luaState)
    {
	VoidstormContext* context = (VoidstormContext*)lua_topointer(luaState, 1);
	int index = (int)lua_tointeger(luaState, 2);

	bool32 result = (context->input->currentController->arr[index].isPressed);
    
	lua_pushboolean(luaState, result);
	return 1;
    };

    static int isButtonPressed(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);
	int index = (int)lua_tointeger(luaState, 1);

	bool32 result = (context->input->currentController->arr[index].isPressed
			 && !context->input->previousController->arr[index].isPressed);
    
	lua_pushboolean(luaState, result);
	return 1;
    };

    static int isButtonReleased(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);
	int index = (int)lua_tointeger(luaState, 1);

	bool32 result = (!context->input->currentController->arr[index].isPressed
			 && context->input->previousController->arr[index].isPressed);

	lua_pushboolean(luaState, result);
	return 1;
    }

    static int getLeftStick(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);
	toVec2(luaState, glm::vec2(context->input->currentController->leftStickX, context->input->currentController->leftStickY));
	return 1;
    }

    static int getRightStick(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);
	toVec2(luaState, glm::vec2(context->input->currentController->rightStickX, context->input->currentController->rightStickY));
	return 1;
    }

    const static luaL_Reg input_functions[] =
    {
	{ "isButtonDown", isButtonDown },
	{ "isButtonPressed", isButtonPressed },
	{ "isButtonReleased", isButtonReleased },
	{ "getLeftStick", getLeftStick },
	{ "getRightStick", getRightStick },
	{ NULL, NULL}
    };

    /*
      ENTITY FUNCTIONS
    */ 
    static int getNrOfEntities(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;

	lua_pushinteger(luaState, world->entities.getNrOfEntities());
    
	return 1;
    }

    static int createEntity(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	Entity entity = world->entities.create();

	// NOTE (daniel): A entity is always accompanied with a transform component.
	TransformManager::Instance instance = world->transforms.lookup(entity);
	if(instance.index == 0)
	{
	    instance = world->transforms.create(entity);
	}

	lua_pushinteger(luaState, entity.id);
	return 1;
    }

    static int destroyEntity(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);
	Entity entity = { (uint32_t)lua_tointeger(luaState, 1) };

	context->world->transforms.destroy(context->world->transforms.lookup(entity));
	context->world->physics.destroy(context->world->physics.lookup(entity));
	context->world->sprites.destroy(context->world->sprites.lookup(entity));
	context->world->responders.destroy(context->world->responders.lookup(entity));
	context->world->collisions.destroy(context->world->collisions.lookup(entity));
	context->world->entities.destroy(entity);

	return 0;
    }

    static int printEntity(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	Entity entity = { (uint32_t)lua_tointeger(luaState, 1) };

	const char* prnt = "Index: %d - Generation %d\n";
	PRINT(prnt, entity.index(), entity.generation());

	if(world->collisions.lookup(entity).index != 0) PRINT("Has Collision.\n");
	if(world->responders.lookup(entity).index != 0) PRINT("Has Responder.\n");
	if(world->transforms.lookup(entity).index != 0) PRINT("Has Transform.\n");
	if(world->physics.lookup(entity).index != 0) PRINT("Has Physics.\n");
	if(world->sprites.lookup(entity).index != 0) PRINT("Has Sprite..\n");
    
	return 0;
    }

    static int addSprite(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	Entity entity = { (uint32_t)lua_tointeger(luaState, 1) };

	SpriteManager::Instance instance = world->sprites.lookup(entity);
	if(instance.index == 0)
	{
	    instance = world->sprites.create(entity);
	}

	lua_pushinteger(luaState, instance.index);
	return 1;
    }

    static int addPhysics(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	Entity entity = { (uint32_t)lua_tointeger(luaState, 1) };

	PhysicsManager::Instance instance = world->physics.lookup(entity);
	if(instance.index == 0)
	{
	    instance = world->physics.create(entity);
	}

	return 0;
    }

    static int addCollision(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	Entity entity = { (uint32_t)lua_tointeger(luaState, 1) };
	uint32_t type = (uint32_t)lua_tointeger(luaState, 2);
	uint32_t mask = (uint32_t)lua_tointeger(luaState, 3);

	CollisionManager::Instance instance = world->collisions.lookup(entity);
	if(instance.index == 0)
	{
	    instance = world->collisions.create(entity, type, mask);
	}
	
	return 0;
    }

    static int addCircleShape(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);
	Entity entity = { (uint32_t)lua_tointeger(luaState, 1) };
	float radius = (float)lua_tonumber(luaState, 2);

	CollisionManager::Instance instance = context->world->collisions.lookup(entity);
	if(instance.index != 0)
	{
	    TransformManager::Instance tinstance = context->world->transforms.lookup(entity);
	    glm::vec2 position = context->world->transforms.getPosition(tinstance);
	    
	    context->world->collisions.createCircleShape(instance, radius, position);
	}

	return 0;
    }

    static int addPolygonShape(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);
	Entity entity = { (uint32_t)lua_tointeger(luaState, 1) };
	uint32_t count = (uint32_t)lua_tointeger(luaState, 2);
	assert(count < VOIDSTORM_MAX_POLYGON_VERTICES);
	glm::vec2 vertices[VOIDSTORM_MAX_POLYGON_VERTICES];
	for(uint32_t i = 0; i < count; ++i)
	{
	    vertices[i] = *(glm::vec2*)lua_touserdata(luaState, 3 + i);
	}
	
	CollisionManager::Instance instance = context->world->collisions.lookup(entity);
	if(instance.index != 0)
	{
	    context->world->collisions.createPolygonShape(instance, vertices, count);
	}

	return 0;
    }

    static int addResponder(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	Entity entity = { (uint32_t)lua_tointeger(luaState, 1) };
    
	CollisionResponderManager::Instance instance = world->responders.lookup(entity);
	if(instance.index == 0)
	{
	    instance = world->responders.create(entity);
	}

	return 0;
    }

    static int setType(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	CollisionManager::Instance instance = world->collisions.lookup({ (uint32_t)lua_tointeger(luaState, 1) });
	int type = (int)lua_tointeger(luaState, 2);

	world->collisions.setType(instance, type);
	return 0;
    }

    static int getType(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	CollisionManager::Instance instance = world->collisions.lookup({ (uint32_t)lua_tointeger(luaState, 1) });

	uint32_t type = world->collisions.getType(instance);
	lua_pushinteger(luaState, type);
	return 1;
    }

    static int setMask(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	CollisionManager::Instance instance = world->collisions.lookup({ (uint32_t)lua_tointeger(luaState, 1) });
	int mask = (int)lua_tointeger(luaState, 2);

	world->collisions.setMask(instance, mask);
	return 0;
    }

    static int setOffset(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	CollisionManager::Instance instance = world->collisions.lookup({ (uint32_t)lua_tointeger(luaState, 1) });
	glm::vec2* v = (glm::vec2*)lua_touserdata(luaState, 2);
	
	world->collisions.setOffset(instance, *v);
	return 0;
    }

    static int setRadius(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	CollisionManager::Instance instance = world->collisions.lookup({ (uint32_t)lua_tointeger(luaState, 1) });
	float radius = (float)lua_tonumber(luaState, 2);
    
	world->collisions.setRadius(instance, radius);
	return 0;
    }

    // TODO (daniel): There is probely a better and cleaner way for passing these results.
    static int getCollidedEntity(lua_State* luaState)
    {    
	World* world = getContext(luaState)->world;
	CollisionResponderManager::Instance instance = world->responders.lookup({ (uint32_t)lua_tointeger(luaState, 1) });

	int& entityCount = world->responders.data.collidedWith[instance.index].entityCount;
	
	lua_createtable(luaState, entityCount, 0);
	for (int i = 0; i < entityCount; i++)
	{
	    Entity& e = world->responders.data.collidedWith[instance.index].entity[i];
	    glm::vec2 pos = world->responders.data.collidedWith[instance.index].position[i];
	    
	    lua_createtable(luaState, 2, 0);	 
	     
	    lua_pushinteger(luaState, e.id);
	    lua_rawseti(luaState, -2, 1);

	    toVec2(luaState, pos);
	    lua_rawseti(luaState, -2, 2);

	    lua_rawseti(luaState, -2, i + 1);

	    e = { (uint16_t)-1 };
	}

	entityCount = 0;
	return 1;
    }

    static int setPosition(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);
	Entity e = { (uint32_t)lua_tointeger(luaState, 1) };
	glm::vec2 newPosition = *(glm::vec2*)lua_touserdata(luaState, 2);

	TransformManager::Instance transformInstance = context->world->transforms.lookup(e);
	CollisionManager::Instance collisionInstance = context->world->collisions.lookup(e);

	/* NOTE (daniel): When moving entities directly we cannot rely on the physics system to move the collision box, so we do it ourself here. */
	if(collisionInstance.index != 0)
	{
	    glm::vec2 oldPosition = context->world->transforms.getPosition(transformInstance);
	    glm::vec2 displacement = newPosition - oldPosition;
	    
	    if(glm::length(displacement) > 0)
	    {
	        context->world->collisions.tree.moveProxy(
		    context->world->collisions.getNode(collisionInstance),
		    displacement);
	    }
	}

	context->world->transforms.setPosition(transformInstance, newPosition);
	assert(newPosition == newPosition && "API SetPosition Failed");
	return 0;
    }

    static int getPosition(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	TransformManager::Instance instance = world->transforms.lookup({ (uint32_t)lua_tointeger(luaState, 1) });

	glm::vec2 pos = world->transforms.getPosition(instance);
	toVec2(luaState, pos);
	return 1;
    }

    static int setRotation(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	TransformManager::Instance instance = world->transforms.lookup({ (uint32_t)lua_tointeger(luaState, 1) });
	float value = (float)lua_tonumber(luaState, 2);
	
	world->transforms.setRotation(instance, value);
	return 0;
    }

    static int getRotation(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	TransformManager::Instance instance = world->transforms.lookup({ (uint32_t)lua_tointeger(luaState, 1) });

	float value  = world->transforms.getRotation(instance);    
	lua_pushnumber(luaState, value);
	return 1;
    }

    static int setDepth(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	TransformManager::Instance instance = world->transforms.lookup({ (uint32_t)lua_tointeger(luaState, 1) });
	float value = (float)lua_tonumber(luaState, 2);
	
	world->transforms.setDepth(instance, value);
	return 0;
    }


    static int getDepth(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	TransformManager::Instance instance = world->transforms.lookup({ (uint32_t)lua_tointeger(luaState, 1) });

	float value  = world->transforms.getDepth(instance);    
	lua_pushnumber(luaState, value);
	return 1;
    }

    static int setScale(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	TransformManager::Instance instance = world->transforms.lookup({ (uint32_t)lua_tointeger(luaState, 1) });
	float value = (float)lua_tonumber(luaState, 2);

	world->transforms.setScale(instance, value);
	return 0;
    }


    static int getScale(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	TransformManager::Instance instance = world->transforms.lookup({ (uint32_t)lua_tointeger(luaState, 1) });

	float value  = world->transforms.getScale(instance);   
	lua_pushnumber(luaState, value);
	return 1;
    }
    
    static int setTexture(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	SpriteManager::Instance instance = world->sprites.lookup({ (uint32_t)lua_tointeger(luaState, 1) });
	Texture** texture = (Texture**)lua_touserdata(luaState, 2);

	world->sprites.setTexture(instance, *texture);
	return 0;
    }
    
    static int setOrigin(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	SpriteManager::Instance instance = world->sprites.lookup({ (uint32_t)lua_tointeger(luaState, 1) });
	glm::vec2* origin = (glm::vec2*)lua_touserdata(luaState, 2);

	world->sprites.setOrigin(instance, *origin);
	return 0;
    }

    static int setSize(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	SpriteManager::Instance instance = world->sprites.lookup({ (uint32_t)lua_tointeger(luaState, 1) });
	glm::vec2* size = (glm::vec2*)lua_touserdata(luaState, 2);

	world->sprites.setSize(instance, *size);
	
	return 0;
    }

    static int setColor(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	SpriteManager::Instance instance = world->sprites.lookup({ (uint32_t)lua_tointeger(luaState, 1) });
	glm::vec4* color = (glm::vec4*)lua_touserdata(luaState, 2);
    
	world->sprites.setColor(instance, *color);	
	return 0;
    }

    static int getColor(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	SpriteManager::Instance instance = world->sprites.lookup({ (uint32_t)lua_tointeger(luaState, 1) });
	
	glm::vec4 color = world->sprites.getColor(instance);
	toColor(luaState, color);
	return 1;
    }

    static int setMass(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	PhysicsManager::Instance instance = world->physics.lookup({ (uint32_t)lua_tointeger(luaState, 1) });
	float mass = (float)lua_tonumber(luaState, 2);

	world->physics.setMass(instance, mass);
	return 0;
    }

    static int addForce(lua_State* luaState)
    {
	World* world = getContext(luaState)->world;
	PhysicsManager::Instance instance = world->physics.lookup({ (uint32_t)lua_tointeger(luaState, 1) });

	glm::vec2* v = (glm::vec2*)lua_touserdata(luaState, 2);
	
	world->physics.addForce(instance, *v);
	return 0;
    }

    const static luaL_Reg es_functions[] =
    {
	{ "getNrOfEntities", getNrOfEntities},
	{ "createEntity", createEntity},
	{ "destroyEntity", destroyEntity},
	{ "printEntity", printEntity},
	{ "addSprite", addSprite},
	{ "addPhysics", addPhysics},
	{ "addCollision", addCollision},
	{ "setCircleShape", addCircleShape},
	{ "setPolygonShape", addPolygonShape},
	{ "addResponder", addResponder},
	{ "setType", setType},
	{ "getType", getType},
	{ "setMask", setMask},
	{ "setOffset", setOffset},
	{ "setRadius", setRadius},
	{ "getCollidedEntity", getCollidedEntity},
	{ "setPosition", setPosition},
	{ "getPosition", getPosition},
	{ "setRotation", setRotation},
	{ "getRotation", getRotation},
	{ "setDepth", setDepth},
	{ "getDepth", getDepth},
	{ "setScale", setScale},
	{ "getScale", getScale},
	{ "setTexture", setTexture},
	{ "setSize", setSize},
	{ "setOrigin", setOrigin},
	{ "setColor", setColor},
	{ "getColor", getColor},
	{ "setMass", setMass},
	{ "addForce", addForce},
	{ NULL, NULL }
    };

   /*
      FONT FUNCTIONS
    */    
    static int fontWrite(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);
	const char* text = lua_tostring(luaState, 1);
	glm::vec2* position = (glm::vec2*)lua_touserdata(luaState, 2);
	bool32 mode = lua_toboolean(luaState, 3);

	context->renderer->write(text, *position, mode);

	return 0;
    };

    const static luaL_Reg font_functions[] =
    {
	{ "write", fontWrite },
	{ NULL, NULL }
    };


    /*
      ENGINE FUNCTIONS
    */    
    static int setPostProcessParams(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);

	float gaussBlurSigma = (float)lua_tonumber(luaState, 1);
	float gaussBlurTapSize = (float)lua_tonumber(luaState, 2);
	float exposure = (float)lua_tonumber(luaState, 3);

	context->renderer->setPostProcessParams(gaussBlurSigma, gaussBlurTapSize, exposure);

	return 0;
    }

    static int getResolution(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);

	toVec2(luaState, context->renderer->getResolution());
	return 1;
    }

    static int setCameraPosition(lua_State* luaState)
    {
	VoidstormContext* context = getContext(luaState);

	glm::vec2* pos = (glm::vec2*)lua_touserdata(luaState, 1);
    
	context->renderer->setCameraPosition(*pos);

	return 0;
    }

    const static luaL_Reg voidstorm_functions[] =
    {
	{ "setPostProcessParams", setPostProcessParams},
	{ "getResolution", getResolution},
	{ "setCameraPosition", setCameraPosition },
	{ NULL, NULL }
    };

    void initialize(lua_State* luaState)
    {
	lua_getglobal(luaState, "_G");
    
	luaL_register(luaState, NULL, glib);
	luaL_register(luaState, "input", input_functions);	
	luaL_register(luaState, "es", es_functions);
	luaL_register(luaState, "font", font_functions);
	luaL_register(luaState, "voidstorm", voidstorm_functions);
	luaL_register(luaState, "vec2", vec2_functions_s);
	luaL_register(luaState, "color", color_functions_s);
	luaL_register(luaState, "texture", texture_functions_s);
	luaL_register(luaState, "particle", particle_functions_s);
    
	lua_pop(luaState, 8);

	lua_newtable(luaState);
	luaL_register(luaState, 0, vec2_functions_m);
	lua_setglobal(luaState, "vec2_m");

	lua_newtable(luaState);
	luaL_register(luaState, 0, color_functions_m);
	lua_setglobal(luaState, "color_m");

	lua_newtable(luaState);
	luaL_register(luaState, 0, texture_functions_m);
	lua_pushvalue(luaState, -1);
	lua_setfield(luaState, -2, "__index");
	lua_setglobal(luaState, "texture_m");

	lua_newtable(luaState);
	luaL_register(luaState, 0, particle_functions_m);
	lua_pushvalue(luaState, -1);
	lua_setfield(luaState, -2, "__index");
	lua_setglobal(luaState, "particle_m");
    
	lua_pop(luaState, 1);	
    }    
}
