
-- Effect description for a star system
starsystem = ParticleEffect("starsystem")

local starsystem_emitters_x = 20;
local starsystem_emitters_y = 20;

local starsystem_margin_x = (World.size.x / starsystem_emitters_x) * 2.0
local starsystem_margin_y = (World.size.y / starsystem_emitters_y) * 2.0

for y = 0, starsystem_emitters_y, 1 do
   for x = 0, starsystem_emitters_x, 1 do

      local emitter = starsystem:addEmitter()
      emitter.texture = texture.new("test.dds")
      emitter.colorMin = color.new(0,0,0,1)
      emitter.colorMax = color.new(1,1,1,1)
      emitter.particlesPerEmit = 5
      emitter.lifetime = 0 -- infinite
      emitter.spawntime = 0 -- only initial spawn
      emitter.lifetimeMin = 0 -- live forever
      emitter.lifetimeMax = 0
      emitter.sizeMin = 1
      emitter.sizeMax = 1
      emitter.depthMin = -2
      emitter.depthMax = 2
      emitter.force = vec2.new(0.01, 0.01)
      emitter.relative = false
      emitter.spread = 360
      emitter.rotationMin = 0
      emitter.rotationMax = 360
      emitter.position = vec2.new(starsystem_margin_x * x,
				  starsystem_margin_y * y)
   end
end

starsystem:store() 

-- Effect descripton for a trailing particle effect following bullets
bullet_trail = ParticleEffect("bullet_trail")

bullet_trail_emitter0 = bullet_trail:addEmitter()
bullet_trail_emitter0.particlesPerEmit = 1
bullet_trail_emitter0.spawntime = 0.1
bullet_trail_emitter0.lifetimeMin = 2.0
bullet_trail_emitter0.lifetimeMax = 3.0
bullet_trail_emitter0.sizeMin = 0.2
bullet_trail_emitter0.sizeMax = 0.5
bullet_trail_emitter0.rotationMin = 0.0
bullet_trail_emitter0.rotationMax = 3.14
bullet_trail_emitter0.colorMin = color.new(0.1,0.1,0.1,0.1)
bullet_trail_emitter0.colorMax = color.new(0.1,0.1,0.1,0.1)
bullet_trail_emitter0.startColor = color.new(0.1,0.1,0.1,0.1)
bullet_trail_emitter0.endColor = color.new(0,0,0,0)
bullet_trail_emitter0.force = vec2.new(100,100)
bullet_trail_emitter0.spread = 360
bullet_trail_emitter0.position = vec2.new(0, 0)
bullet_trail_emitter0.relative = true
bullet_trail_emitter0.texture = texture.new("diamond.dds")

bullet_trail:store()

-- Effect description for the player ship thrusters
thruster = ParticleEffect("thruster")

local lifetimeMin = 0.1
local lifetimeMax = 0.2
local sizeMin = 0.1
local sizeMax = 0.2
local spreadAngle = 90
local rightPosition = vec2.new(7, 24)
local leftPosition = vec2.new(-7, 24)
local idleThrusterForce = vec2.new(75, 75)
local thrusterTexture = texture.new("boulder.dds")

thruster_emitter0 = thruster:addEmitter()
thruster_emitter0.lifetimeMin = lifetimeMin
thruster_emitter0.lifetimeMax = lifetimeMax
thruster_emitter0.sizeMin = sizeMin
thruster_emitter0.sizeMax = sizeMax
thruster_emitter0.spread = spreadAngle
thruster_emitter0.position = rightPosition
thruster_emitter0.texture = thrusterTexture

thruster_emitter1 = thruster:addEmitter()
thruster_emitter1.lifetimeMin = lifetimeMin
thruster_emitter1.lifetimeMax = lifetimeMax
thruster_emitter1.sizeMin = sizeMin
thruster_emitter1.sizeMax = sizeMax
thruster_emitter1.spread = spreadAngle
thruster_emitter1.position = leftPosition
thruster_emitter1.texture = thrusterTexture

thruster_emitter2 = thruster:addEmitter()
thruster_emitter2.lifetimeMin = lifetimeMin
thruster_emitter2.lifetimeMax = lifetimeMax
thruster_emitter2.sizeMin = sizeMin - 0.1
thruster_emitter2.sizeMax = sizeMax
thruster_emitter2.force = idleThrusterForce
thruster_emitter2.spread = spreadAngle
thruster_emitter2.position = rightPosition
thruster_emitter2.texture = thrusterTexture

thruster_emitter3 = thruster:addEmitter()
thruster_emitter3.lifetimeMin = lifetimeMin
thruster_emitter3.lifetimeMax = lifetimeMax
thruster_emitter3.sizeMin = sizeMin - 0.1
thruster_emitter3.sizeMax = sizeMax
thruster_emitter3.force = idleThrusterForce
thruster_emitter3.spread = spreadAngle
thruster_emitter3.position = leftPosition
thruster_emitter3.texture = thrusterTexture

thruster:store()
