-- Thruster effect descripton to be used by the player ship.
thruster = ParticleEffect("thruster")

local lifetimeMin = 0.1
local lifetimeMax = 0.5
local sizeMin = 0.2
local sizeMax = 0.4
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

-- Effect descripton for a trailing particle effect following bullets.
testeffect = ParticleEffect("testeffect")
testeffect_emitter0 = testeffect:addEmitter()
testeffect_emitter0.particlesPerEmit = 1
testeffect_emitter0.spawnTime = 0.1
testeffect_emitter0.lifetimeMin = 2.0
testeffect_emitter0.lifetimeMax = 3.0
testeffect_emitter0.sizeMin = 0.2
testeffect_emitter0.sizeMax = 0.5
testeffect_emitter0.rotationMin = 0.0
testeffect_emitter0.rotationMax = 3.14
testeffect_emitter0.startColor = color.new(1,0,0,1)
testeffect_emitter0.endColor = color.new(1,1,1,0)
testeffect_emitter0.force = vec2.new(100,100)
testeffect_emitter0.spread = 360
testeffect_emitter0.position = vec2.new(0, 0)
testeffect_emitter0.relative = true
testeffect_emitter0.texture = texture.new("diamond.dds")
testeffect:store()

-- Explosion effect description when an enemy explodes.
explosion = ParticleEffect("explosion")
explosion_emitter0 = explosion:addEmitter()
explosion_emitter0.particlesPerEmit = 36
explosion_emitter0.lifetime = 0.5
explosion_emitter0.spawnTime = 0.5
explosion_emitter0.lifetimeMin = 0.3
explosion_emitter0.lifetimeMax = 0.5
explosion_emitter0.sizeMin = 0.2
explosion_emitter0.sizeMax = 0.5
explosion_emitter0.rotationMin = 0.0
explosion_emitter0.rotationMax = 3.14
explosion_emitter0.startColor = color.new(1,0,0,1)
explosion_emitter0.endColor = color.new(1,1,1,0)
explosion_emitter0.force = vec2.new(1000,1000)
explosion_emitter0.spread = 360
explosion_emitter0.position = vec2.new(0, 0)
explosion_emitter0.relative = true
explosion_emitter0.texture = texture.new("boulder.dds")
explosion:store()


