#pragma once

/*
  TODO:

  Project:
  Fix opengl_glx for linux compat.
  
  
  Data path
  Parenting entities
  CRT-Emulation!
  Stary-Background!

  Improvments:
  - Shapes are heap allocated in runtime, probely not the best idea
  
  Reallocation:
  - Components
  - Lines
  - Particle buffers
  - Sprites
  
  Internal build option in premake
  DebugView in C.voidstorm_debug
  
  BugList:
  - Shooting continues after releasing the right stick

  FeaturesToAdd:
  - Mouse & Keyboard.
  - Touch Input.

  Sprite:
  Origin (If we want rotate around another point then sprite center)
  Source (If we want spritessheets)

  Future:
  - PolygonVsPolygon
  - Multiple shapes per entity
  - ODE
  - Different simulation body for particles (Swirling etc)
  - Polygon-Particles?

  -  RESOURCE MANAGER (voidstorm_resources)
     ReferenceCounting? Ref<T>
     Shaders
     Fonts
   
  -  AUDIO
  -  MENUS

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
#include <dcutil/stack.h>
#include <dcutil/pool.h>

#include <dcfx/context.h>

#include "voidstorm_misc.h"
#include "voidstorm_math.h"
#include "voidstorm_resources.h"
#include "voidstorm_input.h"
#include "voidstorm_entity.h"
#include "voidstorm_component.h"
#include "voidstorm_render.h"
#include "voidstorm_shape.h"
#include "voidstorm_physics.h"

extern HeapAllocator* g_allocator;

// TODO: Rename to engine context.
struct GameState
{
    Renderer* renderer;
    PhysicsWorld* physics;
    ResourceManager* resources;
    GameInput* input;
    World* world;
};









