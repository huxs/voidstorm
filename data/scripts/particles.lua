bufferSizeTier = {}
bufferSizeTier.low = 0
bufferSizeTier.medium = 1
bufferSizeTier.high = 2

ParticleEmitter = class(nil)

function ParticleEmitter:new()
   self.particlesPerEmit = 1
   self.spawntime = 0.5
   self.lifetime = 0;
   self.lifetimeMin = 1.0
   self.lifetimeMax = 1.0
   self.rotationMin = 0.0
   self.rotationMax = 0.0
   self.sizeMin = 1.0
   self.sizeMax = 1.0
   self.depthMin = 0.0
   self.depthMax = 0.0
   self.bufferSizeTier = bufferSizeTier.low
   self.colorMin = color.new(1,1,1,1)
   self.colorMax = color.new(1,1,1,1)
   self.startColor = color.new(1,1,1,1)
   self.endColor = color.new(1,1,1,0)   
   self.force = vec2.new(1,0)
   self.spread = 0 -- angle
   self.position = vec2.new(0,0)
   self.relative = true
   self.texture = nil   
end

ParticleEffect = class(nil)

function ParticleEffect:new(name)
   print("Creating effect " .. name)
   self.name = name   
   self.emitters = {}
end

function ParticleEffect:addEmitter()
   local emitter = ParticleEmitter()
   table.insert(self.emitters, emitter)
   return emitter
end

function ParticleEffect:store()
   particle.store(self)
end

-- Explosion effect description when an enemy explodes
explosion = ParticleEffect("explosion")
explosion_emitter0 = explosion:addEmitter()
explosion_emitter0.particlesPerEmit = 36
explosion_emitter0.lifetime = 0.5
explosion_emitter0.spawntime = 0.5
explosion_emitter0.lifetimeMin = 0.3
explosion_emitter0.lifetimeMax = 0.5
explosion_emitter0.sizeMin = 0.2
explosion_emitter0.sizeMax = 0.5
explosion_emitter0.rotationMin = 0.0
explosion_emitter0.rotationMax = 3.14
explosion_emitter0.colorMin = color.new(1,0,0,1)
explosion_emitter0.colorMax = color.new(1,0,0,1)
explosion_emitter0.startColor = color.new(1,0,0,1)
explosion_emitter0.endColor = color.new(1,1,1,0)
explosion_emitter0.force = vec2.new(1000,1000)
explosion_emitter0.spread = 360
explosion_emitter0.position = vec2.new(0, 0)
explosion_emitter0.relative = true
explosion_emitter0.texture = texture.new("boulder.dds")
explosion:store()

-- Example on making a spiral particle system
spiral = ParticleEffect("spiral")
spiral_emitter = spiral:addEmitter()
spiral_emitter.bufferSizeTier = bufferSizeTier.high
spiral_emitter.particlesPerEmit = 10
spiral_emitter.spawntime = 0.005
spiral_emitter.lifetimeMin = 0.5
spiral_emitter.lifetimeMax = 1
spiral_emitter.relative = true
spiral_emitter.force = vec2.new(1000,1000)
spiral_emitter.spread = 0
spiral_emitter.position = vec2.new(15, 0)
spiral_emitter.texture = texture.new("pixel.dds")
spiral_emitter.sizeMin = 4
spiral_emitter.sizeMax = 4
spiral_emitter.depthMin = 1
spiral_emitter.depthMax = 2
spiral_emitter.rotationMin = 0.0
spiral_emitter.rotationMax = 0.0
spiral:store()




