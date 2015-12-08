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





