#pragma once

/*
  TODO (daniel):
  - x86 libs for utility/render
  - opengl_glx for linux compabillity
  - camera entities would be pretty cool
  - Optimization: Following of entities is very slow in lua 5x128 follow entities -> 20 ms
  - Optimization: SIMD on spritebatching
  - Set the source rectangle for a sprite
  - Add shaders and fonts to resource manager
  - Better debug view. Add voidstorm_debug.cpp. / Tweak debug variables from lua

  PHYSICS:
  - Rotation
  - Friction between objects
  
  TODO (daniel): *Potential Future*
  - CRT-Emulation mode
  - Android (Touch Input etc..)
  - Polygon-Particle / Splines / Swirling particles
  - Concave polygons (Multiple convex per entity)
  - Audio!
  - Menus! 
*/

#include <SDL2/SDL.h>
#include <Lua/lua.hpp>

#include "voidstorm_config.h"
#include "voidstorm_platform.h"
#include "voidstorm_alloc.h"

#define TINYSTL_ALLOCATOR TinyStlAllocator
#include <TINYSTL/vector.h>
#include <TINYSTL/string.h>
#include <TINYSTL/unordered_map.h>

#include <dcutil/handleallocator.h>
#include <dcutil/hash.h>
#include <dcutil/allocator.h>
#include <dcutil/color.h>

#include <dcfx/context.h>

#include "voidstorm_misc.h"
#include "voidstorm_math.h"
#include "voidstorm_resources.h"
#include "voidstorm_input.h"
#include "voidstorm_entity.h"
#include "voidstorm_component.h"
#include "voidstorm_shape.h"
#include "voidstorm_collision.h"
#include "voidstorm_physics.h"
#include "voidstorm_render.h"

extern HeapAllocator* g_heapAllocator;
extern dcutil::StackAllocator* g_permStackAllocator;
extern dcutil::StackAllocator* g_gameStackAllocator;

struct World
{
    void reset();

    EntityManager entities;
    TransformManager transforms;
    PhysicsManager physics;
    CollisionManager collisions;
    CollisionResponderManager responders;
    SpriteManager sprites;
};

struct VoidstormContext
{
    Renderer* renderer;
    ResourceManager* resources;
    GameInput* input;
    World* world;
};









